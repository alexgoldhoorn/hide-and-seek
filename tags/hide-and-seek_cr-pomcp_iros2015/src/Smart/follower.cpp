
#include "Smart/follower.h"

#include <cmath>


#include <iostream>
using namespace std;

Follower::Follower(SeekerHSParams* params) : AutoPlayer(params) {
}

Follower::~Follower() {
}

bool Follower::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    _map = gmap;

    return true;
}

double Follower::getNextDirection(Pos seekerPos, Pos hiderPos, bool opponentVisible, bool &haltAction) {
    double ang = 0;

    if (opponentVisible) {
        ang = _map->getDirection(seekerPos,hiderPos);

        haltAction = false;
    } else {
        //hider not visible, so don't move
        haltAction = true;
    }

    return ang;
}


Pos Follower::getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    Pos returnPos;

    //set the last position, which we want to follow
    if (opponentVisible && hiderPos.isSet()) {
        _lastPos = hiderPos;
    }

    if (_lastPos.isSet()) {
        //if a position is set
        double d = _lastPos.distanceEuc(seekerPos);

        if ( (_params->onlySendStepGoals && d > 1) ||
             (!_params->onlySendStepGoals && d < 2*_params->winDist && hiderPos.isSet()) )  {

            //stop
            bool stopBeforePos = (_params->solverType==SeekerHSParams::SOLVER_FOLLOWER_LAST_POS || (hiderPos.isSet() && !_params->onlySendStepGoals));
            returnPos = getNextPosAsStep(seekerPos, _lastPos, n, stopBeforePos);
                //_map->tryMoveDirStep(dir, seekerPos, dist, _params->seekerStepDistance, _params->doVisibCheckBeforeMove);
        } else {
            returnPos = _lastPos;
        }
    } else {
        //otherwise don't move

        returnPos = seekerPos;
    }

    assert(returnPos.isSet());

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
        case SeekerHSParams::SOLVER_FOLLOWER_LAST_POS:
            name = "FollowerLastPos";
            break;
        case SeekerHSParams::SOLVER_FOLLOWER_LAST_POS_EXACT:
        case SeekerHSParams::SOLVER_COMBI_POMCP_FOLLOWER:
        case SeekerHSParams::SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF:
            name = "FollowerLastPosExact";
            break;
        default:
            name="UNKNOWN Follower";
            break;
    }

    return name;
}

bool Follower::useGetAction() {
    return false;
}
