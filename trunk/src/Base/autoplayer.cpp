#include "Base/autoplayer.h"

#include "Base/hsglobaldata.h"
#include "Utils/generic.h"
#include <cmath>
#include "exceptions.h"

#ifdef USE_QT
#include "AutoHider/fromlisthider.h"
#endif

#include <sstream>
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace std;
using namespace hsutils;


AutoPlayer::AutoPlayer(SeekerHSParams* params) :
    _map(NULL), _params(params), _numIterationsSkipped(0), _numIterations(0),
    _hiderPlayer(NULL), _seekerPlayer1(NULL) {
}

AutoPlayer::AutoPlayer(SeekerHSParams* params, GMap* map) :
    _map(map), _params(params), _numIterationsSkipped(0), _numIterations(0),
    _hiderPlayer(NULL), _seekerPlayer1(NULL) {
}

AutoPlayer::~AutoPlayer() {
}

Pos AutoPlayer::getInitPos() {
    assert(_map!=NULL);
    assert(_map->isMapFixed());

    Pos initPos;

    if (_initPos.isSet()) {
        //init pos
        initPos = _initPos;

    } else if (!_params->seekerPosFile.empty()) {
#ifdef USE_QT
        //AG160614: check if using a file name with '?' which should be replaced by seekerID
        QString seekerPosFileQStr = QString::fromStdString(_params->seekerPosFile);
        seekerPosFileQStr.replace("?", QString::number(playerInfo.seekerID));

        //read position file to get start pos
        FromListHider fromListHider(_params, seekerPosFileQStr.toStdString()); //_params->seekerPosFile);
        fromListHider.setMap(_map);
        initPos = fromListHider.getInitPos();
#else
        cout << "AutoPlayer::getInitPos: ERROR: using a FromListHider while QT is not used!"<<endl;
#endif
    } else if (_params->randomPosDistToBase>0) {
        //random pos based on distance from base
        Pos base(_map->getBase());

        assert(base.isSet());

        //AG140106: do a check if it is realistic
        if (base.row()-_params->randomPosDistToBase<0 && base.col()-_params->randomPosDistToBase<0 &&
                base.row()+_params->randomPosDistToBase>=_map->rowCount() && base.col()+_params->randomPosDistToBase>=_map->colCount()) {
            cout << "WARNING: probably the random distance "<<_params->randomPosDistToBase<<" to the base "<<base.toString()
                 << " cannot be reached on the "<<_map->rowCount()<<"x"<<_map->colCount()<<" map."<<endl;
        }

        DEBUG_CLIENT(cout<<"Calculating start position at a distance of "<<_params->randomPosDistToBase<<" to the base."<<endl;);

        //AG130105: try to find a random place
        //first create list of possible positions, use Pos as vector
        vector<Pos> posInitVec;
        if (base.row()-_params->randomPosDistToBase>=0) {
            Pos p(base.row()-_params->randomPosDistToBase,0);
            posInitVec.push_back(p);
        }
        if (base.col()-_params->randomPosDistToBase>=0) {
            Pos p(0,base.col()-_params->randomPosDistToBase);
            posInitVec.push_back(p);
        }
        if (base.row()+_params->randomPosDistToBase<_map->rowCount()) {
            Pos p(base.row()+_params->randomPosDistToBase,0);
            posInitVec.push_back(p);
        }
        if (base.col()+_params->randomPosDistToBase<_map->colCount()) {
            Pos p(0,base.col()+_params->randomPosDistToBase);
            posInitVec.push_back(p);
        }

        //use the fact that manhatten distance requires one coord to be the distance, other can be random
        bool isOk = false;
        int d = 0;
        do {
            //first decide on which side of the base
            initPos = posInitVec[hsutils::random(posInitVec.size()-1)];
            //set other coord
            if (initPos.row()>0) {
                //set col
                int minCol = base.col()-_params->randomPosDistToBase;
                if (minCol<0) minCol=0;
                int maxCol = base.col()+_params->randomPosDistToBase;
                if (maxCol>=_map->colCount()) maxCol=_map->colCount()-1;
                initPos.set(0, hsutils::random(minCol,maxCol));
            } else {
                //set row
                int minRow = base.row()-_params->randomPosDistToBase;
                if (minRow<0) minRow=0;
                int maxRow = base.row()+_params->randomPosDistToBase;
                if (maxRow>=_map->rowCount()) maxRow=_map->rowCount()-1;
                initPos.set( hsutils::random(minRow,maxRow), 0);
            }

            isOk = _map->isPosInMap(initPos);
            if (isOk) isOk = !_map->isObstacle(initPos);

        } while (!isOk);

        //distance
        d = _map->distance(base,initPos);

        //now move randomly until the distance has been reached
        while(d!=_params->randomPosDistToBase) {
            //choose random action, avoiding staying at the same pose (action 0)
            int a = hsutils::random(HSGlobalData::NUM_ACTIONS-2)+1;

            //try move
            Pos newPos = _map->tryMove(a,initPos);

            if (newPos.isSet()) {
                d = _map->distance(newPos,base);
                initPos = newPos;
            }
        }

        DEBUG_CLIENT(cout<<"Found pos to be at "<<initPos.toString()<<endl;);

    } else if (isSeeker()) {
        //none of previous, and is seeker, so start at base

        initPos = _map->getBase();

        //AG140520
        if (!initPos.isSet()) {
            cout << "AutoPlayer::getInitPos: WARNING: no base at the map, using random pos"<<endl;
            initPos = _map->genRandomPos();
        }

    } else {
        //none of above, and is hider, so start at random hidden pos

        bool ok=false;

        while(!ok) {
            if(_map->numObstacles()==0 || !_map->getBase().isSet()) {
                //if it is a map with no Invisible cells
                //AG140520: or no base (i.e. no known pos of seeker, to test visibility)
                initPos = _map->genRandomPos();

                ok=true;

            } else {
                //get not visible points
                //AG160210: this should include invisible points based on distance
                //AG160216: TODO: check if we should use sim of not visib.
                vector<Pos> invisPoints = _map->getInvisiblePoints(_map->getBase(),_params->takeDynObstOcclusionIntoAccountWhenLearning, false);

                initPos = invisPoints[hsutils::random(invisPoints.size()-1)];

                //AG140528: add random cell location if continuous
                if (_params->useContinuousPos) {
                    initPos.add(hsutils::randomDouble(0,1), hsutils::randomDouble(0,1));
                }cout<<endl;

                ok=true;
            }

            if (initPos == _map->getBase()) {
                ok = false;  //assume seeker starts at base //TODO: should be at a win dist...
            }
        }
    }

    if (_params->useContinuousPos && !initPos.hasDouble()) {
        //set values to center of cells
        initPos.add(0.5,0.5);
    }

    return initPos;
}


bool AutoPlayer::initBelief(GMap* gmap, PlayerInfo* otherPlayer) {
    setMap(gmap); //to be sure that other functions are called (e.g. in auto hider)
    assert(otherPlayer!=NULL);
    /*assert(gmap!=NULL);
    _map = gmap;*/

    //just create a vector of 2
    _playerInfoVec.resize(2);
    //and set indexes
    _thisPlayerID = 0;

    _playerInfoVec[_thisPlayerID] = &playerInfo;
    _playerInfoVec[1] = otherPlayer;

    if (isSeeker()) {
        _hiderPlayerID = 1;
        assert(!otherPlayer->isSeeker());

        _hiderPlayer = otherPlayer;
        _seekerPlayer1 = &playerInfo;
    } else {
        _hiderPlayerID = _thisPlayerID;
        assert(otherPlayer->isSeeker());

        _hiderPlayer = &playerInfo;
        _seekerPlayer1 = otherPlayer;
    }

    //ag150706: check prob sum=1
    //the sum over all seekers should be 1
    //assert(_seekerPlayer1->useObsProb == 1);
    assert(checkSumObsProbIs1());

    //call the init belief
    return initBeliefRun();
}


bool AutoPlayer::initBeliefMulti(GMap* gmap, std::vector<PlayerInfo*> playerVec, int thisPlayerID, int hiderPlayerID) {
    setMap(gmap); //to be sure that other functions are called (e.g. in auto hider)
    /*assert(gmap!=NULL);
    _map = gmap;*/
    _playerInfoVec = playerVec;

    assert(playerVec.size()>1);
    assert(thisPlayerID>=0 && thisPlayerID<(int)_playerInfoVec.size());
    assert(hiderPlayerID==-1 || (hiderPlayerID>=0 && hiderPlayerID<(int)_playerInfoVec.size()));

    _hiderPlayerID = hiderPlayerID;
    _thisPlayerID = thisPlayerID;

    if (hiderPlayerID==-1) {
        _hiderPlayer = NULL;
    } else {
        _hiderPlayer = _playerInfoVec[hiderPlayerID];
    }

    //set first player (mainly for hider)
    if (_hiderPlayerID==0) {
        _seekerPlayer1 = _playerInfoVec[1];
    } else {
        _seekerPlayer1 = _playerInfoVec[0];
    }
    assert(_seekerPlayer1->isSeeker());

    //ag150706: check prob sum=1
    //the sum over all seekers should be 1
    assert(checkSumObsProbIs1());

    //call the init belief
    return initBeliefRun();
}

bool AutoPlayer::checkSumObsProbIs1() {
    double psum = 0;
    for(PlayerInfo* playerI : _playerInfoVec) {
        if (playerI->isSeeker())
            psum += playerI->useObsProb;
    }
    return (abs(1.0-psum)<0.0001);
}

double AutoPlayer::getNextDirection(/*Pos seekerPos, Pos hiderPos, bool opponentVisible,*/ bool &haltAction) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::getNextDirection: not implemented, should be implemented by subclass, or call getNextAction()");
    return -1;
}


Pos AutoPlayer::getNextPos(int actionDone, int *newActionRet) {
    //TODO check where these are used and if they have to be updated here or in an other place

    //action to store
    int newAction = -1;

    //get the next pos from the algorithm
    playerInfo.nextPos = getNextPosRun(actionDone, &newAction);

    //AG150624: set chosenPos if not set
    if (playerInfo.nextPos.isSet() && !playerInfo.chosenGoalPos.isSet())
        playerInfo.chosenGoalPos = playerInfo.nextPos;

    //increase nr of iterations
    _numIterations++;

    //set action
    playerInfo.lastAction = newAction;

    if (newActionRet!=NULL) {
        if (newAction==-1) {
            //deduce action
            newAction = deduceAction(playerInfo.currentPos, playerInfo.nextPos);
            assert(newAction>=0 && newAction<HSGlobalData::NUM_ACTIONS);
        }
        *newActionRet = newAction;
    }

    return playerInfo.nextPos;
}

Pos AutoPlayer::getNextPosRun(int actionDone, int *newAction) {
    //NOTE: this function will run only if it is not overriden
    // it then assumes that the function getNextDirection is implemented

    Pos nextPos;

    bool haltAction = false;
    //get the action with direction
    double dir = getNextDirection(haltAction);

    if (haltAction) {
        //action is halt
        nextPos = playerInfo.currentPos;

        playerInfo.lastAction = HSGlobalData::ACT_H;
    } else {
        //distance
        double dist = /*n **/ _params->seekerStepDistance;
        unsigned int tries = 0;
        do {
            //try to move
            nextPos = _map->tryMoveDir(dir, playerInfo.currentPos /*curPos*/ , dist, true);
            tries++;
            dist /= 2; //lower distance if moving is not possible
        } while (tries<2 && !nextPos.isSet());

        if (!nextPos.isSet()) {
            cout << "WARNING: was not able to move in direction "<<(180*dir/M_PI)<<"º, last tried distance is "
                 <<dist<<" cells, now using current position."<<endl;

            playerInfo.lastAction = -1;
        } else {
            playerInfo.lastAction = deduceAction(dir);
        }
    }

    return nextPos;
}

Pos AutoPlayer::selectRobotPosMulti() {    //not implemented
    throw CException(_HERE_, "AutoPlayer::selectRobotPosMulti: not implemented, should be implemented by subclass");
    return Pos();
}

bool AutoPlayer::calcNextRobotPoses2(int actionDone) {
    _numIterations++;

    return calcNextRobotPoses2Run(actionDone);
}

bool AutoPlayer::calcNextHBList(int actionDone) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::calcNextHBList: not implemented, should be implemented by subclass");
    return false;
}

bool AutoPlayer::calcNextRobotPoses2Run(int actionDone) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::calcNextRobotPoses2Run: not implemented, should be implemented by subclass");
    return false;
}

bool AutoPlayer::calcNextHBListRun(int actionDone) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::calcNextHBListRun: not implemented, should be implemented by subclass");
    return false;
}

bool AutoPlayer::tracksBelief() const {
    return false;
}


double AutoPlayer::scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone) {
    throw CException(_HERE_, "AutoPlayer::scoreObservation: the scoreObservation function has not been implemented");
    return -1;
}

double AutoPlayer::getBelief(int r, int c) {
    throw CException(_HERE_, "AutoPlayer::getBelief: the getBelief function has not been implemented");
    return -1;
}


double AutoPlayer::getBelief(const Pos& pos) {
    return getBelief(pos.row(),pos.col());
}


void AutoPlayer::setMap(GMap* map) {
    assert(map!=NULL);
    DEBUG_CLIENT(cout<<"Map set: "<<map->getName()<<" ("<<map->rowCount()<<"x"<<map->colCount()<<", cells="<<map->getCellSize_m()<<"m)"<<endl;);
    _map = map;
}

GMap* AutoPlayer::getMap() const {
    return _map;
}

void AutoPlayer::setInitPos(Pos initPos) {
    _initPos = initPos;
}

/*!
 * \brief canScoreObservations tells whether the auto player can score observations (using scoreObservation())
 * \return
 */
bool AutoPlayer::canScoreObservations() const {
    return false;
}

bool AutoPlayer::handles2Obs() const {
    return false;
}

Pos AutoPlayer::getClosestSeekerObs(Pos seekerPos) {
    if (_map->isPosInMap(seekerPos) && !_map->isObstacle(seekerPos)) {
        cout << "WARNING: AutoPlayer::getClosestSeekerObs returning same seekerPos as passed as param"<<endl;
        return seekerPos;
    }

    //only used for continuous actions
    assert(_params->useContinuousPos);

    Pos returnPos(seekerPos);
    returnPos.convertValuesToInt();
    returnPos.add(0.5,0.5, true);

    if (_map->isPosInMap(returnPos) && !_map->isObstacle(returnPos)) {
        return returnPos;
    }

    //AG NOTE/TODO: this algorithm could be more efficient..
    //still nothing found, so loop all states to find closest state
    double minDist = INFINITY;
    vector<Pos> closestPosVec;
    for(int r=0; r<_map->rowCount(); r++) {
        for(int c=0; c<_map->colCount(); c++) {
            if (!_map->isObstacle(r,c)) {
                //get distance
                double d = _map->distanceEuc(seekerPos.rowDouble(),seekerPos.colDouble(),r+0.5,c+0.5);
                if (d<=minDist) {
                    //is closer
                    if (d<minDist) {
                        minDist = d;
                        closestPosVec.clear();
                    }
                    //add to list of closest points
                    returnPos.set(r+0.5,c+0.5);
                    closestPosVec.push_back(returnPos);
                }
            }
        }
    }

    //get position to return
    switch (closestPosVec.size()) {
        case 0:
            throw CException(_HERE_, "current seeker pos is not legal, and no closest seeker position has been found.");
            break;
        case 1:
            returnPos = closestPosVec[0];
            break;
        default:
            returnPos = closestPosVec[hsutils::random(closestPosVec.size()-1)];
            break;
    }

    return returnPos;
}

string AutoPlayer::getMapBeliefProbAsString(int rows, int cols, int w) {
    if (!tracksBelief()) {
        //doesn't track belief
        return "[NO BELIEF]";
    }

    stringstream ss;

    for(int r=-1; r<=_map->rowCount(); r++) {
        for(int c=-1; c<=_map->colCount(); c++) {
            if (r==-1 || r==_map->rowCount()) {
                //show header up and below
                if (c==-1 || c==_map->colCount()) {
                    ss << setw(w) << "r/c";
                } else {
                    ss << setw(w) << c;
                }
            } else if (c==-1 || c==_map->colCount()) {
                //column headers
                //if (r==-1 || r==map->rowCount()) {
                ss << setw(w) << r;
            } else if (_map->isObstacle(r,c)) {
                ss << setw(w+1) << 'X';
            } else {
                ss << " " << setprecision(w-2) << setw(w) << getBelief(r,c);
            }
        }
        ss << endl;
    }

    return ss.str();
}

bool AutoPlayer::storeMapBeliefAsImage(std::string file, int cw) {
#ifndef DO_NOT_WRITE_BELIEF_IMG
    cv::Mat* image = getMapBeliefAsImage(cw);

    if (image==NULL) {
        return false;
    } else {
        //write image
        bool ok = cv::imwrite(file,*image);
        //delete object
        delete image;

        return ok;
    }
#else
    return false;
#endif
}

#ifndef DO_NOT_WRITE_BELIEF_IMG
bool AutoPlayer::storeImage(cv::Mat *image, string file) {
    if (image==NULL) {
        return false;
    } else {
        //write image
        bool ok = cv::imwrite(file,*image);

        return ok;
    }
}

void AutoPlayer::paintCircleOnImage(cv::Mat *image, const Pos &p, const cv::Scalar &color, double origCellSize, double radiusPart) {
    double r = p.rowDouble() * origCellSize;
    double c = p.colDouble() * origCellSize; // /_params->beliefMapZoomFactor;
    int x;
    int y;

    //AG150210: round -> floor (it are indices of image cells/pixels

    if (_params->useContinuousPos) {
        x = (int)floor(c);
        y = (int)floor(r);
    } else {
        x = (int)floor(c+origCellSize/2.0);
        y = (int)floor(r+origCellSize/2.0);
    }

    int radius =  (int)(radiusPart*origCellSize/2.0);
    cv::circle(*image, cv::Point(x,y), radius, color, 1);
}

void AutoPlayer::paintCrossOnImage(cv::Mat *image, const Pos &p, const cv::Scalar &color, double origCellSize, double radiusPart) {
    double r = p.rowDouble() * origCellSize;
    double c = p.colDouble() * origCellSize; // /_params->beliefMapZoomFactor;
    int x;
    int y;

    //AG150210: round -> floor (it are indices of image cells/pixels

    if (_params->useContinuousPos) {
        x = (int)floor(c);
        y = (int)floor(r);
    } else {
        x = (int)floor(c+origCellSize/2.0);
        y = (int)floor(r+origCellSize/2.0);
    }

    int w =  (int)((radiusPart*origCellSize/2.0)/M_SQRT2);
    cv::line(*image, cv::Point(x-w,y-w), cv::Point(x+w,y+w), color, 1);
    cv::line(*image, cv::Point(x-w,y+w), cv::Point(x+w,y-w), color, 1);
}

cv::Mat* AutoPlayer::getMapBeliefAsImage(int cw) {
    if (!tracksBelief()) {
        return NULL;
    }
    unsigned int zrows,zcols;
    double** belief = getBeliefZoomMatrix(zrows, zcols);

    int imgRows = (int)zrows*cw;
    int imgCols = (int)zcols*cw;

    cv::Mat* image = new cv::Mat(imgRows, imgCols /*_map->rowCount()*cw,_map->colCount()*cw*/,CV_8UC3);

    cv::Scalar color;

    //paint img
    for(int r=0; r<(int)zrows; r++) {
        for(int c=0; c<(int)zcols; c++) {
            //belief
            double b = belief[r][c];

            if (b<0) {
                //obstacle is black
                color = cv::Scalar(0,0,0);
            } else if (_params->gameType==HSGlobalData::GAME_HIDE_AND_SEEK && _map->isBase(r,c)) {
                //base is green
                color = cv::Scalar(0,255,0);
            } else {
                //free field is white, and if probability 1 of hider then red

                if (b==0) {
                    bool hasSeeker = false;

                    if (hasSeeker) {
                        color = cv::Scalar(255,0,0);
                    } else {
                        color = cv::Scalar(255,200,200);
                    }
                } else {
                    int v = (int) round( (1-b)*255 );
                    color = cv::Scalar(v,v,255);
                }
            }
            cv::rectangle(*image, cv::Point(c*cw,r*cw),cv::Point((c+1)*cw,(r+1)*cw),color,CV_FILLED);
        }
    }

    if (belief!=NULL)
        FreeDynamicArray<double>(belief,zrows);

    //obstacles
    color = cv::Scalar(0,0,0);
    double origCellSize = 1.0*imgRows/_map->rowCount();
    int origCellSizeInt = (int)(origCellSize);

    //'paint' obstacles
    for (int i = 0; i <_map->numObstacles(); i++) {
        Pos p = _map->getObstacle(i);
        int y = (int)round(p.rowDouble()*origCellSize);
        int x = (int)round(p.colDouble()*origCellSize);
        cv::rectangle(*image, cv::Point(x,y),cv::Point(x+origCellSizeInt,y+origCellSizeInt),color,CV_FILLED);
    }

    //paint seeker
    paintCircleOnImage(image, playerInfo.currentPos /*seekerPos*/, cv::Scalar(255,0,0), origCellSize);
    if (playerInfo.chosenGoalPos.isSet()) { // print it's goal
        paintCrossOnImage(image, playerInfo.chosenGoalPos, cv::Scalar(255,0,0), origCellSize);
    }
    //AG160511: dyn. obst.
    for(const Pos& pos : playerInfo.dynObsVisibleVec) {
        paintCircleOnImage(image, pos, cv::Scalar(255,255,0), origCellSize, 0.6);
    }

    //AG150521: paint other seekers
    for(PlayerInfo* p : _playerInfoVec) { //cout<<" - "<<p->toString(true)<<endl;
        if (p->isSeeker() && *p!=playerInfo && p->posRead) {
            //other seeker
            paintCircleOnImage(image, p->currentPos, cv::Scalar(255,128,128), origCellSize, 0.7);

            //seeker's observation
            if (p->hiderObsPosWNoise.isSet()) {
                paintCircleOnImage(image, p->hiderObsPosWNoise, cv::Scalar(0,69,255), origCellSize,0.7);
            }

            //goal
            if (p->chosenGoalPos.isSet()) {
                    paintCrossOnImage(image, p->chosenGoalPos, cv::Scalar(255,128,128), origCellSize, 0.7);
            }
            //AG160511: dyn. obst.
            for(const Pos& pos : p->dynObsVisibleVec) {
                paintCircleOnImage(image, pos, cv::Scalar(200,200,0), origCellSize, 0.5);
            }
        }
    }

    //paint obs of hider
    if (playerInfo.hiderObsPosWNoise.isSet()) {
        paintCircleOnImage(image, playerInfo.hiderObsPosWNoise, cv::Scalar(0,0,128), origCellSize);
    }

    return image;
}
#endif //DO_NOT_WRITE_BELIEF_IMG

void AutoPlayer::checkAndFilterPoses(const Pos &seekerPos, const std::vector<IDPos> &hiderPosVector, Pos &seekerPosOut,
                                     IDPos &hiderPosOut, bool& dontExecuteIteration) {
    //check seeker pos
    if (checkValidNextSeekerPos(seekerPos,false)) {
        seekerPosOut = seekerPos;
    } else {
        //'fixing' seeker pos
        if (_params->useContinuousPos) {
            seekerPosOut = getClosestSeekerObs(seekerPos);
            DEBUG_SHS(cout<<"AutoPlayer::checkAndFilterPoses - fixed seeker pos: "<<seekerPosOut.toString()<<endl;);
        } else {
            seekerPosOut = seekerPos;
            DEBUG_SHS(cout<<"AutoPlayer::checkAndFilterPoses - incorrect seeker pos: "<<seekerPosOut.toString()<<" BUT NO FIX!"<<endl;);
        }
    }

    //check hider pos
    hiderPosOut = chooseHiderPos(seekerPosOut, hiderPosVector, false, dontExecuteIteration);
    DEBUG_SHS(cout<<"AutoPlayer::checkAndFilterPoses - chosen hider pos: "<<hiderPosOut.toString()<<endl;);
}

int AutoPlayer::deduceAction(double angle) {
    //not the halt action, get the direction
    double ang = 180*angle/M_PI;

    assert(ang>=0 && ang<=360);

    int a = -1;

    if (/*ang>337.5 ||*/ ang <=22.5) {
        a = HSGlobalData::ACT_N;
    } else if (ang<=67.5) {
        a = HSGlobalData::ACT_NE;
    } else if (ang<=112.5) {
        a = HSGlobalData::ACT_E;
    } else if (ang<=157.5) {
        a = HSGlobalData::ACT_SE;
    } else if (ang<=202.5) {
        a = HSGlobalData::ACT_S;
    }  else if (ang<=247.5) {
        a = HSGlobalData::ACT_SW;
    } else if (ang<=292.5) {
        a = HSGlobalData::ACT_W;
    } else if (ang<=337.5) {
        a = HSGlobalData::ACT_NW;
    } else {
        //throw CException(_HERE_,"AutoPlayer::deduceAction: this case should not have happened");
        a = HSGlobalData::ACT_N;
    }

    return a;
}

int AutoPlayer::deduceAction(const Pos& fromPos, const Pos& toPos) {
    if (!fromPos.isSet() || !toPos.isSet()) return -1;

    int a = -1;
    //distance
    DEBUG_CLIENT(cout<<"Deduce action "<<fromPos.toString()<<"->"<<toPos.toString(););

    double dist = _map->distanceEuc(fromPos,toPos);

    DEBUG_CLIENT(cout <<"[d="<<dist<<"]:";); //<<endl;

    if (dist > _params->seekerStepDistance*_params->seekerStepDistancePartForHaltActionDeduction) {

        //deduce action base on angle
        a = deduceAction(_map->getDirection(fromPos,toPos));
    } else {
        a = HSGlobalData::ACT_H;
    }

    DEBUG_CLIENT(cout<<ACTION_COUT(a)<<endl;);

    return a;
}

int AutoPlayer::deduceAction() {
    //assert(_thisPlayer!=NULL);
    return deduceAction(playerInfo.previousPos, playerInfo.currentPos);
}

double AutoPlayer::getBeliefScore(Pos hiderPos) {
    if (!tracksBelief()) {
        //doesn't track belief
        throw CException(_HERE_, "AutoPlayer::getBeliefScore: belief is not tracked");
    }
    if (!hiderPos.isSet()) {
        throw CException(_HERE_, "AutoPlayer::getBeliefScore: hider pos is not set");
    }

    double score = 0;

    //calculate the weighted belief score:
    // sum(b[r][c] * dist((r,c),hiderPos)))
    for(int r=0; r<_map->rowCount(); r++) {
        for(int c=0; c<_map->colCount(); c++) {
            if (!_map->isObstacle(r,c)) {
                double b = getBelief(r,c);
                if (b>0) {
                    //add the weighted score for this cell
                    score += b*_map->distance(r,c,hiderPos.row(),hiderPos.col());

                }
            }
        }
    }

    return score;
}

IDPos AutoPlayer::chooseHiderPos(const Pos& seekerPos, const std::vector<IDPos>& hiderPosVector, bool checkPrev, bool& dontExecuteIteration) {
    assert(seekerPos.isSet() && _map->isPosInMap(seekerPos) && !_map->isObstacle(seekerPos));

    IDPos chosenHiderPos;
    dontExecuteIteration = false;

    //last hider pos
    Pos lastHiderPos;
    if (_hiderPlayer!=NULL)
        lastHiderPos = _hiderPlayer->previousPos;

    //There are several variants of scoring to choose the hider
    switch(_params->filterScoreType) {
        case HSGlobalData::FILTER_SCORE_OLD: {
            throw CException(_HERE_, "AutoPlayer::chooseHiderPos: FILTER_SCORE_OLD not implemented");
            break;
        }
        case HSGlobalData::FILTER_SCORE_NEW1:
        case HSGlobalData::FILTER_SCORE_NEW2_WEIGHTED:
        case HSGlobalData::FILTER_SCORE_NEW2_PREF_TAG: {
            //AG150122: copy because we should not change the const hiderPosVector
            std::vector<IDPos> hiderPosVector2(hiderPosVector);

            DEBUG_SHS(cout << "AutoPlayer::chooseHiderPos(v2): checking "<<hiderPosVector2.size()<< " hider poses (seeker="
                      <<seekerPos.toString()<<"):"<<endl;);

            //max score
            double maxS = 0;
            //list of observations (index) with max score
            vector<size_t> maxScoreObsVec;

            //AG140310: always score the hidden observation
            //note2: always added as last
            IDPos hiddenPos;
            hiderPosVector2.push_back(hiddenPos);
            //AG140310: indicates whether observation of a visible point has been found
            //bool visibObsFound = false;
            //last action, can be deduced or not
            int lastAction = -2;
            if (_params->useDeducedAction && /*_lastSeekerPos*/ playerInfo.previousPos.isSet()) { //AG150526: assume lastSeekerPos=currentPos (i.e. last read)
                lastAction = deduceAction(/*_lastSeekerPos*/ playerInfo.previousPos, seekerPos);
            } else {
                lastAction = -1;
            }

            DEBUG_SHS(
                cout<<"Deduced action: ";
                if (lastAction<0)
                cout <<"unknown ("<<lastAction<<")"<<endl;
                else
                    cout <<ACTION_COUT(lastAction)<<endl;
                );

            //AG140508: update score
            if (_params->hiderStepDistance /*+_params->contNextHiderStateStdDev*/ > _params->filterDistScoreMaxDist)
                cout << "AutoPlayer::chooseHiderPos: WARNING: hiderStepDistance ("<<_params->hiderStepDistance<<") > distScoreMaxDist ("
                     <<_params->filterDistScoreMaxDist<<")"<<endl;

            //validate the hider pos and choose 'best' option
            for(size_t i = 0; i < hiderPosVector2.size(); i++) {
                //the hider pos to be checked
                const IDPos& hiderPos = hiderPosVector2[i];

                //get score
                double score = 0;
                double sbase = 0;

                if (hiderPos.isSet()) {
                    if (hiderPos.id()==0) {
                        //AG140411: an id of 0 means that it comes form the tag detector, and should be much more robust
                        sbase = _params->filterTagBaseScore;
                    } else {
                        sbase = _params->filterLaserBaseScore; //ag140122: hider detected, but we don't care for the score of the measurement itself   hiderPosVector[i][2];
                    }
                } else {
                    sbase = _params->filterHiddenBaseScore; // hidden position, if it is added artificially
                }

                DEBUG_SHS(
                    cout << " * hider = (";
                if (!hiderPos.isSet()) {
                    cout<<"hidden";
                } else {
                    if (hiderPos.id()==0) {
                        cout <<"tag; ";
                    } else {
                        cout <<"laser; ";
                    }
                    cout<<hiderPos.toString();
                }
                if (hiderPos.id()>=0)
                cout << "; id="<<hiderPos.id();
                cout <<"), base score="<<sbase<<" ["<<flush;
                );

                //check
                if (!checkValidNextHiderPos(hiderPos, seekerPos, checkPrev)) {
                    DEBUG_SHS(cout<<"]: not valid!"<<endl;);
                    continue; //skip since not valid
                }

                DEBUG_SHS(cout<<"]";);

                if (_params->filterScoreType==HSGlobalData::FILTER_SCORE_NEW2_PREF_TAG && hiderPos.id()==0) {
                    //AG140507: if we prefer tag, just stop and choose this
                    DEBUG_SHS(cout<<"found a tag!"<<endl;);
                    maxScoreObsVec.clear();
                    maxScoreObsVec.push_back(i);;
                    break; //stop since we found the tage
                }

                if (_params->simulateNotVisible) {
                    if (hiderPos.isSet() && !_map->isVisible(seekerPos,hiderPos,false,_params->simNotVisibDist)) {
                        DEBUG_SHS(cout<<"] simulating NOT visible, ignoring"<<endl;);
                        continue;
                    }
                }

                double obsScore = 0;
                if (canScoreObservations()) {
                    //AG140117: check if we can score observations
                    obsScore = scoreObservation(seekerPos, hiderPos, lastAction);

                    DEBUG_SHS(cout << ", obs score="<<obsScore<<flush;);

                    if (obsScore>0) {
                        score += obsScore;
                        //if (hiderPos.isSet()) visibObsFound = true;
                    }
                } else {
                    DEBUG_SHS(cout<<", (no obs score)"<<flush;);
                }

                //AG140122: use distance to prev as score
                //(AG140411: added check of hidden previous pos)
                if (hiderPos.isSet() && /*_lastHiderPos*/ lastHiderPos.isSet()) {
                    //the closer, the higher a score, BUT we need to prevent the score from
                    //growing exponantially, therefore round them (now 0.5)
                    //AG TODO: maybe an exponential function instead of continuous
                    double d = _map->distanceEuc(/*_lastHiderPos*/ lastHiderPos, hiderPos); //AG150527: assume that the previous hider has been set
                    assert(d>=0);

                    DEBUG_SHS(cout<<", dist score="<<flush;);

                    if (d < _params->filterDistScoreMaxDist) {
                        double distScore = 0;

                        //AG140508: update score
                        if (d <= _params->hiderStepDistance/*+_params->contNextHiderStateStdDev*/) {
                            distScore = 1;
                        } else {
                            distScore = 1 - (d - _params->hiderStepDistance) / (_params->filterDistScoreMaxDist - _params->hiderStepDistance);
                            //if (distScore<0) cout <<"distscore="<<distScore<<";d="<<d<<";hiderstepd="<<_params->hiderStepDistance<<";filterdistscmaxd="<<_params->filterDistScoreMaxDist<<endl;
                            assert(distScore>=0);
                        }

                        //AG140425: weight based on num sim, to give more priority
                        if (_params->filterScoreType == HSGlobalData::FILTER_SCORE_NEW1) {
                            score += distScore;
                        } else {
                            score += distScore / _params->numSim;
                        }

                        DEBUG_SHS(cout<<distScore<<flush;);
                    } else {
                        DEBUG_SHS(cout<<", 0 (too far)"<<flush;);
                    }


                }  else if (obsScore==0) {
                    score = _params->filterMinScore;

                    //AG140416: first action, put a score in order to be able to compare with base score (otherwise all 0)
                    DEBUG_SHS(cout<<", cannot score obs->score="<<score<<flush;);
                }

                DEBUG_SHS(cout<<", [";);

                if (checkPrev || checkValidNextHiderPos(hiderPos,seekerPos, true)) {
                    score *= 2; //increase score because it is consistent with prev score
                    DEBUG_SHS(cout<<"] valid score (x2)"<<flush;);
                } else {
                    DEBUG_SHS(cout<<"]";);
                }

                score *= sbase;

                DEBUG_SHS(cout << ": total score="<<score;);

                if (score >= maxS) {
                    //AG140310: max score AND either visible OR NOT visible AND no other visible observations found
                    if (score > maxS) {
                        maxScoreObsVec.clear();
                        maxS = score;
                    }

                    maxScoreObsVec.push_back(i);
                }
                DEBUG_SHS(cout << endl;);
            } // for all poses

            //index
            int i = -1;

            if (maxScoreObsVec.size()==1) {
                i = maxScoreObsVec[0];
            } else if (maxScoreObsVec.size()>1) {
                //randomly choose a best one
                i = maxScoreObsVec[hsutils::random(maxScoreObsVec.size()-1)];
            } // else: empty

            if (i>=0) {
                chosenHiderPos = hiderPosVector2[i];
            }

            break;
        }

        case HSGlobalData::FILTER_SCORE_USE_TAG_ONLY:
        case HSGlobalData::FILTER_SCORE_USE_ID: {
            DEBUG_SHS(
                cout << "AutoPlayer::chooseHiderPos(use track id): checking "<<hiderPosVector.size()<< " hider poses (seeker = "
                    <<seekerPos.toString()<<"), "<<flush;
                if (_params->filterScoreType==HSGlobalData::FILTER_SCORE_USE_TAG_ONLY)
                    cout << "searching tag:"<<flush;
                else
                    cout << "searching ID "<<_params->filterFollowID<<": "<<flush;
                );

            //follower ID
            int followerID = 0;
            if (_params->filterScoreType==HSGlobalData::FILTER_SCORE_USE_ID) {
                followerID = _params->filterFollowID;
            } //else followerID = 0 -> tag ID

            bool found = false;

            //search for the id, which should be on the 3rd position of the vector
            //NOTE: it assumes that the ID is unique
            for(const IDPos& hiderPos : hiderPosVector) {
                if (hiderPos.id()==followerID) {
                    //found the ID
                    found = true;

                    if (_params->simulateNotVisible) {
                        //simulating not visible hider
                        if (hiderPos.isSet() && _map->isPosInMap(hiderPos) && !_map->isObstacle(hiderPos) &&
                                _map->isVisible(seekerPos,hiderPos,true,_params->simNotVisibDist)) { //AG140526: also use dyn.obst. to occlude

                            chosenHiderPos = hiderPos;
                        } //else: simulate not visible
                    } else {
                        chosenHiderPos = hiderPos;
                    }

                    break;
                }
            }

            //check
            if (!checkValidNextHiderPos(chosenHiderPos, seekerPos, checkPrev)) {
                DEBUG_SHS(cout<<"found, but not valid"<<endl;);
                chosenHiderPos.clear();
            } else {
                DEBUG_SHS(
                if (found) {
                    cout << "FOUND"<<flush;
                    if (!chosenHiderPos.isSet()) {
                            cout << ", but simulating not visible";
                        }
                } else {
                    cout << "NOT FOUND";
                }
                cout << endl;
                );
            }
            break;
        }

        default: {
            throw CException(_HERE_,"AutoPlayer::chooseHiderPos: unknown filter score type");
            break;
        }

    }//switch score type


    //AG140514: filter based on history
    if (_params->filterCanStopNumberOfIterations>0 && _numIterations>0) {
        DEBUG_SHS(cout<<"Filter based on history: ";);
        dontExecuteIteration = true;
        //first check if there is an observation score
        if (canScoreObservations()) {
            int lastAction = -2;
            if (_params->useDeducedAction && /*_lastSeekerPos*/ playerInfo.previousPos.isSet()) {
                lastAction = deduceAction(/*_lastSeekerPos*/playerInfo.previousPos, seekerPos);
            } else {
                lastAction = -1;
            }

            //AG140117: check if we can score observations
            double obsScore = scoreObservation(seekerPos, chosenHiderPos, lastAction);
            if (obsScore>0) {
                dontExecuteIteration = false;
            }
            DEBUG_SHS(cout<<", obs score="<<obsScore;);
        }

        if (dontExecuteIteration && chosenHiderPos.isSet() && /*_lastSeekerPos*/ playerInfo.previousPos.isSet()) {
            double d = _map->distanceEuc(lastHiderPos,chosenHiderPos);
            DEBUG_SHS(cout<<", dist="<<d<<"(maxScoreDist="<<_params->filterDistScoreMaxDist<<",numItSkippd="<<_numIterationsSkipped<<")";);
            if (d < _params->filterDistScoreMaxDist * (_numIterationsSkipped+1) ) {
                dontExecuteIteration = false;
            }
        }
        DEBUG_SHS(cout<<endl;);

        if (dontExecuteIteration) {
            DEBUG_SHS(cout<<" - Not accepting because of previous ("<<_numIterationsSkipped<<"/"<<_params->filterCanStopNumberOfIterations<<")";);
            if (_numIterationsSkipped >= _params->filterCanStopNumberOfIterations) {
                dontExecuteIteration = false;
                _numIterationsSkipped = 0;
                DEBUG_SHS(cout<<", but too many stopped, so accepting";);
            } else {
                _numIterationsSkipped++;
            }
            DEBUG_SHS(cout<<endl;);
        } else {
            _numIterationsSkipped = 0;
        }
    }

    DEBUG_SHS(
        cout << " -> CHOSEN HIDER POS.: "<<flush;
        if (chosenHiderPos.isSet())
        cout <<chosenHiderPos.toString()<<endl;
        else
            cout << "hidden"<<endl;
        );

    return chosenHiderPos;
}



bool AutoPlayer::checkValidNextSeekerPos(const Pos& seekerNextPos, bool checkPrev) const {
    bool valid = true;

    if (!_map->isPosInMap(seekerNextPos)) {
        //pos out of map
        DEBUG_SHS(cout << "AutoPlayer::checkValidNextSeekerPos: ERROR position is out of map"<<endl;);
        valid = false;
    } else if (_map->isObstacle(seekerNextPos)) {
        //check if obstacle
        DEBUG_SHS(cout << "AutoPlayer::checkValidNextSeekerPos: ERROR position is an obstacle"<<endl;);
        valid = false;
    } else if (checkPrev && _map->distance(/*_lastSeekerPos*/ playerInfo.previousPos, seekerNextPos)>_params->hiderStepDistance+_params->contNextSeekerStateStdDev*2) {
        //distance is higher than 1
        DEBUG_SHS(cout << "AutoPlayer::checkValidNextSeekerPos: ERROR distance to to previous position higher than step+2*std ("<<
                  _params->hiderStepDistance+_params->contNextSeekerStateStdDev*2<<")"<<endl;);
        valid = false;
    }

#ifdef DEBUG_SHS_ON
    if (!valid) {
        cout << "  NOT VALID Next seeker pos "<<seekerNextPos.toString();
        if (checkPrev)
            cout << " (current pos:"<< playerInfo.previousPos.toString()<<")";
        cout<<endl;
    }
#endif

    //ok
    return valid;
}

bool AutoPlayer::checkValidNextHiderPos(const IDPos& hiderNextPos, const Pos& seekerNextPos, bool checkPrev) {
    Pos lastHiderPos;
    if (_hiderPlayer!=NULL)
        lastHiderPos = _hiderPlayer->previousPos;

    bool lastVisib = lastHiderPos.isSet();
    bool visibNext = hiderNextPos.isSet();
    bool valid = true;

    if (visibNext && !_map->isPosInMap(hiderNextPos)) {
        //pos out of map
        DEBUG_SHS(cout << "AutoPlayer::checkValidNextHiderPos: ERROR position is out of map"<<endl;);
        valid = false;
    } else if (visibNext && _map->isObstacle(hiderNextPos)) {
        //check if obstacle
        DEBUG_SHS(cout << "AutoPlayer::checkValidNextHiderPos: ERROR position is an obstacle"<<endl;);
        valid = false;
    } else if (_map->numObstacles()==0 && !visibNext) {
        DEBUG_SHS(cout << "AutoPlayer::checkValidNextHiderPos: ERROR position is hidden but there are no obstacles"<<endl;);
        valid = false;
    } else if (checkPrev) {
        //ag130417: imply my raytracing algo!  TODO: improve
        //
        //DEBUG_SHS(cout<<"visib="<<visib<<",visibNext="<<visibNext<<endl;);

        //AG150527: assume the previous pos has been set
        Pos lastSeekerPos = playerInfo.previousPos;


        if (lastVisib) {
            lastVisib = _map->isVisible(lastSeekerPos,lastHiderPos,false, false); //AG160216: TODO check if using max dist
            DEBUG_SHS(if (!lastVisib) cout << " (current visible but according to raytrace not) "<<flush;);
        }
        if (visibNext) {
            visibNext = _map->isVisible(seekerNextPos,hiderNextPos,false, false); //AG160216: TODO check if using max dist
            DEBUG_SHS(if (!visibNext) cout << " (next visible but according to raytrace not) "<<flush);
        }

        if (lastVisib && visibNext) {
            /*if (_map->distance(hiderPos,hiderNextPos)>1) {
                //distance is higher than 1
                DEBUG_SHS(cout << "ERROR distance to previous position higher than 1"<<endl;);
                valid = false;
            }*/
        } else if (!lastVisib && !visibNext) {
            //DEBUG_SHS(cout<<" - stays hidden"<<endl;);
            //AG TODO: this could als be a incorrect
        } else {

            if (lastVisib) { // && !visibNext
                valid = false;
                //check if any of next pos is invisible seen from robot
                for(int r=lastHiderPos.row()-1; !valid && r<=lastHiderPos.row()+1; r++) {
                    for(int c=lastHiderPos.col()-1; !valid && c<=lastHiderPos.col()+1; c++) {
                        if (_map->isPosInMap(r,c) && !_map->isObstacle(r,c)) {
                            if (!_map->isVisible(seekerNextPos.row(), seekerNextPos.col(), r, c, false, false)) { //AG160216: TODO check if using max dist
                                //DEBUG_SHS(cout<<"at least one next hidden - ok"<<endl;);
                                valid = true;
                            }
                        }
                    }
                }

                DEBUG_SHS(if (!valid) cout<<"AutoPlayer::checkValidNextHiderPos: no hidden cells founds next to previous"<<endl;);

            } else if (visibNext) { // && !visib

                if (tracksBelief()) {
                    valid = false;
                    //check if previous was invisible, then there should be a belief >0 for any neigbhouring cell (or itself)
                    for(int r=hiderNextPos.row()-1; !valid && r<=hiderNextPos.row()+1; r++) {
                        for(int c=hiderNextPos.col()-1; !valid && c<=hiderNextPos.col()+1; c++) {
                            if (_map->isPosInMap(r,c) && !_map->isObstacle(r,c)) {
                                if (getBelief(r,c)>0) {//
                                    //if (!_map->isVisible(r,c,seekerNextPos.row,seekerNextPos.col)) {
                                    //DEBUG_SHS(cout<<"at least one belief>0 close - ok"<<endl;);
                                    valid = true;
                                }
                            }
                        }
                    }

                    DEBUG_SHS(if (!valid) cout<<"AutoPlayer::checkValidNextHiderPos: no cell with belief>0 next to new location"<<endl;);
                } /*else {
                    DEBUG_SHS(cout<<"can't check belief with current auto player"<<endl;);
                }*/

            }
        }
    }

#ifdef DEBUG_SHS_ON
    if (!valid) {
        cout << "  NOT VALID next hider pos "<<(visibNext?hiderNextPos.toString():"hidden");
        if (checkPrev)
            cout << " (current pos:"<<(hiderNextPos.isSet()?hiderNextPos.toString():"hidden")<<")";
        cout <<"; seeker next pos: "<<seekerNextPos.toString();// <<endl;
    }
#endif

    return valid;
}

Pos AutoPlayer::getNextPosAsStep(const Pos& seekerPos, const Pos& goalPos, int n, bool stayAtMinWinDistFromgoal) {
    DEBUG_SHS(cout <<"AutoPlayer::getNextPosAsStep: from "<<seekerPos.toString()<<" to "<<goalPos.toString()<<": "<<flush;);

    Pos nextPos;
    if (_params->onlySendStepGoals) {
        //calculate the min distance for which we consider it to have arrived to each goal pos
        double minDist = _params->seekerStepDistance;
        if (stayAtMinWinDistFromgoal)
            minDist = max(minDist, _params->winDist);

        //AG150610: removed ==, since this is same as distanceEuc check, and now do only 1 distance check
        if ( _map->distanceEuc(seekerPos,goalPos) <= minDist) {
            //seeker arrived to the goal
            DEBUG_SHS(cout <<"returning same pos, because close to goal";
                /*if (seekerPos==goalPos)
                    cout << "poses are same";
                else
                    cout<< "close to goal"*/
            );

            nextPos = goalPos;

        } else {
            assert(n==1); // should be 1 for simulation, because pathplanning gives 1 step ahead
            //get next step towards goal following 'pathplanner'
            nextPos = _map->getNextStep(seekerPos, goalPos);

            if (_params->useContinuousPos) {
                //AG140526: we have to make it the center of the cell, since the pathplanner gives discrete positions
                if (!nextPos.hasDouble()) nextPos.add(0.5,0.5);
                //get direction
                double dir = _map->getDirection(seekerPos,nextPos);

                //move in the direction only distance 1 (also in diagonal)
                nextPos = _map->tryMoveDirStep(dir, seekerPos, n, _params->seekerStepDistance, _params->doVisibCheckBeforeMove);

                DEBUG_SHS(cout<<" make sure of dist. 1: dir="<<dir);
            }

            DEBUG_SHS(cout<<" next step: "<<nextPos.toString());
        }
    } else {
        //get distance and direction
        double dist = n*_params->seekerStepDistance;
        double dir = _map->getDirection(seekerPos,goalPos);

        DEBUG_SHS(cout<<" dist="<<dist<<" dir="<<dir;);

        if (stayAtMinWinDistFromgoal) {
            //ensure that the given goal pos is at a minimum distance from the goal pos
            //(this is to prevent collision of the robot with the goal, e.g. a person)
            double dsh = _map->distanceEuc(seekerPos,goalPos);
            if (dist>dsh-_params->winDist) {
                //if the goal is too close, put it from a certain distance from the hider
                dist = dsh-_params->winDist;
                DEBUG_SHS(cout<<" reduced dist="<<dist;);
            }
        }

        //move the given distance to the goal
        nextPos = _map->tryMoveDirStep(dir, seekerPos, dist, _params->seekerStepDistance, _params->doVisibCheckBeforeMove);
    }

    DEBUG_SHS(cout<<"->next pos="<<nextPos.toString()<<endl;);

    return nextPos;
}

void AutoPlayer::setMinDistanceToObstacle(Pos &pos) {
    if (_params->useContinuousPos && _params->minDistToObstacle>0) {

        bool cont = true;

        //now check if obstacle within min dist
        //NOTE: we assume the minDistToObstacle <=1
        for(int i=0; i<5 && cont; i++) {
            //AG140527: added a loop such that
            DEBUG_SHS(cout<<"setMinDistanceToObstacle, i"<<i<<": "<<flush;);

            bool obstLeft = isThereAnObstacleAt(pos,0,-_params->minDistToObstacle);
            bool obstRight = isThereAnObstacleAt(pos,0,_params->minDistToObstacle);
            bool obstUp = isThereAnObstacleAt(pos,-_params->minDistToObstacle,0);
            bool obstDown = isThereAnObstacleAt(pos,_params->minDistToObstacle,0);

            double newCol, newRow;

            //obst. at left/right
            if (obstLeft && obstRight) {
                //put in middle
                newCol = pos.col() + 0.5;
                DEBUG_SHS(cout<<"obst left & right";);
            } else if (obstLeft) {
                newCol = pos.col() + _params->minDistToObstacle;
                DEBUG_SHS(cout<<"obst left";);
            } else if (obstRight) {
                newCol = pos.col() + 1 - _params->minDistToObstacle;
                DEBUG_SHS(cout<<"obst right";);
            } else {
                newCol = pos.colDouble();
            }

            //obst up/down
            if (obstUp && obstDown) {
                //put in middle
                newRow = pos.row() + 0.5;
                DEBUG_SHS(cout<<"obst up & down";);
            } else if (obstUp) {
                newRow = pos.row() + _params->minDistToObstacle;
                DEBUG_SHS(cout<<"obst up";);
            } else if (obstDown) {
                newRow = pos.row() + 1 - _params->minDistToObstacle;
                DEBUG_SHS(cout<<"obst down";);
            } else {
                newRow = pos.rowDouble();
            }

            //if (pos.equals(newRow,newCol) || (!obstLeft && !obstRight && !obstUp && !obstDown)) {
                //handle diagonal obstaclse
                //recalculate distance horizontally and vertically if diagonal distance is minDistToObst
                double minObstDistForDiag = _params->minDistToObstacle / M_SQRT2; // GMap::SQRT_2;

                bool obstLeftTop = isThereAnObstacleAt(pos,-minObstDistForDiag,-minObstDistForDiag);
                bool obstRightTop = isThereAnObstacleAt(pos,-minObstDistForDiag,minObstDistForDiag);
                bool obstLeftBottom = isThereAnObstacleAt(pos,minObstDistForDiag,-minObstDistForDiag);
                bool obstRightBottom = isThereAnObstacleAt(pos,minObstDistForDiag,minObstDistForDiag);

                if (obstLeftTop+obstRightTop+obstLeftBottom+obstRightBottom > 1) {
                    newRow = pos.row() + 0.5;
                    newCol = pos.col() + 0.5;
                    DEBUG_SHS(cout<<"several obst. in diagonal";);
                } else if (obstLeftTop) {
                    //obstacle at left top
                    newRow = pos.row() + minObstDistForDiag;
                    newCol = pos.col() + minObstDistForDiag;
                    DEBUG_SHS(cout<<"obst left-up";);
                } else if (obstRightTop) {
                    //obstacle at right top
                    newRow = pos.row() + minObstDistForDiag;
                    newCol = pos.col() + 1 - minObstDistForDiag;
                    DEBUG_SHS(cout<<"obst right-up";);
                } else if (obstLeftBottom) {
                    //obstacle at left bottom
                    newRow = pos.row() + 1 - minObstDistForDiag;
                    newCol = pos.col() + minObstDistForDiag;
                    DEBUG_SHS(cout<<"obst left-down";);
                } else if (obstRightBottom) {
                    //obstacle at right bottom
                    newRow = pos.row() + 1 - minObstDistForDiag;
                    newCol = pos.col() + 1 - minObstDistForDiag;
                    DEBUG_SHS(cout<<"obst right-down";);
                }
            //}

            if (pos.equals(newRow,newCol)) {
                DEBUG_SHS(cout<<" equals: pos="<<pos.toString()<<", new:r"<<newRow<<"c"<<newCol<<endl;);
                cont = false;
            } else {
                pos.set(newRow,newCol);
                DEBUG_SHS(cout<<" "<<pos.toString()<<endl;);
            }
        }//for
    }
}

bool AutoPlayer::isThereAnObstacleAt(Pos p, double dr, double dc) const {
    p.add(dr,dc);
    if (_map->isPosInMap(p)) {
        return _map->isObstacle(p);
    } else {
        return false;
    }
}

double** AutoPlayer::getBeliefZoomMatrix(unsigned int& zrows, unsigned int& zcols) {
    zrows = (unsigned int)ceil(_map->rowCount()/_params->beliefMapZoomFactor);
    zcols = (unsigned int)ceil(_map->colCount()/_params->beliefMapZoomFactor);

    double** zbmap = AllocateDynamicArray<double>(zrows,zcols);
    //cout<<"belief zoom matrix:"<<endl;
    //create zoomed matrix
    for(int zr=0; zr<(int)zrows; zr++) {
        for(int zc=0; zc<(int)zcols; zc++) {
            //AG140602: use floor instead of round!!
            int startR = (unsigned int)floor(zr*_params->beliefMapZoomFactor);
            int startC = (unsigned int)floor(zc*_params->beliefMapZoomFactor);

            zbmap[zr][zc] = 0;
            bool isObs = true;
            for(int r=startR; r<startR+_params->beliefMapZoomFactor && r<_map->rowCount(); r++) {
                for(int c=startC; c<startC+_params->beliefMapZoomFactor && c<_map->colCount(); c++) {
                  if (!_map->isObstacle(r,c)) {
                    isObs = false;
                    double b = getBelief(r,c);
                    if (b>0) zbmap[zr][zc] += b;
                  }
                }
            }

            if (isObs) zbmap[zr][zc] = -1;
        }
    }

    return zbmap;
}

double AutoPlayer::getReward() {
    return 0;
}

bool AutoPlayer::getChosenHiderPos(Pos &chosenHiderPos) const {
    if (_hiderPlayer!=NULL) {
        chosenHiderPos = _hiderPlayer->currentPos;
        return true;
    } else {
        return false;
    }
}

SeekerHSParams* AutoPlayer::getParams() const {
    return _params;
}
