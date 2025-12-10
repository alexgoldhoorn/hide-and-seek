#include <cassert>
#include <cmath>
#include <iostream>

#include "mcts.h"
#include "belief.h"

#include "seekerhs.h"
#include "Utils/generic.h"
#include "hsglobaldata.h"
#include "hsconfig.h"

//iriutils
#include "exceptions.h"


// TODO: could use RAVE (http://www.cs.utexas.edu/~pstone/Courses/394Rspring11/resources/mcrave.pdf)

using namespace pomcp;
using namespace std;

MCTS::MCTS(SeekerHSParams* params, Simulator* simulator)
{
    //check params
    if (params->numInitBeliefStates <= 0)
        throw CException(_HERE_, "number of initial belief states should be at least 1");
    if (params->numSim <= 0)
        throw CException(_HERE_, "number of simulations should be at least 1");
    if (params->maxTreeDepth <= 0)
        throw CException(_HERE_, "maximum tree depth should be at least 1");
    if (params->numInitBeliefStates<1)
        throw CException(_HERE_, "number of initial belief states should be at least 1");


    //set init values
    _simulator = simulator;
    _params = params;
    _treeDepth = 0;
    _root = NULL;

}

void MCTS::init() {
    DEBUG_POMCP(cout<<"MCTS.init"<<endl;);

    //generate root
    State* initState = _simulator->genInitState();
    DEBUG_POMCP(cout<<" init state: "<< initState->toString()<<endl;);

    _root = new Node(_simulator,NULL);
    expandNode(_root, initState);

    DEBUG_POMCP(cout<<" expanded root node: "<< _root->toString(true)<<endl;);
    Belief* belief = _root->getBelief();

    //check params
    assert(_params->numInitBeliefStates>0);
    //init belief
    FOR(i,_params->numInitBeliefStates) {
        State* state = _simulator->genInitState();
        belief->add(state);
    }

    DEBUG_POMCP(cout<<" belief: "<<belief->toString(-1)<<endl;);

    DEBUG_POMCP(cout<<"MCTS.init done"<<endl;);
}

MCTS::~MCTS() {
    delete _root;
}


Node* MCTS::getRoot() {
    return _root;
}


void MCTS::uctSearch() {
    //history depth
    size_t historyDepth = _history.size();
    DEBUG_POMCP(cout<<"MCTS.uctSearch (histDepth="<<historyDepth<<"; #sims="<<_params->numSim<<")"<<endl;);

    //do the simulations
    FOR(i,_params->numSim) {
        DEBUG_POMCP_SIM(cout << " sim#"<<i<<flush;);
        //get random state
        State* state = _root->getBelief()->getRandomSample(); //->copy(); //TODO: choose based on belief(!)
        DEBUG_POMCP_SIM(cout << " " <<state->toString()<<flush;);

        //new simulation
        _treeDepth = 0;
        double totalR = simulateNode(_root, state);
        DEBUG_POMCP_SIM(cout <<" r="<<totalR<<endl;);

        //trunctate history to go back to 'start' of simulation
        _history.truncate(historyDepth);
    }

    DEBUG_POMCP(cout<<"MCTS.uctSearch done"<<endl;);
}


double MCTS::simulateNode(Node *node, State *state) {
    //get greedy action for current state
    int action = greedyUCB(node,true);

    DEBUG_POMCP_SIM(cout<<" simN,td="<<_treeDepth<<flush;);
    //PeekTreeDepth = _treedepth
    if (_treeDepth > _params->maxTreeDepth) { //ag130730: max -> exclusive
        //reached the maximum tree depth
        DEBUG_POMCP_SIM(cout<<" maxDepth "<<endl<<flush;);
        return 0;
    }

    if (_treeDepth == 1) {
        //add a sample to belief
        node->getBelief()->add(state->copy());
        DEBUG_POMCP_SIM(cout<<" addBelief:"<<state->toString()<<flush;);
    }

    //get child
    NodeA* nodeA = node->getChild(action);

    //simulate node
    double totalR = simulateNodeA(nodeA, state, action);
    //add reward for current node
    node->addValue(totalR);

    return totalR;
}

double MCTS::simulateNodeA(NodeA *nodea, State *state, int action) {
    int o=-1;
    double immediateReward = 0, delayedReward = 0;

    //get new state, o and r
    State* nstate = _simulator->step(state, action, o, immediateReward);
    bool final = _simulator->isFinal(nstate);

    //add to history
    _history.add(action, o);

    //get node
    //bool isNodeNew = nodea->isChildSet(o);
    Node* nnode = NULL; //nodea->getChild(o);
    if (nodea->isChildSet(o)) {
        //node child already exists
        nnode = nodea->getChild(o);
        assert(nnode!=NULL);

    } else if (/*nnode == NULL &&*/ !final && nodea->getCount()>=_params->expandCount) {
        //decide to expand the node
        nnode = nodea->getChild(o);
        assert(nnode!=NULL); //should be made internally if not existed

        expandNode(nnode, nstate);
        DEBUG_POMCP_SIM(cout<<" expanded: "<<nnode->toString()<<flush;);
    }

    DEBUG_POMCP_SIM(cout<<" simNA,a="<<action<<",o="<<o<< ",final="<<final <<flush;);
    DEBUG_POMCP_SIM(cout<<(nnode==NULL?"[NULL]":nnode->toString() )<<flush;);

    if (!final) {
        //calculate reward until 'end'
        _treeDepth++;
        if (nnode != NULL) {
            //node existed so use policy tree to estimate reward
            delayedReward = simulateNode(nnode,nstate);
        } else {
            //node doesn't exist (yet), so use rollout to estimate reward
            delayedReward = rollout(nstate);
        }
        _treeDepth--;
    }

    delete nstate;

    //calc reward
    double totalR = immediateReward + _simulator->getDiscount()*delayedReward;

    DEBUG_POMCP_SIM(cout<<" rew: immR="<<immediateReward<<",delayedR="<<delayedReward<<",totalR="<<totalR<<flush;);

    nodea->addValue(totalR);
    return totalR;
}

int MCTS::greedyUCB(Node *node, bool ucb) {
    //init bestactions
    vector<int> bestActions;
    double bestActValue = -HSGlobalData::INFTY;

    //precalc N and log(N)
    unsigned int N = node->getCount();
    double logN = log(N+1); //AG: why N+1 -> because log(0)=-inf

    //init v and n
    double v = 0;
    unsigned int n = 0;

    //for each action calc expected reward
    FOR(a,_simulator->getNumActions()) {
        //get node for action
        NodeA* anode = node->getChild(a);

        if (anode!=NULL) { //AG note: child now always created, if NULL->new
            //get n and value
            v = anode->getValue();
            n = anode->getCount();

            if (ucb) { //add exploration value
                v += _params->explorationConst * sqrt(logN / n);
            }

            if (v>=bestActValue) {
                //'best' value
                if (v>bestActValue) { //'better' value
                    bestActions.clear();
                }
                //add action
                bestActValue = v;
                bestActions.push_back(a);
            }
        } //else: action not calculated!! (maybe show warning)
    }

    //assume at least 1 action best, but COULD be that
    assert(bestActions.size()>0);

    if (bestActions.size()==1) {
        return bestActions[0];
    } else {
        //get random action
        return bestActions[random(bestActions.size())];
    }
}

double MCTS::rollout(State *sstate) {
    double discount = 1.0;
    double totalR = 0;
    bool final = false;
    State* state = sstate->copy();
    State* nState = NULL;
    int a,o;
    double r;

    DEBUG_POMCP_SIM(cout<<" rollout"<<flush;);

    //do a simulation rollout
    //for(unsigned i=0; i<_params.numSim && !final; i++) {
    for (unsigned int numSteps = 0; numSteps + _treeDepth < _params->maxTreeDepth && !final; numSteps++) {
        //get possible actions
        vector<int> actions;
        _simulator->getActions(state, &_history, actions);

        //choose random action
        a = actions[random(actions.size())];

        //do step
        nState = _simulator->step(state, a, o, r);

        //add to history
        _history.add(a,o);

        //update (discounted) reward
        totalR += r * discount;
        discount *= _simulator->getDiscount();

        //next state is current state
        delete state;
        state = nState;

        //check final
        final = _simulator->isFinal(state);
    }

    if (nState!=NULL) delete nState;

    return totalR;
}

Node* MCTS::expandNode(Node* node, State *state) {
    DEBUG_POMCP(cout<<"MCTS.expandNode"<<endl;);

    //set initial value and count
    _simulator->setInitialNodeValue(state, &_history, node);
    DEBUG_POMCP_SIM();

    //get actions
    vector<int> actions;
    _simulator->getActions(state,&_history,actions,true);

    //loop all actions
    DEBUG_POMCP_SIM(cout << "actions: "<<flush;);
    for(vector<int>::iterator it = actions.begin(); it != actions.end(); it++) {
        DEBUG_POMCP_SIM(cout << " "<<*it<<flush;);
        //create child
        NodeA* child = node->setChild(*it);

        //set value for child with it's action
        _simulator->setInitialNodeValue(state, &_history, child, *it);
        //note: in Silver's code: default sets all values to 0 of legal actions
        DEBUG_POMCP_SIM(cout<< "["<<child->toString()<<"]";);
    }
    DEBUG_POMCP_SIM(cout<<endl;);

    DEBUG_POMCP(cout<<"MCTS.expandNode done: "<<node->toString(false)<<endl;);

    return node;
}

#ifdef ORIG_UPDATE //the update as in the POMCP code of Silver

bool MCTS::update(int action, int obs, double reward) { //TODO: reward in update
    DEBUG_POMCP(cout<<"MCTS.update"<<endl;);

    //get child
    NodeA* nodeA = _root->getChild(action);
    Node* node = NULL;
    if (nodeA!=NULL) {
        node = nodeA->getChild(obs);
    }

    //create new belief
    //Belief* belief = NULL;

    //copy belief from node
    /*if (node!=NULL) {
        belief = node->getBelief(); //->copy();  //AG TODO: do we need a copy??
    }*/

    //TODO (?): if params.useTransforms ... //to avoid particel deprivation (in code Silver&Veness)

    if (node==NULL || node->getBelief()->isEmpty()) {
        DEBUG_POMCP(cout<<"MCTS.update failed, no belief points"<<endl;);

        return false; //i.e. not updated
    } else {
        State* state = NULL;
        /*if (node != NULL && !node->getBelief()->isEmpty()) {
            state = node->getBelief()[0];
        } else {
            state = (*belief)[0];
        }*/
        //get first state of belief //TODO: are the
        state = (*node->getBelief())[0];

        //now cut tree
        //TODO!: mabye we should delete the parents!!!!
        //delete _root;

        //create new root
        Node* newRoot = new Node(_simulator,NULL);
        expandNode(newRoot,state);
        //copy beliefs from old root
        //TODO: if using 'useTransform' -> maybe different setup,
        //      or keep track of seperate list of beliefs
        newRoot->getBelief()->copyFrom(_root->getBelief());

        //remove old root from mem
        delete _root;

        //set new root
        _root = newRoot;


        DEBUG_POMCP(cout<<"MCTS.update done belief expanded to: "<<_root->toString(false)<<endl;);

        return true;
    }
}
#else

bool MCTS::update(int action, int obs, double reward) { //TODO: reward in update, multiple observations
    DEBUG_POMCP(cout<<"MCTS.update"<<endl;);

    //get child
    NodeA* nodeA = _root->getChild(action);
    Node* node = NULL;
    if (nodeA!=NULL) {
        node = nodeA->getChild(obs);
    }

    //create new belief
    //Belief* belief = NULL;

    //copy belief from node
    /*if (node!=NULL) {
        belief = node->getBelief(); //->copy();  //AG TODO: do we need a copy??
    }*/

    //TODO (?): if params.useTransforms ... //to avoid particel deprivation (in code Silver&Veness)

    if (node==NULL || node->getBelief()->isEmpty()) {
        DEBUG_POMCP(cout<<"MCTS.update failed, no belief points"<<endl;);

        return false; //i.e. not updated
    } else {
        //State* state = NULL;
        /*if (node != NULL && !node->getBelief()->isEmpty()) {
            state = node->getBelief()[0];
        } else {
            state = (*belief)[0];
        }*/
        //get first state of belief //TODO: are the
        ///state = (*node->getBelief())[0];


        //now cut tree
        //TODO!: mabye we should delete the parents!!!!
        //delete _root;

        //create new root //TODO!!! !WHY DOESNT IT TAKE THE CHILD AS A ROOOT?????????? this way we can reuse part of the tree...
        Node* newRoot = new Node(_simulator,NULL);

        Belief* belief = newRoot->getBelief();

        //ag130731: first gen new belief
        FOR(i,_params->numInitBeliefStates) {
            //get random belief point
            State* state = _root->getBelief()->getRandomSample();

            int obsOut = -1;
            double rew = -1;

            //new state
            State* newState = _simulator->step(state, action, obsOut, rew, obs);

            //add new belief
            belief->add(newState);
        }

        //ag130731: gen new state consistent with prev belief
        State* state = newRoot->getBelief()->getRandomSample();


        expandNode(newRoot,state);
        //copy beliefs from old root
        //TODO: if using 'useTransform' -> maybe different setup,
        //      or keep track of seperate list of beliefs

        ///newRoot->getBelief()->copyFrom(_root->getBelief());

        //remove old root from mem
        delete _root;

        //set new root
        _root = newRoot;


        DEBUG_POMCP(cout<<"MCTS.update done belief expanded to: "<<_root->toString(false)<<endl;);

        return true;
    }
}
#endif

int MCTS::selectAction() {
    //if param: choose rolloutSearch / UCT search
    DEBUG_POMCP(cout << "MCTS.selectAction"<<endl;);

    //generate/update the tree
    uctSearch();

    DEBUG_POMCP_SIM(cout << "New tree: " << _root->toString(true)<<endl;);

    //get the action
    int a = greedyUCB(_root,false);

    DEBUG_POMCP(cout << "MCTS.selectAction: uctSearch, action="<<a<<endl;);
    return a;
}

