#include "AutoHider/randomwalker.h"

#include <iostream>
#include <cassert>

#include "Base/hsglobaldata.h"

#include "exceptions.h"

using namespace std;

RandomWalker::RandomWalker(SeekerHSParams* params, size_t n) : /*AutoHider(params),*/ AutoWalker(params, n) {
    _autoWalkerGoalVec.resize(n);
}

RandomWalker::~RandomWalker() {
}

/*bool RandomWalker::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    bool ok = AutoHider::initBelief(gmap,seekerInitPos,hiderInitPos,opponentVisible);

    //set next goal
    _nextGoal = _map->genRandomPos();
    DEBUG_AUTOHIDER(cout<<"RandomWalker, next goal: "<<_nextGoal.toString()<<endl;);

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

            //next goal
            _autoWalkerGoalVec[i] = _map->genRandomPos();
        }
    }

    return ok;
}*/

/*bool RandomWalker::initBelief(GMap* gmap, PlayerInfo* hiderPlayer) {
    return AutoHider::initBelief(gmap, hiderPlayer);
}

bool RandomWalker::initBeliefMulti(GMap* gmap, std::vector<PlayerInfo*> playerVec, int thisPlayerID, int hiderPlayerID) {
    return AutoHider::initBeliefMulti(gmap, playerVec, thisPlayerID, hiderPlayerID);
}*/

bool RandomWalker::initBeliefRun() {
    //set next goal
    _nextGoal = _map->genRandomPos();
    DEBUG_AUTOHIDER(cout<<"RandomWalker, next goal: "<<_nextGoal.toString()<<endl;);

    if (_autoWalkerVec.size()>0) {
        //use random walkers

        //NOTE: assuming there are at least n+2 positions
        //NOTE2: we can allow more at same place, and if continuous just a min distance to other persons
        assert(_hiderPlayer!=NULL);
        assert(_seekerPlayer1!=NULL);
        Pos hPos = _hiderPlayer->currentPos;
        Pos sPos = _seekerPlayer1->currentPos;


        //init pos of each random walker
        for(size_t i=0; i<_autoWalkerVec.size(); i++) {
            //get a random pos that is not the hider nor seekerpos
            IDPos ranPos(i+1); //AG140513: set ID starting from 1,
            do {
                ranPos.set(_map->genRandomPos());
            //} while (ranPos==seekerInitPos || ranPos==hiderInitPos);
            } while (ranPos==sPos || ranPos==hPos);
            //set to init for this 'person'
            _autoWalkerVec[i] = ranPos;

            //next goal
            _autoWalkerGoalVec[i] = _map->genRandomPos();
        }
    }

    return true;
}


/*int RandomWalker::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    throw CException(_HERE_, "RandomWalker::getNextAction: not implemented, use getNextPos");
}*/

/*Pos RandomWalker::getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    //cout<<"RandomWalker::getNextPosRun: current pos ="<<hiderPos.toString()<<endl;
    if (hiderPos == _nextGoal || (_params->useContinuousPos && hiderPos.distanceEuc(_nextGoal)<=1.0 ) ) {
        _nextGoal = _map->genRandomPos();
        DEBUG_AUTOHIDER(cout<<"GOAL REACHED, new goal: "<<_nextGoal.toString()<<endl;);
    }

    //next pos
    Pos nPos = getNextPosAsStep(hiderPos, _nextGoal, 1, false);

    DEBUG_AUTOHIDER(cout<<"RandomWalker next step to goal: "<<nPos.toString()<<endl;);

    return nPos;
}*/

Pos RandomWalker::getNextPosRun(int actionDone, int *newAction) { //Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    if (playerInfo.currentPos == _nextGoal || (_params->useContinuousPos && playerInfo.currentPos.distanceEuc(_nextGoal)<=1.0 /*_params->winDist*/ ) ) {
        _nextGoal = _map->genRandomPos();
        DEBUG_AUTOHIDER(cout<<"GOAL REACHED, new goal: "<<_nextGoal.toString()<<endl;);
    }

    //next pos
    playerInfo.nextPos = getNextPosAsStep(playerInfo.currentPos, _nextGoal, 1, false);

    DEBUG_AUTOHIDER(cout<<"RandomWalker next step to goal: "<<playerInfo.nextPos.toString()<<endl;);

    return playerInfo.nextPos;
}


int RandomWalker::getHiderType() const {
    return HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM_WALKER;
}

std::string RandomWalker::getName() const {
    return "RandomWalker";
}


std::vector<IDPos> RandomWalker::getAllNextPos(Pos seekerPos, Pos hiderPos) {
    //update all positions
    //NOTE: maybe we should create a new vector for the t+1 positions

    //cout << "Random Auto Walkers:"<<endl;
    for(size_t i=0; i<_autoWalkerVec.size(); i++) {
        //get a random pos that is not the hider nor seekerpos
        IDPos pos = _autoWalkerVec[i];
        Pos goal = _autoWalkerGoalVec[i];

        //check if goal reached
        if (pos == goal || (_params->useContinuousPos && pos.distanceEuc(goal)<=1.0)) {
            goal = _autoWalkerGoalVec[i] = _map->genRandomPos();
        }

        //next pos
        _autoWalkerVec[i].set(getNextPosAsStep(pos, goal, 1, false));
    }

    return _autoWalkerVec;
}

/*void RandomWalker::setMap(GMap* map) {
    AutoHider::setMap(map);
}

GMap* RandomWalker::getMap() const {
    return AutoHider::getMap();
}

SeekerHSParams* RandomWalker::getParams() const {
    return _params;
}*/
