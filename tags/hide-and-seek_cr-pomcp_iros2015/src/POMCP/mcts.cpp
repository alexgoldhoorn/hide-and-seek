
#include <cassert>
#include <cmath>
#include <iostream>

#include "POMCP/mcts.h"
#include "POMCP/belief.h"

#include "seekerhs.h"
#include "Utils/generic.h"
#include "hsglobaldata.h"
#include "hsconfig.h"

//iriutils
#include "exceptions.h"

//for test only
#include "POMCP/hsstate.h"



//#include "POMCP/hspomcp.h"///


// TODO: could use RAVE (http://www.cs.utexas.edu/~pstone/Courses/394Rspring11/resources/mcrave.pdf)

using namespace pomcp;
using namespace std;

MCTS::MCTS(SeekerHSParams* params, Simulator* simulator, PersonPathPredConsumer* personPathPredConsumer) :
    _randomGenerator(_randomDevice()), _uniformProbDistr(0,1)
{
    //check params
    /*if (params->numBeliefStates <= 0)
        throw CException(_HERE_, "number of initial belief states should be at least 1");*/
    if (params->numSim <= 0)
        throw CException(_HERE_, "number of simulations should be at least 1");
    if (params->maxTreeDepth <= 0)
        throw CException(_HERE_, "maximum tree depth should be at least 1");
    /*if (params->numInitBeliefStates<1)
        throw CException(_HERE_, "number of initial belief states should be at least 1");*/
    if (params->expandCount<0)
        throw CException(_HERE_, "expand count should be 0 or higher");
    if (params->explorationConst<0)
        throw CException(_HERE_, "exploration constant should be 0 or higher");

    //set init values
    _simulator = simulator;
    _params = params;
    _personPathPredConsumer = personPathPredConsumer;
    _treeDepth = 0;
    _root = NULL;
}


//AG150122: added obs 2 of 2nd seeker
void MCTS::init(const State* initObs, const State* initObs2, double obs1p) {
    DEBUG_POMCP(cout<<"MCTS.init"<<endl;);
    assert(_root==NULL);

    //AG131104: initObs not necesarilly has to be not NULL

    //generate root
    State* initState = _simulator->genInitState(initObs, initObs2, obs1p);
    assert(initState!=NULL);

    DEBUG_POMCP(cout<<" init state: "<< initState->toString();
            if (initObs2!=NULL) cout<<", init state of seeker2: "<<initObs2->toString();
            cout <<endl;
    );

    _root = new Node(_simulator,NULL);
    expandNode(_root, initState);

    //AG150216: not used anymore
    delete initState;

    DEBUG_POMCP(cout<<" expanded root node: "<< _root->toString(false)<<endl;);
    Belief* belief = _root->getBelief();

    /*if (_params->numBeliefStates == 0) { //AG131031: add all belief states
        vector<State*> beliefStateVec = _simulator->genAllInitStates(initObs);
        belief->addAll(beliefStateVec,true);
    } else {
        //add a limited amount of randomly chosen states
        FOR(i,_params->numBeliefStates) {
            State* state = _simulator->genInitState(initObs);
            belief->add(state);
        }
    }*/

    //AG140303: changed, tested for n in getallInitStates
    vector<State*> beliefStateVec = _simulator->genAllInitStates(initObs,_params->numBeliefStates,initObs2,obs1p);

    /*cout <<"From init obs " << initObs->toString()<<" generated belief states: #"<<beliefStateVec.size()<<endl<<"out of map:"<<endl;
    cout << "_map is null="<<(_map==NULL)<<endl;
    for(State* s : beliefStateVec) {
        HSState* hs = static_cast<HSState*>(s);
        cout <<" hs:"<<(hs==NULL?"NULL":hs->toString())<<endl;
        if (!_map->isPosInMap(hs->hiderPos)) {
            cout<<hs->toString()<<" "<<flush;
        }
    }
    cout<<endl;*/

    //add belief states and delete this copy
    belief->addAll(beliefStateVec,true);

    DEBUG_POMCP(cout<<" Init belief: "<<flush<<belief->toString(DEBUG_NUM_BELIEF_TOSTRING)<<endl;);

    assert(belief->size()>0);

#ifdef GUI_DEBUG
    cout << "Connecting signal-slot:"<<flush;
    //_treeWindow.show();
    //TreeWindow::instance()->showTree(_root);
    connect(this, SIGNAL(actionChosen(pomcp::Node*,int)), TreeWindow::instance(), SLOT(showTree(pomcp::Node*,int)));
    cout << "OK"<<endl;
#endif

    DEBUG_POMCP(cout<<"MCTS.init done"<<endl;);
}

MCTS::~MCTS() {
    if (_root!=NULL) {
        //AG140424: also delete children
        _root->deleteChildrenExceptFor(NULL);
        //delete root
        delete _root;
    }
}


pomcp::Node* MCTS::getRoot() {
    return _root;
}


void MCTS::uctSearch() {
    //history depth
    size_t historyDepth = _history.size();
    DEBUG_POMCP(cout<<"MCTS.uctSearch (histDepth="<<historyDepth<<"; #sims="<<_params->numSim<<")"<<endl;);

    //set parameters that are used in simulator (might have been changed externally, e.g. by ROS)
    _simulator->resetParams();

    //do the simulations
    FOR(i,_params->numSim) {
        DEBUG_POMCP_SIM(cout << " sim#"<<i<<flush;);
        //get random state
        State* state = _root->getBelief()->getRandomSample(); //->copy(); //TODO: choose based on belief(!)
        DEBUG_POMCP_SIM(cout << " " <<state->toString()<<endl;);

        //new simulation
        _treeDepth = 0;
        /*if (_personPathPredConsumer != NULL)
            _personPathPredConsumer->resetTime(); //AG140909: reset time of path prediction consumer*/
        //AG140910: reset path of simulator
        _simulator->resetTimer();

        double totalR = simulateNode(_root, state);
        DEBUG_POMCP_SIM(cout <<" r="<<totalR<<endl;);

        //trunctate history to go back to 'start' of simulation
        _history.truncate(historyDepth);
    }

    DEBUG_POMCP(cout<<"MCTS.uctSearch done"<<endl;);
}


double MCTS::simulateNode(Node *node, State *state) {
    DEBUG_POMCP_SIM(cout<<" simN,td="<<_treeDepth<<",node="<<node->toString(true)<<endl;);
    //PeekTreeDepth = _treedepth
    if ((int)_treeDepth > _params->maxTreeDepth) { //ag130730: max -> exclusive
        //reached the maximum tree depth
        DEBUG_POMCP_SIM(cout<<" maxDepth "<<endl;);
        return 0;
    }

    //get greedy action for current state
    int action = greedyUCB(node,true);

    if (_treeDepth == 1) {
        //add a sample to belief
        node->getBelief()->add(state);
        DEBUG_POMCP_SIM(cout<<" addBelief:"<<state->toString()<<flush;);
    }

    DEBUG_POMCP_SIM(cout <<"-> action: "<<ACTION_COUT(action)<< endl;);

    //get child
    NodeA* nodeA = node->getChild(action);
    assert(nodeA!=NULL); //should exist since it is created in the ExpandNode called by simlateNodeA for new nodes
    /*if (nodeA == NULL) {
        nodeA = node->createChild(action);
    }*/

    //simulate node
    double totalR = simulateNodeA(nodeA, state, action);

    //add reward for current node
    node->addValue(totalR);

    DEBUG_POMCP_DETAIL(cout<<"simN: received totalR="<<totalR<<",new node value: "<<node->toString()<<endl;);

    return totalR;
}


double MCTS::simulateNodeA(NodeA *nodea, State *state, int action) {
    State* obs;
    double immediateReward = 0, delayedReward = 0;

    //get new state, o and r
    State* nstate = _simulator->step(state, action, obs, immediateReward);

    assert(obs!=NULL);
    bool final = _simulator->isFinal(nstate);

    //add to history
    _history.add(action, obs->copy());

    //AG140320: convert observation to obs
    obs->convertToObservation();

    //get node
    Node* nnode = nodea->getChild(obs);

    if (nnode == NULL && !final && (int)nodea->getCount()>=_params->expandCount) {
        //decide to create and to expand the node
        nnode = nodea->createChild(obs);
        expandNode(nnode, nstate);
        DEBUG_POMCP_SIM(cout<<" expanded: "<<nnode->toString()<<flush;);
    }

    DEBUG_POMCP_SIM(cout<<" simNA,a="<<ACTION_COUT(action)<<",o="<<obs->toString()<< ",s'="
                    <<(nstate==NULL?"NULL":nstate->toString())<< ",final="<<final<<",r="<<immediateReward <<flush;);
    DEBUG_POMCP_SIM(cout<< " " << (nnode==NULL?"[NULL]":nnode->toString() )<<endl;);

    if (!final) {
        //calculate reward until 'end'
        _treeDepth++;
        DEBUG_POMCP_DETAIL(cout <<"TreeDepth="<<_treeDepth<<endl;);
        /*if (_personPathPredConsumer != NULL)
            _personPathPredConsumer->increaseTime(); //AG140909: increase time of predictor */
        _simulator->increaseTimer(); //AG140910: increase time

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
    delete obs;

    //calc reward
    double totalR = 0;

    //calculate the expected reward
    switch (_params->expectedRewardCalc) {
    case SeekerHSParams::EXPECTED_REWARD_CALC_SUM:
        totalR = immediateReward + _simulator->getDiscount()*delayedReward;
        break;
    case SeekerHSParams::EXPECTED_REWARD_CALC_NORM:
        totalR = (immediateReward + _simulator->getDiscount()*delayedReward) / (1+_simulator->getDiscount());
        break;
    default:
        throw CException(_HERE_,"unexpected expected reward calc type");
    }

    DEBUG_POMCP_SIM(cout<<"simNA: immR="<<immediateReward<<",delayedR="<<delayedReward<<",totalR="<<totalR<<endl;);

    nodea->addValue(totalR);
    return totalR;
}

int MCTS::greedyUCB(Node *node, bool ucb) {
    assert(node!=NULL);

    //init bestactions
    vector<int> bestActions;
    double bestActValue = -HSGlobalData::INFTY_POS_DBL;

    //precalc N and log(N)
    unsigned int N = node->getCount();
    double logN = log(N); //AG131014: changed to from    log(N+1); //AG: why N+1 -> because log(0)=-inf
                            //because it may give big differences, and if N=0 than no advantage can be made...(?)

    DEBUG_POMCP_DETAIL(cout <<"GreedyUCB (ucb="<<ucb<<"), N="<<N<<", log(N)="<<logN<<flush;);

    //init v and n
    double v = 0;
    unsigned int n = 0;

    //for each action calc expected reward

    FOR(a,_simulator->getNumActions()) {
        //get node for action
        NodeA* anode = node->getChild(a);
        /*if (anode == NULL) {
            anode = node->createChild(a);
        }*/ // node should have been made by expand (?)

        //AG131008: nodes can be null for actions that are not possible
        //assert(anode!=NULL);
        if (anode==NULL) {
            continue; //skip, since this action was not possible (as found out in expandNode)
        }

        //if (anode!=NULL) { //AG note: child now always created, if NULL->new
            //get n and value
            v = anode->getValue();
            n = anode->getCount();

            DEBUG_POMCP_DETAIL(cout<<" a="<<ACTION_COUT(a)<<": v="<<v<<",n="<<n<<flush;);


            if (ucb && N>0) { //add exploration value
                if (n==0) {
                    v += INFINITY;
                } else {
                    v += _params->explorationConst * sqrt(logN / n);
                }
                DEBUG_POMCP_DETAIL(cout<<",v+ucb="<<v<<flush;)
            }

            //cout <<"a="<<a<<"; v="<<v<<"; bestv="<<bestActValue<<endl;
            if (v>=bestActValue) {
                //'best' value
                if (v>bestActValue) { //'better' value
                    bestActions.clear();
                }
                //add action
                bestActValue = v;
                bestActions.push_back(a);                
            }
        //} //else: action not calculated!! (maybe show warning)
    }

    //assume at least 1 action best, but COULD be that
    assert(bestActions.size()>0);

    if (bestActions.size()==1) {
        return bestActions[0];
    } else {
        //get random action
        return bestActions[random(bestActions.size()-1)];
    }
}

//TODO: here ALSO strategy of 'smart seeker'

double MCTS::rollout(const State *sstate) {
    //accumulated discount
    double accDiscount = 1.0;
    //discount factor
    double discount = _simulator->getDiscount();
    //total (accumulated) reward
    double totalR = 0;
    //final state boolean
    bool final = false;
    //start state
    State* state = sstate->copy();
    //next state
    State* nState = NULL;
    //action
    int a;
    //observation
    State* obs;
    //immediate reward
    double r;
    //immediate reward vector
    vector<double> imRewardVec;


    DEBUG_POMCP_SIM(cout<<" rollout from "<<sstate->toString()<<flush;);

    //do a simulation rollout
    //for(unsigned i=0; i<_params.numSim && !final; i++) {
    for (int numSteps = 0; numSteps + (int)_treeDepth < _params->maxTreeDepth && !final; numSteps++) {
        DEBUG_POMCP_DETAIL(cout << " |s"<< numSteps<<":"<< flush;);
        //get possible actions
        /*vector<int> actions;
        _simulator->getActions(state, &_history, actions);

        //choose random action
        a = actions[random(actions.size()-1)];
        */
        //get the roll-out action following its specific policy
        a = _simulator->getActionForRollout(state, &_history);

        //do step
        nState = _simulator->step(state, a, obs, r);

        //add to history
        _history.add(a,obs);

        //calculate the expected reward
        switch (_params->expectedRewardCalc) {
        case SeekerHSParams::EXPECTED_REWARD_CALC_SUM: {
            //update (discounted) reward
            totalR += r * accDiscount;
            accDiscount *= discount;
            break;
        }
        case SeekerHSParams::EXPECTED_REWARD_CALC_NORM:
            imRewardVec.push_back(r);
            break;
        default:
            throw CException(_HERE_,"unexpected expected reward calc type");
        }

        //next state is current state
        delete state;
        state = nState;

        //check final
        final = _simulator->isFinal(state);
        DEBUG_POMCP_DETAIL(cout << "a="<<ACTION_COUT(a)<<"->"<<nState->toString()<<";final="<<final<<";r="
                           <<r<<";nxt disc="<<accDiscount<<";totalR="<<totalR <<flush;);
    }

    //calculate reward if using normalization
    if (_params->expectedRewardCalc == SeekerHSParams::EXPECTED_REWARD_CALC_NORM && imRewardVec.size()>0) {
        int lastIndex = imRewardVec.size()-1;
        totalR = imRewardVec[lastIndex];
        //now loop from end to top to calc totalR
        for (int i=lastIndex-1; i>=0; i--) {
            totalR = (imRewardVec[i] + discount*totalR) / (1+discount);
        }
    }

    DEBUG_POMCP_DETAIL(cout<<endl;);

    //AG150216: state always has to be deleted! (if passed through for or not)
    //if (nState!=NULL) delete nState;
    delete state;


    return totalR;
}

pomcp::Node* MCTS::expandNode(Node* node, const State *state) {
    DEBUG_POMCP_SIM(cout<<"MCTS.expandNode"<<endl;);
    assert(node!=NULL);
    assert(state!=NULL);

    //set initial value and count
    if (_params->setInitNodeValue) {
        _simulator->setInitialNodeValue(state, &_history, node);
    }
    //node->setCountAndValue(0,0);
    //DEBUG_POMCP_SIM();

    //creating all children
    //AG131008: instead of creating ALL actions, create ONLY the ones of the legal actions!
    /*for(int a=0; a<_simulator->getNumActions(); a++) {
        NodeA* child = node->getChild(a);
        if (child==NULL) {
            child = node->createChild(a);
        }
    }*/

    //get actions which are legal here
    vector<int> actions;
    _simulator->getActions(state,&_history,actions,true);

    if (actions.size()==0) {
        //AG131009: at least action should be possible
        throw CException(_HERE_,"MCTS.expandNode: at least one action should be possible (retrieved from Simulator.getActions()");
    }

    //loop all actions
    DEBUG_POMCP_SIM(cout << "actions: "<<flush;);

    //for the children for which
    for(vector<int>::iterator it = actions.begin(); it != actions.end(); it++) {
        DEBUG_POMCP_SIM(cout << " "<<*it<<flush;);

        NodeA* child = node->getChild(*it);
        if (child==NULL) {
            child = node->createChild(*it);
        }

        //set value for child with it's action
        if (child->getCount()==0 && _params->setInitNodeValue) {
            //ag130830: if no value has been set yet, then create an initial value
            //ag131022: and if param set node init value
            _simulator->setInitialNodeValue(state, &_history, child, *it);
        }
        //child->setCountAndValue(0,0);

        //note: in Silver's code: default sets all values to 0 of legal actions
        DEBUG_POMCP_SIM(cout<< "["<<child->toString()<<"]";);
    }
    DEBUG_POMCP_SIM(cout<<endl;);

    DEBUG_POMCP_SIM(cout<<"MCTS.expandNode done: "<<node->toString(false)<<endl;);

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
#elif defined(MY_UPDATE1)

/* AG130805: version adapted by AG:
  Try to get the node from the tree: action,obs
  If the node exists and contains a belief, then use that,
  if no: generate beliefs from the root belief doing obs o and action a
  *//*
TODO: check belief update
if started hidden, then when going visible the simulated value is NOT same as observation
if starting visible, the hider's location is consistent!
Check how to update.
and also if started hidden..sometimes actions inconsistent..
check why delay or not correct updated
and maybe for bigger fields for when hidden.. necessary more random samples*/
bool MCTS::update(int action, State* obs, double reward) { //TODO: reward in update, multiple observations
    DEBUG_POMCP(cout<<"MCTS.update (obs="<<obs<<")"<<endl;);

    //get child for obs and action (if exists)
    NodeA* nodeA = _root->getChild(action);
    Node* node = NULL;
    if (nodeA!=NULL) {
        node = nodeA->getChild(obs);
    }


    //TODO (?): if params.useTransforms ... //to avoid particel deprivation (in code Silver&Veness)


    Node* newRoot = new Node(_simulator,NULL);
    Belief* newBelief = newRoot->getBelief();

    //if (node==NULL || node->getBelief()->isEmpty()) {
        //no belief for node, generate belief starting from root belief, doing action and getting observation (as passed as param)
        DEBUG_POMCP(cout << "Generating new belief points"<<endl;);

        //ag130731: first gen new belief
        FOR(i,_params->numInitBeliefStates) {
            //get random belief point
            State* state = _root->getBelief()->getRandomSample();

            State* obsOut = NULL;
            double rew = -1;

            //new state
            State* newState = _simulator->step(state, action, obsOut, rew, obs);

            delete obsOut;

            //add new belief
            newBelief->add(newState);
        }
        //AG130827: only add states that are consistent with obs
    /*} else {
        DEBUG_POMCP(cout << "Choosing belief from node, choosing from: " << node->getBelief()->toString()<<endl;);

        //copy beliefs from node
        //TODO: should use this node as root!!!!
        ///belief->copyFrom(node->getBelief());

        //AG130806: I'm filtering the numinitbeliefsttates, by getting random numInitbeliefstates

        FOR(i,_params->numInitBeliefStates) {
            //get random belief point
            State* state = node->getBelief()->getRandomSample();

            //add new belief
            newBelief->add(state->copy());
        }

    }*/

//this works but has delay of 1 step .. so it should look 1 step in future....


    /*if (node==NULL || node->getBelief()->isEmpty()) {
        DEBUG_POMCP(cout<<"MCTS.update failed, no belief points (node=null:"<<(node==NULL)<<";id="<<node->getNodeID() <<endl;);

        return false; //i.e. not updated
    } else {*/
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




        //ag130731: gen new state consistent with prev belief
        State* state = newBelief->getRandomSample();


        expandNode(newRoot,state);
        //copy beliefs from old root
        //TODO: if using 'useTransform' -> maybe different setup,
        //      or keep track of seperate list of beliefs

        ///newRoot->getBelief()->copyFrom(_root->getBelief());

        //remove old root from mem
        delete _root;

        //set new root
        _root = newRoot;

        //DEBUG_POMCP(cout<<" Updated belief: "<<newBelief->toString(-1)<<endl;);


        DEBUG_POMCP(cout<<"MCTS.update done belief expanded to: "<<_root->toString(false)<<endl;);

        return true;
    //}
}

#else //MY_UPDATE2

/* AG130805: version adapted by AG:
  Try to get the node from the tree: action,obs
  If the node exists and contains a belief, then use that,
  if no: generate beliefs from the root belief doing obs o and action a
  *//*
TODO: check belief update
if started hidden, then when going visible the simulated value is NOT same as observation
if starting visible, the hider's location is consistent!
Check how to update.
and also if started hidden..sometimes actions inconsistent..
check why delay or not correct updated
and maybe for bigger fields for when hidden.. necessary more random samples*/
bool MCTS::update(int action, const State* obs, double reward, const State* obs2, double obs1p /*, HSPOMCP* hspomcp*/) {
    //TODO: reward in update, multiple observations
    DEBUG_POMCP(
        cout<<"MCTS.update (action="<<ACTION_COUT(action)<<"; obs="<<obs->toString();
        if (obs2!=NULL) cout <<"; obs2="<<obs2->toString();
        cout<<")"<<endl;
    );

    //get child for obs and action (if exists)
    NodeA* nodeA = _root->getChild(action);
    Node* node = NULL;
    if (nodeA!=NULL) {
        //first make sure that the observations are readable
        State* obsState = obs->copy();
        obsState->convertToObservation();
        //retrieve the node through the observation
        node = nodeA->getChild(obsState);

        //AG140424: missing delete after copy
        delete obsState;
    }

    //new root
    Node* newRoot = NULL;    

    //check if node existed
    if (node!=NULL) {
        //take root
        newRoot = node;

        DEBUG_POMCP(cout << "POMCP::Update: using existing node as root: " << node->toString() <<endl;);

        if (_params->pomcpFilterBeliefAtUpdate) {
            //now check consistency of the belief points
            Belief* beliefs = node->getBelief();

            //check all beliefs
            for(size_t i=0; i<beliefs->size(); i++) {
                //one belief
                if (!_simulator->isStateConsistentWithObs((*beliefs)[i], obs, obs2)) {
                    //not consistent, so remove
                    DEBUG_POMCP_DETAIL(cout << "WARNING: inconsistent belief: "<<(*beliefs)[i]->toString()<<" with obs="<<obs->toString()<<" - ";);

                    //AG150225: if using cont. there is a probability that we keep the (potentially) false negative/positive
                    //          NOTE: we should detect if it is a false neg/pos (check if isSet?)
                    if (_params->pomcpSimType==SeekerHSParams::POMCP_SIM_CONT &&
                            _uniformProbDistr(_randomGenerator)<_params->contUpdInconsistAcceptProb //AG150319, changed from contFalseNegProb
                        ) {
                        DEBUG_POMCP_DETAIL(cout << "keeping as false neg/pos"<<endl;);
                    } else {
                        DEBUG_POMCP_DETAIL(cout << "removing"<<endl;);
                        beliefs->remove(i);
                        i--;
                    }
                }
                //TODO: we can make it more efficient
            }

            /*if (_params->numBeliefStates>0) {
                while (beliefs->size() > _params->numBeliefStates) {
                    int i = random(beliefs->size()-1);
                    beliefs->remove(i);
                }
            }*/

            DEBUG_POMCP(cout<< "after filtering, #beliefs="<<beliefs->size()/*<<":"<<beliefs->toString(10)*/<<endl;);
        }

    } else {        
        //create a new root
        newRoot = new Node(_simulator, NULL);

        DEBUG_POMCP(cout << "POMCP::Update: using NEW node as root: " << newRoot->toString() <<endl;);
    }

    //the new belief
    Belief* newBelief = newRoot->getBelief();

    /*if (hspomcp!=NULL) {
        Node* oldRoot = _root;
        _root = new Node(_simulator, NULL);
        for(int i=0;i<newBelief->size();i++) {
            _root->getBelief()->add((*newBelief)[i]);
        }
        hspomcp->storeMapBeliefAsImage("beforegennewuupdate.png",Pos(0,0),NULL,Pos(),NULL);
        delete _root;
        _root = oldRoot;
    }*/

    if (_params->numBeliefStates > 0) {
        //add beliefs to fill in until the preset number
        if (newBelief->size()<_params->numBeliefStates) {
            //old belief
            Belief* oldBelief = _root->getBelief();

            DEBUG_POMCP(cout << "Generating new belief points, #beliefs="<< newBelief->size()<<", from old belief:#="<<oldBelief->size()<<endl;);

            //counter to check how often inconsistent found (debug purposes)
            unsigned int c = 0, totC = 0;

            //ag130731: first gen new belief
            for(size_t i=newBelief->size(); i</*oldBelief->size() */_params->numBeliefStates; i++) {
                //get random belief point
                State* state = oldBelief->getRandomSample();

                State* obsOut = NULL;
                double rew = 0;

                //new state
                State* newState = _simulator->step(state, action, obsOut, rew, false, obs, obs2, obs1p);

                assert(obsOut==NULL);

                if (_params->pomcpSimType!=SeekerHSParams::POMCP_SIM_CONT ||
                        _params->contUpdInconsistAcceptProb==0) { //AG150325: when not allowing inconsistencies, remove them

                    if (newState == NULL) {
                        //no consistent state found
                        //remove state from previous root since we don't want to use them again
                        oldBelief->remove(state);

                        if (oldBelief->size() == 0) {
                            cout << "WARNING: no consistent old belief"<<endl;
                            break;
                        }

                        //increase counter
                        c++; totC++;
                        if (c>=_params->numBeliefStates) {
                            cout << "WARNING: PASSED "<<_params->numBeliefStates<<" times through loop generation without finding consistent states, oldbelief.size="
                                 <<oldBelief->size()<<endl;
                            //TODO: handle this case!!
                            c=0;
                        }
                        //reduce index since we didn't create any
                        i--;
                        //continue;
                    } else {
                        c = 0;
                    }
                }

                //add new belief
                if (newState != NULL) {
                    newBelief->add(newState,true);
                }
            }

            DEBUG_POMCP(cout << "New belief points generated, new size="<<newBelief->size() <<endl;);

            //we should either have no belief (if old belief was completely inconsistent), or there should be an old belief
            //and the required number of belief states
            //assert(oldBelief->size()==0 && newBelief->size()==0 || oldBelief->size()>0 && newBelief->size()==_params->numBeliefStates);

        }
    } /*else*/

    if (newBelief->size()==0) {
        DEBUG_POMCP(cout << "WARNING: POMCP::update: for new belief no states propogated, re-initializing belief"<<endl;);
        //ag131104: set belief based on obs
        vector<State*> beliefStateVec = _simulator->genAllInitStates(obs,_params->numBeliefStates,obs2,obs1p);
        newBelief->addAll(beliefStateVec,true);
    }

    assert(newBelief->size()>0);

    if (node == NULL) {
        //created a new node, so expand it

        //ag130731: gen new state consistent with prev belief
        State* state = newBelief->getRandomSample();

        expandNode(newRoot,state);
    }

    //now delete the old root and its children, except for the new root
    _root->deleteChildrenExceptFor(newRoot);
    //remove old root from mem
    delete _root;

    //set new root
    _root = newRoot;

    DEBUG_POMCP(cout<<"MCTS.update done belief expanded to: "<<_root->toString(false)<<endl;);

    return true;
}
#endif

int MCTS::selectAction() {
    //if param: choose rolloutSearch / UCT search
    DEBUG_POMCP(cout << "MCTS.selectAction"<<endl;);

    //generate/update the tree
    uctSearch();

    //cout << " after uct search"<<endl;
    //cout << "root==NULL:"<<(_root==NULL)<<endl;
#ifdef DEBUG_POMCP_DETAIL_ON
    DEBUG_POMCP(cout << "New tree: " << flush << _root->toString(true)<<endl;);
#else
    DEBUG_POMCP(cout << "New tree: " << flush << _root->toString(false)<<endl;);
#endif

    //ag130818: check tree
    bool check = checkTree(_root);
    if (!check) {
        cout << "MCTS::selectAction: ERROR: tree found not to be correct"<<endl;
        //EXIT?
    }

    //get the action
    int a = greedyUCB(_root,false);

#ifdef GUI_DEBUG
    cout << "Sending msg:"<<flush;
    //_treeWindow.show();
    //TreeWindow::instance()->showTree(_root, a);
    emit actionChosen(_root,a);
    cout << "OK"<<endl;
    //TODO: wait
#endif

    DEBUG_POMCP(cout << "MCTS.selectAction done: uctSearch, action="<<ACTION_COUT(a)<<endl;);
    return a;
}

vector<int> MCTS::getNextMultipleActions(int n) {
    assert(n>0);

    //init with HALT
    vector<int> nextActVector(n,HSGlobalData::ACT_H);

    //learn policy tree and get first action
    nextActVector[0] = selectAction();


    Node* root = _root;
    //find the next (future) actions
    for(int i=1; root!=NULL && i<n; i++) {
        //get node for action
        NodeA* nodeA = root->getChild(nextActVector[i-1]);
	assert(nodeA!=NULL);

        //now choose most probable (i.e. highest count) child obs
        vector<Node*> maxChildVec;
        unsigned int maxC = 0;
        for(int j=0; j<(int)nodeA->childCount(); j++) {
            ObsNodePair* pair = nodeA->getChildItem(j);
            unsigned int c = pair->node->getCount();
            if (c>=maxC) {
                //better or equal score
                if (c>maxC) {
                    maxChildVec.clear();
                    maxC = c;
                }
                maxChildVec.push_back(pair->node);
            }
        }

        //now get the child
        if (maxChildVec.size()==0) {
            root = NULL; //no child found
	   //cout << "POCMP.nextChild, no child found on index "<<i<<endl;
        } else if (maxChildVec.size()==1) {
            root = maxChildVec[0];
        } else {
            root = maxChildVec[random(maxChildVec.size()-1)];
        }

        //choose next action
        if (root!=NULL) {
            nextActVector[i] = greedyUCB(root,false);
        }
    }

    return nextActVector;
}

bool MCTS::checkNode(Node *node, Node *nextNode, int a, const State* obs) {
    bool ok = true;

    //beliefs
    Belief* belief = nextNode->getBelief();
    for(size_t i=0; i<belief->size(); i++) {
        if (!_simulator->checkNextStateObs(NULL, (*belief)[i], obs)) {
            ok = false;
        }
    }

    return ok;
}

bool MCTS::checkTree(Node* node) {
    assert(node!=NULL);
    bool ok = true;

    //children
    for (int a=0; a<(int)node->childCount(); a++) {
        if (node->isChildSet(a)) {
            NodeA* nodeA = node->getChild(a);
            if (nodeA == NULL) {
                ok = false;
                cout << "MCTS::checkTree: action "<<a<<" is set, but getChild is NULL"<<endl;
            } else {
                //for (int o=0; o<nodeA->childCount(); o++) {
                //for (map<State*,Node*>::iterator it = nodeA->itBeginChild(); it != nodeA->itEndChild(); it++) {
                for (map<string,ObsNodePair>::iterator it = nodeA->itBeginChild(); it != nodeA->itEndChild(); it++) {
                    //if (nodeA->isChildSet(o)) {
                        State* obs = it->second.observation;
                        Node* nodeChild = it->second.node; //nodeA->getChild(o);       //TODO: use State*
                        if (nodeChild==NULL) {
                            ok = false;
                            cout << "MCTS::checkTree: observation " << obs->toString() << " is set, but NULL"<<endl;
                        } else {

                            if ( !checkNode(node, nodeChild, a, obs) ) {
                                ok = false;
                            }

                            if ( !checkTree(nodeChild)) {
                                ok = false;
                            }
                        }
                    /*} else {
                        //not set..
                    }*/
                }
            }
        }
    }
    //for a in node.act
    //for o in node.obs

    return ok;
}

void MCTS::testNodes(GMap* map) {
    cout<<"Generate root:"<<flush;
    Node* root = new Node(_simulator, NULL);
    cout <<"ok"<<endl<<"gen children:"<<flush;
    //State* state = _simulator->genInitState();
    HSState state;
    state.seekerPos = map->genRandomPos();
    state.hiderPos = map->genRandomPos();
    cout <<" state:"<<state.toString()<<".."<<flush;

    expandNode(root,&state);
    cout<<"ok"<<endl;

    for(int q=0; q<10; q++) {
        for(int a=0;a<(int)root->childCount();a++) { cout << "a="<<a<<flush;
            NodeA* nodeA = root->getChild(a);
            if (nodeA!=NULL) {
                for(int i=0;i<10;i++) {
                    HSState obs;
                    obs.seekerPos = map->genRandomPos();
                    obs.hiderPos = map->genRandomPos();
                    cout << " o="<<obs.toString()<<flush;
                    Node* node = nodeA->getChild(&obs);

                    if (node==NULL){
                        cout <<"#"<<flush;
                        node = nodeA->createChild(&obs);
                        cout <<"."<<flush;
                        expandNode(node,&obs);
                        cout <<"x"<<flush;
                    } else cout << "!"<<flush;
                }cout <<endl;
            }
        }

        cout <<"update root:"<<flush;
        NodeA* nodeAR = NULL;
        while (nodeAR==NULL) nodeAR = root->getChild( random(root->childCount()-1) );
        Node* nodeR = nodeAR->getChildItem( random(nodeAR->childCount()-1))->node;
        cout << nodeR->toString()<<endl;
        cout << "delete old:"<<flush;
        root->deleteChildrenExceptFor(nodeR);
        delete root;
        root = nodeR;
        cout << "ok"<<endl;
    }

    cout<<"TREE:"<<endl<<root->toString(true)<<endl;

    cout<<"delete all: "<<flush;
    //delete state;
    root->deleteChildrenExceptFor(NULL);
    cout<<" children .."<<flush;
    delete root;
    cout<<"root, all ok"<<endl;

}
