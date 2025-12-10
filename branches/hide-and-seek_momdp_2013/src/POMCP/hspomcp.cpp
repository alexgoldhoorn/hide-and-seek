#include "hspomcp.h"

#include "hssimulator.h"

#include "seekerhs.h"
#include "Utils/generic.h"
#include "hsconfig.h"

//iriutils
#include "exceptions.h"




using namespace pomcp;

HSPOMCP::HSPOMCP(SeekerHSParams* params, GMap* map) {
    assert(params!=NULL);

    if (params->rewardType==SeekerHSParams::REWARD_NOT_SET) {
        throw CException(_HERE_, "HSPOMCP requires reward type");
    }

    initRandomizer();

    _params = params;
    _map = map; //can be NULL, to be set in init
    _simulator = NULL;
    _mcts = NULL;
    _action = -1;
}

HSPOMCP::~HSPOMCP() {
    delete _mcts;
    delete _simulator;
}


bool HSPOMCP::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    DEBUG_POMCP(cout << "HSPOMCP.initBelief"<<endl;);
    assert(gmap!=NULL);
    assert(seekerInitPos.isSet());
    _map = gmap;

    DEBUG_POMCP(cout<<"Init simulator: "<<flush;);
    _simulator = new HSSimulator(gmap, _params);
    DEBUG_POMCP(cout<<"ok"<<endl<<"Init MCTS: "<<flush;);
    _mcts = new MCTS(_params, _simulator);
    _action = -1;
    DEBUG_POMCP(cout<<"ok"<<endl<<"Init simulator + MCTS: "<<flush;);

    //set init
    _simulator->setSeekerHiderPos(seekerInitPos, hiderInitPos, opponentVisible);
    //init mcts
    _mcts->init();


    DEBUG_POMCP(cout << "Initial BELIEF: " << _mcts->getRoot()->getBelief()->toString() << endl;);

    DEBUG_POMCP(cout <<"ok"<<endl<< "HSPOMCP.initBelief done"<<endl;);

    return true;
}


int HSPOMCP::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible) {

    int o = -1;

    if (_action!=-1) {
        //it is not the first action
        //get observation:
        o = _simulator->getObservation(seekerPos,hiderPos,opponentVisible);

        _mcts->update(_action, o, 0); //NOTE: reward not used

        DEBUG_POMCP(cout << "Updated BELIEF: " << _mcts->getRoot()->getBelief()->toString() << endl;);
    } //else: just initialized (with b0)

    _action = _mcts->selectAction();

    DEBUG_POMCP(cout << "HSPOMCP.getNextAction: "<<_action << " from: s"<<seekerPos.toString()<<",h"<<hiderPos.toString()<<",o="<<o<<")"<<endl;);

    return _action;
}

vector<int> HSPOMCP::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n) {
    vector<int> actions;

    throw CException(_HERE_,"HSPOMCP::getNextMultipleActions: to be implemented");

    return actions;
}

char HSPOMCP::getGameState(Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    assert(seekerInitPos.isSet());

    if (_simulator==NULL) {
        return 255; //no sim, not started yet
    } else {
        HSState state(seekerInitPos, hiderInitPos);
        return _simulator->getGameStatus(&state);
    }

}
