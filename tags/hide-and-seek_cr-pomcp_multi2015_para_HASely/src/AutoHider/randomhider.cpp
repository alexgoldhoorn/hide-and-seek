#include "AutoHider/randomhider.h"

#include "Utils/generic.h"
#include "Base/hsglobaldata.h"

#include "exceptions.h"

#include <iostream>

using namespace std;

RandomHider::RandomHider(SeekerHSParams* params, size_t n) : /*AutoHider(params),*/ AutoWalker(params, n)
{
    initRandomizer();
    _map = NULL;
}

RandomHider::~RandomHider() {
}

/*int RandomHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    int a = -1;

    do {
        _hiderPlayer.playerInfo.currentPos = hiderPos;
        a = random(HSGlobalData::NUM_ACTIONS-1); // rand() % HSGlobalData::NUM_ACTIONS;
    } while (!_hiderPlayer.move(a));

    return a;
}*/

Pos RandomHider::getNextPosRun(int actionDone, int* newAction) {
    int a = -1;

    do {
        //random action
        a = random(HSGlobalData::NUM_ACTIONS-1);

        playerInfo.nextPos = _map->tryMove(a, playerInfo.currentPos);

    } while (!playerInfo.nextPos.isSet());

    //set action
    if (newAction!=NULL) *newAction = a;

    return playerInfo.nextPos;
}


/*bool RandomHider::initBelief(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    AutoHider::initBelief(gmap, seekerInitPos, hiderInitPos, opponentVisible);

    //cout<<"randh.initBelief: seeekrinit="<<seekerInitPos.toString()<<", hiderinit="<<hiderInitPos.toString()<<" oppvis="<<opponentVisible<<endl;
    //AG140527: NOT USED for autowalker, all done in Server!!!!
    if (_autoWalkerVec.size()>0) {
        //use random walkers

        //NOTE: assuming there are at least n+2 positions
        //NOTE2: we can allow more at same place, and if continuous just a min distance to other persons

        //init pos of each random walker
        for(size_t i=0; i<_autoWalkerVec.size(); i++) {
            //get a random pos that is not the hider nor seekerpos
            IDPos ranPos(i+1); //AG140513: set ID starting from 1,
            do {
                ranPos.set(_map->genRandomPos());
            } while (ranPos==seekerInitPos || ranPos==hiderInitPos);
            //set to init for this 'person'
            _autoWalkerVec[i] = ranPos;
        }
    }

    return true;
}*/

/*bool RandomHider::initBelief(GMap* gmap, PlayerInfo* hiderPlayer) {
    return AutoHider::initBelief(gmap, hiderPlayer);
}

bool RandomHider::initBeliefMulti(GMap* gmap, std::vector<PlayerInfo*> playerVec, int thisPlayerID, int hiderPlayerID) {
    return AutoHider::initBeliefMulti(gmap, playerVec, thisPlayerID, hiderPlayerID);
}*/


bool RandomHider::initBeliefRun() {
    //cout<<"randh.initBelief: seeekrinit="<<seekerInitPos.toString()<<", hiderinit="<<hiderInitPos.toString()<<" oppvis="<<opponentVisible<<endl;
    //AG140527: NOT USED for autowalker, all done in Server!!!!
    if (_autoWalkerVec.size()>0) {
        //use random walkers

        //NOTE: assuming there are at least n+2 positions
        //NOTE2: we can allow more at same place, and if continuous just a min distance to other persons

        //get pos froms seeker  and this
        Pos seekerPos;
        Pos hiderPos;
        if (_hiderPlayer!=NULL)
            hiderPos = _hiderPlayer->currentPos;
        if (_seekerPlayer1!=NULL)
            seekerPos = _seekerPlayer1->currentPos;

        //init pos of each random walker
        for(size_t i=0; i<_autoWalkerVec.size(); i++) {
            //get a random pos that is not the hider nor seekerpos
            IDPos ranPos(i+1); //AG140513: set ID starting from 1,
            do {
                ranPos.set(_map->genRandomPos());
            } while (ranPos==hiderPos || ranPos==seekerPos);
            //set to init for this 'person'
            _autoWalkerVec[i] = ranPos;
        }
    }

    return true;
}

std::string RandomHider::getName() const {
    return "RandomHider";
}

int RandomHider::getHiderType() const {
    return HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM;
}

/*
std::vector<IDPos> RandomHider::getAllNextPos(Pos seekerPos, Pos hiderPos) {
    //update all positions
    //NOTE: maybe we should create a new vector for the t+1 positions
    for(size_t i=0; i<_autoWalkerVec.size(); i++) {
        //get a random pos that is not the hider nor seekerpos
        Pos ranPos;
        bool cont = true;
        do {
            Pos curPos = _autoWalkerVec[i];

            //get the next action
            int a = getNextAction(curPos, hiderPos, false);
            //get pos
            ranPos = _map->tryMove(a, curPos);

            if (ranPos.isSet()) {
                //now check if no collisions
                cont = !checkMovement(i, ranPos, seekerPos, hiderPos);
            } else {
                //could not move
                cont = true;
            }
        } while (cont);
        //set to init for this 'person'
        _autoWalkerVec[i].set(ranPos);
    }

    return _autoWalkerVec;
}*/

std::vector<IDPos> RandomHider::getAllNextPos(Pos seekerPos, Pos hiderPos) {
    //update all positions
    //NOTE: maybe we should create a new vector for the t+1 positions
    for(size_t i=0; i<_autoWalkerVec.size(); i++) {
        //get a random pos that is not the hider nor seekerpos
        Pos ranPos;
        bool cont = true;
        do {
            Pos curPos = _autoWalkerVec[i];

            //get the next action
            int a = random(HSGlobalData::NUM_ACTIONS-1);

            //get pos
            ranPos = _map->tryMove(a, curPos);

            if (ranPos.isSet()) {
                //now check if no collisions
                cont = !checkMovement(i, ranPos, seekerPos, hiderPos);
            }
        } while (cont);
        //set to init for this 'person'
        _autoWalkerVec[i].set(ranPos);
    }

    return _autoWalkerVec;
}

/*
void RandomHider::setMap(GMap* map) {
    AutoHider::setMap(map);    
}

GMap* RandomHider::getMap() const {
    return AutoHider::getMap();
}

SeekerHSParams* RandomHider::getParams() const {
    return _params;
}
*/
