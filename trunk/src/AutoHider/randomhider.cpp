#include "AutoHider/randomhider.h"

#include "Utils/generic.h"
#include "Base/hsglobaldata.h"

#include "exceptions.h"

#include <iostream>

using namespace std;

RandomHider::RandomHider(SeekerHSParams* params, size_t n) : AutoWalker(params, n)
{
    hsutils::initRandomizer();
    _map = NULL;
}

RandomHider::~RandomHider() {
}

Pos RandomHider::getNextPosRun(int actionDone, int* newAction) {
    int a = -1;

    do {
        //random action
        a = hsutils::random(HSGlobalData::NUM_ACTIONS-1);

        playerInfo.nextPos = _map->tryMove(a, playerInfo.currentPos);

    } while (!playerInfo.nextPos.isSet());

    //set action
    if (newAction!=NULL) *newAction = a;

    return playerInfo.nextPos;
}

bool RandomHider::initBeliefRun() {
    //AG140527: NOT USED for autowalker, all done in Server!!!!

    //AG160129: is start pos set, use it
    if (_startPos.isSet()) {
        //set init belief position
        AutoWalker::initBeliefRun();
    } else if (_autoWalkerVec.size()>0) {
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
            int a = hsutils::random(HSGlobalData::NUM_ACTIONS-1);

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
