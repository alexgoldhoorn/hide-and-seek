#include "Filter/kalmanfilterplayer.h"

#include <cmath>
#include <cassert>
#include <iostream>
#include <eigen3/Eigen/Core>

#include "exceptions.h"
#include "Base/hsconfig.h"

using namespace std;

KalmanFilterPlayer::KalmanFilterPlayer(SeekerHSParams* params) : AutoPlayer(params)
{
    DEBUG_FILTER(cout<<"KalmanFilter"<<endl;);
}

KalmanFilterPlayer::~KalmanFilterPlayer() {
}

bool KalmanFilterPlayer::initBeliefRun() {
    if (!playerInfo.hiderObsPosWNoise.isSet()) {
        cout << "KalmanFilterPlayer::initBeliefRun: ERROR: the hider position should be set for initialization of initBeliefRun()"<<endl;

        if (_params->solverType==SeekerHSParams::SOLVER_FILTER_KALMAN_REQ_VISIB)
            throw CException(_HERE_,"KalmanFilterPlayer::initBeliefRun: observation required (since type is SOLVER_FILTER_KALMAN_REQ_VISIB)");

        return false;
    }

    //movement can be the same
    // TODO: ? should F be changed, since the speed is actually 'learnt', otherwise set step size in F
    /*Eigen::Matrix4d F;
    F <<        1, 0, 1, 0,
                0, 1, 0, 1,
                0, 0, 1, 0,
                0, 0, 0, 1;*/

    DEBUG_FILTER(cout<<"Initializing KalmanFilter:"<<endl);

    //set motion noise based on configuration
    Eigen::Matrix4d Q = Eigen::Matrix4d::Identity() * _params->contNextHiderStateStdDev;
    _kf.setMotionNoise(Q);
    DEBUG_FILTER(cout << "Motion noise (Q): "<<endl<<Q<<endl;);

    //set obs noise
    Eigen::Matrix2d R = Eigen::Matrix2d::Constant(_params->contHiderObsStdDev);
    _kf.setObsNoise(R);
    DEBUG_FILTER(cout << "Obs noise (R): "<<endl<<R<<endl;);

    //Initialize the state with the position of the person, and (0,0) speed
    Eigen::Vector4d x;
    x << playerInfo.hiderObsPosWNoise.rowDouble(), playerInfo.hiderObsPosWNoise.colDouble(), 0, 0;
    DEBUG_FILTER(cout << "Init state (x): "<<endl<<x<<endl;);

    Eigen::Matrix4d P = Eigen::Matrix4d::Identity()* _params->contNextHiderStateStdDev; //assume same noise for init observation
    DEBUG_FILTER(cout << "State cov. (P): "<<endl<<P<<endl;);

    _kf.initState(x,P);

    return true;
}

/*double KalmanFilterPlayer::getNextDirection(bool &haltAction) {
    double ang = 0;

    if (playerInfo.hiderObsPosWNoise.isSet()){
        ang = _map->getDirection(playerInfo.currentPos, playerInfo.hiderObsPosWNoise);

        haltAction = false;
    } else {
        //hider not visible, so don't move
        haltAction = true;
    }

    return ang;
}*/


Pos KalmanFilterPlayer::getNextPosRun(int actionDone, int *newAction) {
    if (!playerInfo.hiderObsPosWNoise.isSet()) {
        DEBUG_FILTER(cout << "KalmanFilterPlayer::getNextPosRun: WARNING: the hider position is not set, only doing prediction."<<endl;);

        if (_params->solverType==SeekerHSParams::SOLVER_FILTER_KALMAN_REQ_VISIB)
            throw CException(_HERE_,"KalmanFilterPlayer::getNextPosRun: observation required (since type is SOLVER_FILTER_KALMAN_REQ_VISIB)");
    }

    //predict
    DEBUG_FILTER(cout<< "KF predict: "<<flush;);
    _kf.predict();
    DEBUG_FILTER(cout<< "ok"<<endl;);

    if (playerInfo.hiderObsPosWNoise.isSet()) {
        //create observation
        Eigen::Vector2d obs;
        obs << playerInfo.hiderObsPosWNoise.rowDouble(), playerInfo.hiderObsPosWNoise.colDouble();

        DEBUG_FILTER(cout<< "KF update: "<<flush;);
        //update
        _kf.update(obs);
        DEBUG_FILTER(cout<< "ok"<<endl;);
    }

    //return pos is center of Kalman filter's state
    const Eigen::Vector4d& x = _kf.getState();
    Pos returnPos(x(0), x(1));

    DEBUG_FILTER(cout << "State (x): "<<endl<<x<<endl;);
    DEBUG_FILTER(cout << "State cov. (P): "<<endl<<_kf.getStateCov()<<endl;);

    //check if return pos is a legal position
    //TODO: is there a nicer way to solve the issue of having legal positions?
    if (!_map->isPosInMap(returnPos) || _map->isObstacle(returnPos)) {
        returnPos = getClosestSeekerObs(returnPos);
    }
    DEBUG_FILTER(cout << "return pos: "<<endl<<returnPos.toString()<<endl;);

    //TODO!! THIS PART IS REPEATED; ALSO IN FOLLOWER

    //if a position is set
    double d = returnPos.distanceEuc(playerInfo.currentPos); //seekerPos);

    if ( (_params->onlySendStepGoals && d > 1) ||
         (!_params->onlySendStepGoals && d < 2*_params->winDist && playerInfo.hiderObsPosWNoise.isSet()) )  {

        //stop
        bool stopBeforePos = (playerInfo.hiderObsPosWNoise.isSet() && !_params->onlySendStepGoals);
        returnPos = getNextPosAsStep(playerInfo.currentPos, returnPos, 1, stopBeforePos);
    }

    return returnPos;
}

bool KalmanFilterPlayer::isSeeker() const {
    return true;
}

std::string KalmanFilterPlayer::getName() const {
    return "KalmanFilter";
}

bool KalmanFilterPlayer::useGetAction() {
    return false;
}
