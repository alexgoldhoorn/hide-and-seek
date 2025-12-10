#include "hideseekmcvi.h"

#include "hsglobaldata.h"


#include "HideSeekModel.h"
#include "RandSource.h"
#include "PolicyGraph.h"
#include "MAction.h"
#include "MObs.h"

HideSeekMCVI::HideSeekMCVI(string policyFile) {
    _policyFile = policyFile;
    _model = NULL;
    _policy = NULL;
    _map = NULL;
    _randSource = NULL;
    _obs = NULL;
}

HideSeekMCVI::~HideSeekMCVI() {
    if (_model!=NULL)
        delete _model;
    if (_policy!=NULL)
        delete _policy;
    if (_randSource!=NULL)
        delete _randSource;
    if (_obs!=NULL)
        delete _obs;
}


bool HideSeekMCVI::initBelief(GMap *map, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible) {
    //set map
    _map = map;
    //create mode
    _model = new HideSeekModel(map);

    //load policy
    _policy = new PolicyGraph(1, _model->getNumObsVar());
    _policy->read(_policyFile);

    MAction::initStatic(_model);

    //random source
    int numTrials = _map->colCount()*_map->rowCount();
    _randSource = new RandSource(numTrials);


    /*ofstream fp;
    fp.open(trace_fn.c_str());
    if (!fp.is_open()){
        cerr << "Fail to open " << trace_fn << "\n";
        exit(EXIT_FAILURE);
    }*/

    // Variables for computing rewards
    /*double currReward;
    sumReward = sumDiscounted = 0;
    double currDiscount = 1;
*/
    // States
    _currState.resize(_model->getNumStateVar(),0);


    // Macro action

    // To keep track of policy graph state
    _obs = new MObs(vector<long>(_model->getNumObsVar(),0)); // observation

    int rootIndex = 0; //the index of policygraph init node, this is 0 for simulation by default
                        //in theory should be same for the policygraph
    _currGraphNode = _policy->getRoot(rootIndex);
    //PolicyGraph::Node *nextGraphNode;

    _action = &(_policy->getAction(_currGraphNode));


    _currDiscount = 1;
    _sumDiscounted = 0;
    _sumReward = 0;


/*

    _model->ini



    Simulator currSim(currModel, policyGraph, maxSimulLength);
    double avgReward, avgDiscounted;
    vector<State > trace;
    if (numTrials == 1){
        currSim.runSingle(maxSimulLength, avgReward, avgDiscounted, "trace.out", currModel.sampleInitState(), currRandSource);
        if (display != -1)
            cout << "Average Reward: " << avgReward << "   Average Discounted Reward: " << avgDiscounted << "\n";

        if (display == 1) cin.get();

    }else{
        double sumReward = 0;
        double sumDiscounted = 0;
        for (long i= 0; i<numTrials; i++){
            currSim.runSingle(maxSimulLength, avgReward, avgDiscounted, "trace.out", currModel.sampleInitState(), currRandSource);
            sumReward += avgReward;
            sumDiscounted += avgDiscounted;
        }
        cout << "Average Reward: " << sumReward/numTrials << "   Average Discounted Reward: " << sumDiscounted/numTrials << "\n";
    }*/

}

int HideSeekMCVI::getNextAction(Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible) {

    if (_model->isTermState(_currState)){
        //fp<<"success"<<endl;
        //break;
        return HSGlobalData::ACT_H; //if final state action is halt
    }

    double currReward = 0;
    long currMacroActState=0, nextMacroActState=0;
    MState nextState;

    if (_action->type == Initial){ // check node type
        currReward = _model->initPolicy(_currState, *_action, currMacroActState, nextState, nextMacroActState, *_obs, *_randSource);
        //currMacroActState = nextMacroActState;
    }
    else{
        currReward = _model->sample(_currState, *_action, nextState, *_obs, *_randSource);
        PolicyGraph::Node* nextGraphNode = _policy->getNextState(_currGraphNode, *_obs);
        _currGraphNode = nextGraphNode;
    }
    _sumReward += currReward;
    _sumDiscounted += _currDiscount * currReward;
    _currDiscount *= _model->getDiscount();
    _currState = nextState;
    _action = &(_policy->getAction(_currGraphNode));

}
