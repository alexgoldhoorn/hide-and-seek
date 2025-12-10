#include "Smart/follower.h"

#include <cmath>
#include <cassert>
#include <iostream>

using namespace std;

Follower::Follower(SeekerHSParams* params) : AutoPlayer(params) {
}

Follower::~Follower() {
}

bool Follower::initBeliefRun() {
    //AG150604: nothing
    return true;
}

double Follower::getNextDirection(bool &haltAction) {
    double ang = 0;

    if (playerInfo.hiderObsPosWNoise.isSet()){ //hiderPlayer->currentPos.isSet()) {
        ang = _map->getDirection(playerInfo.currentPos, playerInfo.hiderObsPosWNoise);//_hiderPlayer->currentPos);

        haltAction = false;
    } else {
        //hider not visible, so don't move
        haltAction = true;
    }

    return ang;
}


Pos Follower::getNextPosRun(int actionDone, int *newAction) {
            //(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    //AG150624: also here we assume there is only 1 player, with more seekers we could use all of its observations
    //(e.g. use TwoSeekerHBExplorer::chooseHiderObsFromMulti)

    Pos returnPos;

    //set the last position, which we want to follow
    if (playerInfo.hiderObsPosWNoise.isSet()) {
        _lastPos = playerInfo.hiderObsPosWNoise;
    }

    if (_lastPos.isSet()) {
        //if a position is set
        double d = _lastPos.distanceEuc(playerInfo.currentPos);

        //AG160407: adapted to 3 situations: 1) simulation; 2) real and close to person; 3) not close
        /*if ( (_params->onlySendStepGoals && d > 1) ||
             (!_params->onlySendStepGoals && d < 2*_params->winDist && playerInfo.hiderObsPosWNoise.isSet()) ) {

            //stop
            bool stopBeforePos = (_params->solverType==SeekerHSParams::SOLVER_FOLLOWER_LAST_POS ||
                                  (playerInfo.hiderObsPosWNoise.isSet() && !_params->onlySendStepGoals));
            returnPos = getNextPosAsStep(playerInfo.currentPos, _lastPos, 1, stopBeforePos);
            */
        if (_params->onlySendStepGoals && d > 1) {
            //
            bool stopBeforePos = (_params->solverType!=SeekerHSParams::SOLVER_FOLLOWER_LAST_POS_EXACT //AG160407: changed to LAST_POS_EXACT, was LAST_POS
                                && playerInfo.hiderObsPosWNoise.isSet()); // AND: hider is visible
            returnPos = getNextPosAsStep(playerInfo.currentPos, _lastPos, 1, stopBeforePos);
             assert(returnPos.isSet());
        } else if (!_params->onlySendStepGoals && playerInfo.hiderObsPosWNoise.isSet()) {
            //get direction
            double dir1 = _map->getDirection(_lastPos, playerInfo.currentPos);
            //got in direction of the hider, staying at a minimal distance
            returnPos = _map->tryMoveDir(dir1, _lastPos, _params->followPersonDist, false);

            //AG160513: there are cases where tryMoveDir returns no pos (when close to person..)
            if (!returnPos.isSet()) {
                cout << "Follower::getNextPosRun: WARNING: GMap::tryMoveDir did not return a postion, returning current pos."<<endl;
                returnPos = playerInfo.currentPos;
            }
            assert(returnPos.isSet());

        } else {
            returnPos = _lastPos;
            assert(returnPos.isSet());
        }
    } else {
        //otherwise don't move
        returnPos = playerInfo.currentPos;
        assert(returnPos.isSet());
    }


    if (newAction!=NULL)  {
        //TODO: this could be done with deduceAction
        *newAction = -1;
    }

    DEBUG_SHS(cout<<"Follower::getNextPosRun: lastPos="<<_lastPos.toString()<<"; returnPos="<<returnPos.toString()<<endl;);

    return returnPos;
}

bool Follower::isSeeker() const {
    return true;
}

std::string Follower::getName() const {
    string name;
    switch(_params->solverType) {
        case SeekerHSParams::SOLVER_FOLLOWER:
            name = "Follower";
            break;
        /*case SeekerHSParams::SOLVER_FOLLOWER_LAST_POS:
            name = "FollowerLastPos";
            break;*/
        case SeekerHSParams::SOLVER_FOLLOWER_LAST_POS_EXACT:
/*        case SeekerHSParams::SOLVER_COMBI_POMCP_FOLLOWER:
        case SeekerHSParams::SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF:*/
            name = "FollowerLastPosExact";
            break;
        case SeekerHSParams::SOLVER_FOLLOWER_SEES_ALL:
            name = "FollowerSeesAll";
            break;
        default:
            name = "FollowerLastPos";
            //name="UNKNOWN Follower";
            break;
    }

    return name;
}

bool Follower::useGetAction() {
    return false;
}
