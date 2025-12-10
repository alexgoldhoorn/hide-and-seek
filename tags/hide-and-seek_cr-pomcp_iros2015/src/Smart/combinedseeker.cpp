
#include <sstream>
#include <iostream>

#include "Smart/combinedseeker.h"
#include "hsglobaldata.h"


using namespace std;

CombinedSeeker::CombinedSeeker(SeekerHSParams* params, AutoPlayer *autoPlayerForVisib, AutoPlayer *autoPlayerForNotVisib) :
    AutoPlayer(params, autoPlayerForNotVisib->getMap()), _autoPlayerForVisib(autoPlayerForVisib), _autoPlayerForNotVisib(autoPlayerForNotVisib)
{
    assert(autoPlayerForVisib!=NULL);
    assert(autoPlayerForNotVisib!=NULL);
    _action = -1;
}

CombinedSeeker::~CombinedSeeker(){
}

bool CombinedSeeker::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    DEBUG_CLIENT(cout << "CombinedSeeker::initBelief: "<<endl;);
    DEBUG_CLIENT(cout << "> Combi - Opp visib - init <"<<endl;);
    bool initOkV = _autoPlayerForVisib->initBelief(gmap, seekerInitPos, hiderInitPos, opponentVisible);
    DEBUG_CLIENT(cout << "> Combi - Opp not visib - init <"<<endl;);
    bool initOkNV = _autoPlayerForNotVisib->initBelief(gmap, seekerInitPos, hiderInitPos, opponentVisible);

    return initOkNV && initOkV;
}


int CombinedSeeker::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    DEBUG_CLIENT(cout << "CombinedSeeker::getNextAction: "<<endl;);
    if (opponentVisible) {
        DEBUG_CLIENT(cout << "> Combi - Opp visib <"<<endl;);
        _action = _autoPlayerForVisib->getNextAction(seekerPos,hiderPos,opponentVisible,actionDone); //TODO: !!! pass action done to getNextAction !!!! the action really done.. to do the update...
        DEBUG_CLIENT(cout << "> Combi - for visib [used], action="<<ACTION_COUT(_action)<<endl;);
        int actNotVisib = _autoPlayerForNotVisib->getNextAction(seekerPos,hiderPos,opponentVisible,actionDone);
        DEBUG_CLIENT(cout << "> Combi - for not visib [not used], action="<<ACTION_COUT(actNotVisib)<<endl;);
    } else {
        DEBUG_CLIENT(cout << "> Combi - Opp not visib <"<<endl;);
        _action = _autoPlayerForNotVisib->getNextAction(seekerPos,hiderPos,opponentVisible,actionDone);
        DEBUG_CLIENT(cout << "> Combi - for not visib [used], action="<<ACTION_COUT(_action)<<endl;);
    }

    return _action;
}

vector<int> CombinedSeeker::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone, int n) {
    DEBUG_CLIENT(cout << "CombinedSeeker::getNextMultipleActions: "<<endl;);
    vector<int> actions;

    if (opponentVisible) {
        DEBUG_CLIENT(cout << "> Combi - Opp visib <"<<endl;);
        actions = _autoPlayerForVisib->getNextMultipleActions(seekerPos,hiderPos,opponentVisible,actionDone,n); //TODO: !!! pass action done to getNextAction !!!! the action really done.. to do the update...
        assert(actions.size()>0);
        DEBUG_CLIENT(cout << "> Combi - for visib [used], actions[0]="<<ACTION_COUT(actions[0])<<", size="<<actions.size()<<endl;);
        vector<int> actNotVisib = _autoPlayerForNotVisib->getNextMultipleActions(seekerPos,hiderPos,opponentVisible,actionDone,n);
        assert(actNotVisib.size()>0);
        DEBUG_CLIENT(cout << "> Combi - for not visib [not used], actions[0]="<<ACTION_COUT(actNotVisib[0])<<", size="<<actNotVisib.size()<<endl;);
    } else {
        DEBUG_CLIENT(cout << "> Combi - Opp not visib <"<<endl;);
        actions = _autoPlayerForNotVisib->getNextMultipleActions(seekerPos,hiderPos,opponentVisible,actionDone,n);
        assert(actions.size()>0);
        DEBUG_CLIENT(cout << "> Combi - for not visib [used], actions[0]="<<ACTION_COUT(actions[0])<<", size="<<actions.size()<<endl;);
    }


    //TODO:.... use real action to update..
    _action = actions[0];

    return actions;
}

Pos CombinedSeeker::getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    DEBUG_CLIENT(cout << "CombinedSeeker::getNextPos: "<<endl;);
    Pos nextPos;
    if (opponentVisible) {
        DEBUG_CLIENT(cout << "> Combi - Opp visib <"<<endl;);
        nextPos = _autoPlayerForVisib->getNextPos(seekerPos,hiderPos,opponentVisible,actions,actionDone,n);
        //assert(actions.size()>0);
        DEBUG_CLIENT(
            cout << "> Combi - for visib [used], next pos="<<nextPos.toString();
            if (actions.size()>0)
                cout <<", actions[0]="<<ACTION_COUT(actions[0])<<", size="<<actions.size()<<endl;
            else
                cout << ", no actions"<<endl;
        );

        vector<int> actNotVisib;
        Pos nextPosNotV = _autoPlayerForNotVisib->getNextPos(seekerPos,hiderPos,opponentVisible,actNotVisib,actionDone,n);
        //assert(actNotVisib.size()>0);
        DEBUG_CLIENT(
            cout << "> Combi - for not visib [not used], next pos="<<nextPosNotV.toString();
            if (actNotVisib.size()>0)
                cout << ", actions[0]="<<ACTION_COUT(actNotVisib[0])<<", size="<<actNotVisib.size()<<endl;
            else
                cout << ", no actions"<<endl;
        );
    } else {
        DEBUG_CLIENT(cout << "> Combi - Opp not visib <"<<endl;);
        nextPos = _autoPlayerForNotVisib->getNextPos(seekerPos,hiderPos,opponentVisible,actions,actionDone,n);
        assert(actions.size()>0);
        DEBUG_CLIENT(cout << "> Combi - for not visib [used], actions[0]="<<ACTION_COUT(actions[0])<<", size="<<actions.size()<<endl;);
        DEBUG_CLIENT(
            cout << "> Combi - for not visib [used], next pos="<<nextPos.toString();
            if (actions.size()>0)
                cout <<", actions[0]="<<ACTION_COUT(actions[0])<<", size="<<actions.size()<<endl;
            else
                cout << ", no actions"<<endl;
        );
    }

    return nextPos;
}

double CombinedSeeker::getBelief(int r, int c) {
    return _autoPlayerForNotVisib->getBelief(r,c);
}

bool CombinedSeeker::tracksBelief() const {
    return _autoPlayerForNotVisib->tracksBelief();
}

bool CombinedSeeker::isSeeker() const {
    return true;
}

std::string CombinedSeeker::getName()  const{
    stringstream ss;
    ss<<"CombinedSeeker:for-visib="<<_autoPlayerForVisib->getName()<<";for-not-visib="<<_autoPlayerForNotVisib->getName();
    return ss.str();
}

bool CombinedSeeker::canScoreObservations() {
    return _autoPlayerForNotVisib->canScoreObservations();
}

double CombinedSeeker::scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone) {
    return _autoPlayerForNotVisib->scoreObservation(seekerPos, hiderPos, actionDone);
}

Pos CombinedSeeker::getClosestSeekerObs(Pos seekerPos) {
    return _autoPlayerForNotVisib->getClosestSeekerObs(seekerPos);
}
