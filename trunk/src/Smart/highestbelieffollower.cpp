#include "Smart/highestbelieffollower.h"

#include "exceptions.h"
#include "Utils/generic.h"

#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

HighestBeliefFollower::HighestBeliefFollower(SeekerHSParams *params, AutoPlayer *autoPlayerWBelief) :
    AutoPlayer(params), _autoPlayerWBelief(autoPlayerWBelief) {

    assert(autoPlayerWBelief!=NULL);

    _timerID = -1;
    _stepsToLastUpdate = 0;
}

HighestBeliefFollower::~HighestBeliefFollower() {
}

bool HighestBeliefFollower::initBelief(GMap *gmap, PlayerInfo *otherPlayer) {
    DEBUG_CLIENT(cout << "HighestBeliefFollower::initBelief[single]: "<<endl;);
    AutoPlayer::initBelief(gmap, otherPlayer);

    //AG150605: copy values
    _autoPlayerWBelief->playerInfo.copyValuesFrom(playerInfo, true);

    _stepsToLastUpdate = 0;
    bool ok = _autoPlayerWBelief->initBelief(gmap, otherPlayer);
    return ok;
}

bool HighestBeliefFollower::initBeliefMulti(GMap *gmap, std::vector<PlayerInfo *> playerVec, int thisPlayerID, int hiderPlayerID) {
    DEBUG_CLIENT(cout << "HighestBeliefFollower::initBelief[multi]: "<<endl;);
    AutoPlayer::initBeliefMulti(gmap, playerVec, thisPlayerID, hiderPlayerID);

    //AG150605: copy values
    _autoPlayerWBelief->playerInfo.copyValuesFrom(playerInfo, true);
    //cout <<"playerInfo:"<<playerInfo.toString()<<", copied: "<<_autoPlayerWBelief->playerInfo.toString()<<endl;

    _stepsToLastUpdate = 0;
    bool ok = _autoPlayerWBelief->initBeliefMulti(gmap ,playerVec, thisPlayerID, hiderPlayerID);
    return ok;
}

bool HighestBeliefFollower::initBeliefRun() { //GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    return true; //AG150604: not implemented, because already passed to 'children'
}

#ifdef OLD_CODE
bool HighestBeliefFollower::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    _map = gmap;
    DEBUG_CLIENT(cout << "HighestBeliefFollower::initBelief: "<<endl;);
    _stepsToLastUpdate = 0;
    bool initB = _autoPlayerWBelief->initBelief(gmap, seekerInitPos, hiderInitPos, opponentVisible);

    return initB;
}

double HighestBeliefFollower::getNextDirection(Pos seekerPos, Pos hiderPos, bool opponentVisible, bool &haltAction) {
    throw CException(_HERE_, "HighestBeliefFollower::getNextDirection: not implemented");
    return 0;
}
#endif

Pos HighestBeliefFollower::getMapPosFromZoomCel(int r, int c) {
    assert(r>=0 && c>=0);
    double halfZCell = _params->beliefMapZoomFactor/2;
    Pos pos(r*_params->beliefMapZoomFactor+halfZCell, c*_params->beliefMapZoomFactor+halfZCell);
    //AG140601: check if it is an obstacle, then choose another cell inside the 'supercell'
    if (!_map->isPosInMap(pos) || _map->isObstacle(pos)) {
        //find the first cell which is not an obstacle
        int width = (int)ceil(_params->beliefMapZoomFactor);
        int startRow = (int)floor(r*_params->beliefMapZoomFactor);
        int startCol = (int)floor(c*_params->beliefMapZoomFactor);
        //int width = (int)ceil(_params->beliefMapZoomFactor);
        //bool found = false;

        //AG160707: create a list of free locations inside the zoom cell; until now the first free cell is chosen, but
        //          a random location would be better in some situations
        vector<Pos> highbelFreePosVec;

        //now loop the cells inside the supercell to find a non-obstacle
        for(int r = startRow; r < startRow+width && r<_map->rowCount() /*&& !found*/; r++) {
            for(int c = startCol; c < startCol+width && c<_map->colCount() /*&& !found*/; c++) {
                if (!_map->isObstacle(r,c)) {
                    //found = true; //AG160707: was here to use first free
                    pos.set(r+0.5,c+0.5);
                    //AG160707: create list of highest belief free poses
                    highbelFreePosVec.push_back(pos);
                }
            }
        }

        if (highbelFreePosVec.empty() /*!found*/) cout << "ERROR: no highest belief point found without obstacle start: r"<<startRow<<"c"<<startCol<<endl;
        //assert(found); // if not found than the 'zooming' was incorrect
        assert(!highbelFreePosVec.empty());

        //AG160707: get random free cell
        if (highbelFreePosVec.size()==1)
            pos = highbelFreePosVec[0];
        else
            pos = highbelFreePosVec[hsutils::random(highbelFreePosVec.size()-1)];

        assert(!_map->isObstacle(pos));
    }

    if (!_params->useContinuousPos)
        pos.convertValuesToInt();

    return pos;
}

vector<BPos> HighestBeliefFollower::findHighestBelief(const Pos &seekerPos, unsigned int n, double maxSearchRange){
    assert(n>0);
    //AG150204: this is a requirement
    if (!_autoPlayerWBelief->tracksBelief())
        throw CException(_HERE_, "HighestBeliefFollower requires an AutoPlayer that keeps track of a belief.");

    //list of max belief points (init with 'not set')
    vector<BPos> maxBPosVec(n);

    // check if a max belief found
    unsigned int numMaxBeliefFound = 0;

    //get zoomed map
    unsigned int brows,bcols;
    double** belief = getBeliefZoomMatrix(brows, bcols);
    DEBUG_CLIENT(cout << "Belief map size: "<<brows<<"x"<<bcols<<endl;);
    assert(belief!=NULL);

    //find max pos
    for(unsigned int r=0; r<brows; ++r) {
        for(unsigned int c=0; c<bcols; ++c) {
            //get belief
            double b = belief[r][c];
            DEBUG_CLIENT(cout << (b>0?"#":".")<<flush;);

            bool doBeliefCheck =  (b>0);
            if (doBeliefCheck && maxSearchRange>0) {
                //distance check to prevent searching the highest belief too far
                Pos p = getMapPosFromZoomCel(r,c);
                doBeliefCheck = (_map->distance(seekerPos,p) <= maxSearchRange);
            }
            if (doBeliefCheck) {
                //AG150123: search highest n values
                int mi=n-1;
                //find index mi which has a belief just higher than b
                for(;mi>=0 && b>maxBPosVec[mi].b; --mi) {}

                if (mi<(int)(n-1)) {
                    //to be placed on index mi+1
                    //aMaxBeliefFound = true;
                    ++numMaxBeliefFound;

                    //now shift all bellow one down (from the end until m+1)
                    for(int j=n-1; j>=mi+2; --j) {
                        maxBPosVec[j] = maxBPosVec[j-1];
                    }
                    //now set new value
                    maxBPosVec[mi+1].b = b;
                    maxBPosVec[mi+1].set(r,c);
                }

            }
        }
        DEBUG_CLIENT(cout<<endl;);
    } //for

    //delete belief matrix
    hsutils::FreeDynamicArray<double>(belief,brows);

    //now choose highest
    if (numMaxBeliefFound==0) {
        cout << "WARNING: HighestBeliefFollower::findHighestBelief: maximum belief is 0"<<endl;
        if (maxSearchRange==0) {
            throw CException(_HERE_,"if there is no search limit, there should be a highest belief point since the sum is 1.0");
        } else {
            //AG160512: if no highest belief is found in the area we should search it outside the region
            cout <<" Researching without maximum distance."<<endl;
            return findHighestBelief(seekerPos, n, 0);
        }
    }

    if (numMaxBeliefFound<n) {
        maxBPosVec.resize(numMaxBeliefFound);
    }

    //convert to original coordinates
    for(BPos& hbPos: maxBPosVec) {
        if (hbPos.isSet()) { //AG150206: check if really is set
            hbPos = getMapPosFromZoomCel(hbPos.row(),hbPos.col());
            //AG140601: check if it is an obstacle, then choose another cell inside the 'supercell'
            if (_map->isObstacle(hbPos)) {
                //find the first cell which is not an obstacle
                int startRow = (int)floor(hbPos.row()*_params->beliefMapZoomFactor);
                int startCol = (int)floor(hbPos.col()*_params->beliefMapZoomFactor);
                int width = (int)ceil(_params->beliefMapZoomFactor);
                bool found = false;

                //now loop the cells inside the supercell to find a non-obstacle
                for(int r = startRow; r < startRow+width && r<_map->rowCount() && !found; r++) {
                    for(int c = startCol; c < startCol+width && c<_map->colCount() &&!found; c++) {
                        if (!_map->isObstacle(r,c)) {
                            found = true;
                            hbPos.set(r+0.5,c+0.5);
                        }
                    }
                }

                if (!found) cout << "ERROR: no highest belief point found without obstacle start: r"<<startRow<<"c"<<startCol<<endl;
                assert(found); // if not found than the 'zooming' was incorrect
            }
        }

        if (!_params->useContinuousPos)
            hbPos.convertValuesToInt();
    }

    return maxBPosVec;
}

Pos HighestBeliefFollower::getNextPosRun(int actionDone, int *newAction) {
    DEBUG_SHS(cout<<"HighestBeliefFollower::getNextPosRun: running autoplayer with belief getNextPos"<<endl;);

    //Pos returnPos;
    //AG150624: here we could use more seeker observation and choose best (e.g. use TwoSeekerHBExplorer::chooseHiderObsFromMulti)
    Pos& hiderPos = playerInfo.hiderObsPosWNoise;
    Pos& seekerPos = playerInfo.currentPos;

    //AG150605: copy values
    _autoPlayerWBelief->playerInfo.copyValuesFrom(playerInfo, true);

    //first update belief, but don't use the resulting action
    Pos aPlayerPos = _autoPlayerWBelief->getNextPos(actionDone, newAction);
    //seeker reward
    playerInfo.seekerReward = _autoPlayerWBelief->playerInfo.seekerReward;
    //store resulting action

    //check if we want to update the next point: when either a timer passed, the hider is visible, or close to the previoulsy calculated position
    if ( (_timerID<0 && _params->highBeliefFollowerUpdateGoalTime_ms>0) ||      //no timer yet
            (_params->highBeliefFollowerUpdateGoalNumSteps>0 && _stepsToLastUpdate<=0)
            ||             //OR: a number of steps to update is set and it is passed
            (       _params->highBeliefFollowerUpdateGoalTime_ms>0
                    && _timer.getTime_ms(_timerID)>=_params->highBeliefFollowerUpdateGoalTime_ms
            )              //OR: there is update time set, and the time is passed
            || hiderPos.isSet() || seekerPos.distanceEuc(_lastGoalPos)<=_params->winDist )
        //OR: the hider is visible, OR the last goal is close
    {
        DEBUG_SHS(cout<<"HighestBeliefFollower::getNextPosRun: timer passed, researching belief for new goal: "<<flush;);

        //AG150123: find list of highest beliefs, but only 1
        vector<BPos> hbPosVec = findHighestBelief(seekerPos,1,_params->highBeliefFollowerHighestDist);
        assert(hbPosVec.size()==1);
        _lastGoalPos = hbPosVec[0];

        DEBUG_SHS(cout<<_lastGoalPos.toString()<<endl;);

        //check timer
        if (_params->highBeliefFollowerUpdateGoalTime_ms>0) {
            if (_timerID<0) {
                _timerID = _timer.startTimer();
            } else {
                _timer.restartTimer(_timerID);
            }
        }

        playerInfo.nextPos = _lastGoalPos;
        _stepsToLastUpdate = _params->highBeliefFollowerUpdateGoalNumSteps;

    } else if (_lastGoalPos.isSet()) {
        //return last position
        playerInfo.nextPos = _lastGoalPos;
    } else {
        //SHOULD not occur
        cout <<"WARNING: HighestBeliefFollower::getNextPosRun: time passed and no last pos found"<<endl;
        playerInfo.nextPos = aPlayerPos; // seekerPos;
    }

    DEBUG_SHS(cout<<"HighestBeliefFollower::getNextPosRun: return "<<playerInfo.nextPos.toString(););
    playerInfo.chosenGoalPos = playerInfo.nextPos;

    if (hiderPos.isSet() || _params->onlySendStepGoals) {
        //get the next step towards goal, using 'navigation'
        playerInfo.nextPos = getNextPosAsStep(seekerPos, playerInfo.nextPos, 1, hiderPos.isSet());

        /*if (_params->onlySendStepGoals && _params->useContinuousPos) {
            //we are simulating n steps and continuous space, should be at correct distance, not sqrt(2)
            _map->tryMoveDirStep()

        }*/ //-> to be done in getNetxtPosAsStep

        DEBUG_SHS(cout<<", in steps, next step: "<<playerInfo.nextPos.toString(););
    }
    DEBUG_SHS(cout<<endl;);

    _stepsToLastUpdate--;

    return playerInfo.nextPos;
}

bool HighestBeliefFollower::isSeeker() const {
    return true;
}

std::string HighestBeliefFollower::getName() const {
    stringstream ss;
    ss<<"HighestBeliefFollower-of:"<<_autoPlayerWBelief->getName();
    return ss.str();
}

bool HighestBeliefFollower::useGetAction() const {
    return false;
}

double HighestBeliefFollower::getBelief(int r, int c) {
    return _autoPlayerWBelief->getBelief(r,c);
}

bool HighestBeliefFollower::tracksBelief() const {
    return _autoPlayerWBelief->tracksBelief();
}

bool HighestBeliefFollower::canScoreObservations() {
    return _autoPlayerWBelief->canScoreObservations();
}

double HighestBeliefFollower::scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone) {
    return _autoPlayerWBelief->scoreObservation(seekerPos, hiderPos, actionDone);
}

Pos HighestBeliefFollower::getClosestSeekerObs(Pos seekerPos) {
    return _autoPlayerWBelief->getClosestSeekerObs(seekerPos);
}

void HighestBeliefFollower::setMap(GMap *map) {
    AutoPlayer::setMap(map);
    _autoPlayerWBelief->setMap(map);
}
