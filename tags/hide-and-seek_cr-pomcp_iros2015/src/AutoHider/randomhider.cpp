#include "AutoHider/randomhider.h"

#include "Utils/generic.h"
#include "hsglobaldata.h"

#include "exceptions.h"

#include <iostream>

using namespace std;

RandomHider::RandomHider(SeekerHSParams* params, size_t n) : AutoHider(params), AutoWalker(n)
{
    initRandomizer();
    _map = NULL;
}

RandomHider::~RandomHider() {
}

int RandomHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    int a = -1;

    do {
        _hiderPlayer.setCurPos(hiderPos);
        a = random(HSGlobalData::NUM_ACTIONS-1); // rand() % HSGlobalData::NUM_ACTIONS;
    } while (!_hiderPlayer.move(a));

    return a;
}

bool RandomHider::initBelief(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
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
}

void RandomHider::setMap(GMap* map) {
    AutoHider::setMap(map);
}

GMap* RandomHider::getMap() const {
    return AutoHider::getMap();
}

SeekerHSParams* RandomHider::getParams() const {
    return _params;
}
