
#include <sstream>
#include <iostream>
#include <cassert>

#include "Smart/combinedseeker.h"
#include "Base/hsglobaldata.h"


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

bool CombinedSeeker::initBelief(GMap *gmap, PlayerInfo *otherPlayer) {
    DEBUG_CLIENT(cout << "CombinedSeeker::initBelief[single]: "<<endl;);
    AutoPlayer::initBelief(gmap, otherPlayer);

    //AG150604: copy player info and meta data
    copyPlayerInfoValues(true);

    DEBUG_CLIENT(cout << "> Combi - Opp visib - init <"<<endl;);
    bool initOkV = _autoPlayerForVisib->initBelief(gmap, otherPlayer);
    DEBUG_CLIENT(cout << "> Combi - Opp not visib - init <"<<endl;);
    bool initOkNV = _autoPlayerForNotVisib->initBelief(gmap, otherPlayer);

    return initOkNV && initOkV;
}

bool CombinedSeeker::initBeliefMulti(GMap *gmap, std::vector<PlayerInfo *> playerVec, int thisPlayerID, int hiderPlayerID) {
    DEBUG_CLIENT(cout << "CombinedSeeker::initBelief[multi]: "<<endl;);
    AutoPlayer::initBeliefMulti(gmap, playerVec, thisPlayerID, hiderPlayerID);

    //AG150604: copy player info and meta data
    copyPlayerInfoValues(true);

    DEBUG_CLIENT(cout << "> Combi - Opp visib - init <"<<endl;);
    bool initOkV = _autoPlayerForVisib->initBeliefMulti(gmap, playerVec, thisPlayerID, hiderPlayerID);
    DEBUG_CLIENT(cout << "> Combi - Opp not visib - init <"<<endl;);
    bool initOkNV = _autoPlayerForNotVisib->initBeliefMulti(gmap, playerVec, thisPlayerID, hiderPlayerID);

    return initOkNV && initOkV;
}

bool CombinedSeeker::initBeliefRun() {
    return true; //AG150604: not implemented, because already passed to 'children'
}

Pos CombinedSeeker::getNextPosRun(int actionDone, int *newAction) {
    //(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    DEBUG_CLIENT(cout << "CombinedSeeker::getNextPos: "<<endl;);

    //AG150604: copy player info and meta data
    copyPlayerInfoValues(false);

    int newAct = -1;

    //AG150624: here we assume there is only 1 seeker, otherwise we should decide which observation to use
    //(e.g. use TwoSeekerHBExplorer::chooseHiderObsFromMulti)
    bool opponentVisible = playerInfo.hiderObsPosWNoise.isSet();

    if (opponentVisible) {
        DEBUG_CLIENT(cout << "> Combi - Opp visib <"<<endl;);
        playerInfo.nextPos = _autoPlayerForVisib->getNextPos(actionDone, &newAct); //(seekerPos,hiderPos,opponentVisible,actions,actionDone,n);
        //assert(actions.size()>0);
        assert(newAct>=0 && HSGlobalData::NUM_ACTIONS); //AG150707:
        DEBUG_CLIENT(
            cout << "> Combi - for visib [used], next pos="<<playerInfo.nextPos.toString()<<", action="<<ACTION_COUT(newAct)<< endl;
            /*if (actions.size()>0)
                cout <<", actions[0]="<<ACTION_COUT(actions[0])<<", size="<<actions.size()<<endl;
            else
                cout << ", no actions"<<endl;*/
        );

        //vector<int> actNotVisib;
        int newNotVAct = -1;
        Pos nextPosNotV = _autoPlayerForNotVisib->getNextPos(actionDone, &newNotVAct); //(seekerPos,hiderPos,opponentVisible,actNotVisib,actionDone,n);
        //assert(actNotVisib.size()>0);
        DEBUG_CLIENT(
            cout << "> Combi - for not visib [not used], next pos="<<nextPosNotV.toString()<<", action="<<ACTION_COUT(newNotVAct)<< endl;
            /*if (actNotVisib.size()>0)
                cout << ", actions[0]="<<ACTION_COUT(actNotVisib[0])<<", size="<<actNotVisib.size()<<endl;
            else
                cout << ", no actions"<<endl;*/
        );
    } else {
        DEBUG_CLIENT(cout << "> Combi - Opp not visib <"<<endl;);
        playerInfo.nextPos = _autoPlayerForNotVisib->getNextPos(actionDone, &newAct); //(seekerPos,hiderPos,opponentVisible,actions,actionDone,n);
        //assert(actions.size()>0);opponentVisible
        //DEBUG_CLIENT(cout << "> Combi - for not visib [used], actions[0]="<<ACTION_COUT(actions[0])<<", size="<<actions.size()<<endl;);
        assert(newAct>=0 && HSGlobalData::NUM_ACTIONS); //AG150707:
        DEBUG_CLIENT(
            cout << "> Combi - for visib [used], next pos="<<playerInfo.nextPos.toString()<<", action="<<ACTION_COUT(newAct)<< endl;
            /*cout << "> Combi - for not visib [used], next pos="<<nextPos.toString();
            if (actions.size()>0)
                cout <<", actions[0]="<<ACTION_COUT(actions[0])<<", size="<<actions.size()<<endl;
            else
                cout << ", no actions"<<endl;*/
        );
    }

    if (newAction!=NULL)
        *newAction = newAct;

    return playerInfo.nextPos;
}

#ifdef OLD_CODE
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
#endif

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

void CombinedSeeker::copyPlayerInfoValues(bool includeMetaInfo) {
    _autoPlayerForNotVisib->playerInfo.copyValuesFrom(playerInfo, includeMetaInfo);
    _autoPlayerForVisib->playerInfo.copyValuesFrom(playerInfo, includeMetaInfo);
}

void CombinedSeeker::setMap(GMap *map) {
    AutoPlayer::setMap(map);
    _autoPlayerForNotVisib->setMap(map);
    _autoPlayerForVisib->setMap(map);
}
