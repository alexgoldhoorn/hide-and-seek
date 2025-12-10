#include "autoplayer.h"

#include "hsglobaldata.h"
#include "Utils/generic.h"

#include "exceptions.h"

#ifdef USE_QT
#include "AutoHider/fromlisthider.h"
#endif

#include <sstream>
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace std;


AutoPlayer::AutoPlayer(SeekerHSParams* params) :
    _map(NULL), _params(params), _randomPosDistToBase(0), _lastAction(-1), _numIterationsSkipped(0), _numIterations(0) {
}

AutoPlayer::AutoPlayer(SeekerHSParams* params, GMap* map) :
    _map(map), _params(params), _randomPosDistToBase(0), _lastAction(-1), _numIterationsSkipped(0), _numIterations(0) {
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
        FromListHider fromListHider(_params, _params->seekerPosFile);
        fromListHider.setMap(_map);
        initPos = fromListHider.getInitPos();
        //cout << " READ INIT POS: "<<initPos.toString()<<endl;
#else
        cout << "AutoPlayer::getInitPos: ERROR: using a FromListHider while QT is not used!"<<endl;
#endif
    } else if (_randomPosDistToBase>0) {
        //random pos based on distance from base
        Pos base(_map->getBase());

        assert(base.isSet());

        //AG140106: do a check if it is realistic
        if (base.row()-_randomPosDistToBase<0 && base.col()-_randomPosDistToBase<0 &&
                base.row()+_randomPosDistToBase>=_map->rowCount() && base.col()+_randomPosDistToBase>=_map->colCount()) {
            cout << "WARNING: probably the random distance "<<_randomPosDistToBase<<" to the base "<<base.toString()
                 << " cannot be reached on the "<<_map->rowCount()<<"x"<<_map->colCount()<<" map."<<endl;
        }

        DEBUG_CLIENT(cout<<"Calculating start position at a distance of "<<_randomPosDistToBase<<" to the base."<<endl;);

        //AG130105: try to find a random place
        //first create list of possible positions, use Pos as vector
        vector<Pos> posInitVec;
        if (base.row()-_randomPosDistToBase>=0) {
            Pos p(base.row()-_randomPosDistToBase,0);
            posInitVec.push_back(p);
        }
        if (base.col()-_randomPosDistToBase>=0) {
            Pos p(0,base.col()-_randomPosDistToBase);
            posInitVec.push_back(p);
        }
        if (base.row()+_randomPosDistToBase<_map->rowCount()) {
            Pos p(base.row()+_randomPosDistToBase,0);
            posInitVec.push_back(p);
        }
        if (base.col()+_randomPosDistToBase<_map->colCount()) {
            Pos p(0,base.col()+_randomPosDistToBase);
            posInitVec.push_back(p);
        }

        //cout << "try base="<<base.toString()<<":"<<flush;
        //use the fact that manhatten distance requires one coord to be the distance, other can be random
        bool isOk = false;
        int d = 0;
        do {
            //first decide on which side of the base
            initPos = posInitVec[random(posInitVec.size()-1)];
            //set other coord
            if (initPos.row()>0) {
                //set col
                int minCol = base.col()-_randomPosDistToBase;
                if (minCol<0) minCol=0;
                int maxCol = base.col()+_randomPosDistToBase;
                if (maxCol>=_map->colCount()) maxCol=_map->colCount()-1;
                initPos.set(0, random(minCol,maxCol));
            } else {
                //set row
                int minRow = base.row()-_randomPosDistToBase;
                if (minRow<0) minRow=0;
                int maxRow = base.row()+_randomPosDistToBase;
                if (maxRow>=_map->rowCount()) maxRow=_map->rowCount()-1;
                initPos.set( random(minRow,maxRow), 0);
            }
            //cout <<pos.toString()<<" "<<flush;

            isOk = _map->isPosInMap(initPos);
            if (isOk) isOk = !_map->isObstacle(initPos);

        } while (!isOk);
        //cout << endl;

        //distance
        d = _map->distance(base,initPos);
        //cout<<"[D="<<d<<"]"<<flush;

        //now move randomly until the distance has been reached
        while(d!=_randomPosDistToBase) {
            //choose random action, avoiding staying at the same pose (action 0)
            int a = random(HSGlobalData::NUM_ACTIONS-2)+1;

            //try move
            Pos newPos = _map->tryMove(a,initPos);

            //cout << " "<<ACTION_COUT(a)<<"->"<<newPos.toString()<<flush;

            if (newPos.isSet()) {
                /*int newD*/ d = _map->distance(newPos,base);
                //if (newD>=d) { //ag140105: don't check this because this requires us to go the same direction, and
                // some maps might not reach a certain direction
                //d=newD;
                initPos = newPos;
                //cout<<"[d="<<d<<"]"<<flush;
                //}
            }
        }
        //cout<<endl;

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
                vector<Pos> invisPoints = _map->getInvisiblePoints(_map->getBase(),_params->takeDynObstOcclusionIntoAccountWhenLearning);
                initPos = invisPoints[random(invisPoints.size()-1)];

                //AG140528: add random cell location if continuous
                if (_params->useContinuousPos) {
                    initPos.add(randomDouble(0,1), randomDouble(0,1));
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

bool AutoPlayer::initBelief2(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible, Pos seeker2InitPos, Pos hiderObs2InitPos, double obs1p) {
    throw CException(_HERE_, "initBelief2, for 2 seekers, not implemented");
    return false;
}

int AutoPlayer::getNextAction2(Pos seekerPos, Pos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p, int actionDone) {
    throw CException(_HERE_, "getNextAction2, for 2 seekers, not implemented");
    return -1;
}

Pos AutoPlayer::getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p, std::vector<int> &actions, int actionDone, int n) {
    throw CException(_HERE_, "getNextPosRun2, for 2 seekers, not implemented");
    return Pos();
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
    _map = map;
}

GMap* AutoPlayer::getMap() const {
    return _map;
}

void AutoPlayer::setInitPos(Pos initPos) {
    _initPos = initPos;
}

void AutoPlayer::setRandomPosDistToBase(int d) {
    _randomPosDistToBase = d;
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
    returnPos.add(0.5,0.5);

    if (_map->isPosInMap(returnPos) && !_map->isObstacle(returnPos)) {
        return returnPos;
    }

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
                    returnPos.set(r,c);
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
            returnPos = closestPosVec[random(closestPosVec.size()-1)];
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

bool AutoPlayer::storeMapBeliefAsImage(std::string file, const Pos& seekerPos, const Pos* seeker2Pos, const Pos& hiderPos,
                                       const Pos* hiderObsPos2, int cw) {
#ifndef DO_NOT_WRITE_BELIEF_IMG
    cv::Mat* image = getMapBeliefAsImage(seekerPos, seeker2Pos, hiderPos, hiderObsPos2, cw);

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
        x = (int)floor(c+origCellSize/2);
        y = (int)floor(r+origCellSize/2);
    }
    cv::circle(*image, cv::Point(x,y), (int)(radiusPart*origCellSize/2), color, 1);
}

cv::Mat* AutoPlayer::getMapBeliefAsImage(const Pos& seekerPos, const Pos* seeker2Pos, const Pos& hiderPos, const Pos* hiderObsPos2, int cw) {
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
            double b = belief[r][c];

            /*if (b!=getBelief(r,c))
                cout << "[zb="<<b<<";b="<<getBelief(r,c)<<"] ";
            else*/
                //cout << b << " ";

            if (b<0 /*_map->isObstacle(r,c)*/) {
                //obstacle is black
                color = cv::Scalar(0,0,0);
            /*} else if (_params->gameType==HSGlobalData::GAME_HIDE_AND_SEEK && _map->isBase(r,c)) {
                //base is green
                color = cv::Scalar(0,255,0);*/
            } else {
                //free field is white, and if probability 1 of hider then red

                if (b==0) {
                    bool hasSeeker = false;
                    /*int startR = (int)(r*_params->beliefMapZoomFactor);
                    int startC = (int)(c*_params->beliefMapZoomFactor);
                    for(int r1=startR; !hasSeeker && r1<startR+_params->beliefMapZoomFactor && r1<_map->rowCount(); r1++) {
                        for(int c1=startC; !hasSeeker && c1<startC+_params->beliefMapZoomFactor && c1<_map->colCount(); c1++) {
                            if (seekerPos.equalsInt(r1,c1))
                                hasSeeker = true;
                        }
                    }*/

                    if (hasSeeker /*seekerPos.equalsInt(r,c)*/) {
                        color = cv::Scalar(255,0,0);
                    } else {
                        color = cv::Scalar(255,200,200);
                    }
                } else {
                    int v = (int) round( (1-b)*255 );
                    color = cv::Scalar(v,v,255);
                }
            }

            /*if (seekerPos.equalsInt(r,c)) {
                color[0]=255;
            }*/

            cv::rectangle(*image, cv::Point(c*cw,r*cw),cv::Point((c+1)*cw,(r+1)*cw),color,CV_FILLED);
        } //cout << endl;
    }

    if (belief!=NULL)
        FreeDynamicArray<double>(belief,zrows);

    //obstacles
    color = cv::Scalar(0,0,0);
    double origCellSize = 1.0*imgRows/_map->rowCount();
    int origCellSizeInt = (int)(origCellSize);

    for (int i = 0; i <_map->numObstacles(); i++) {
        Pos p = _map->getObstacle(i);
        //int r = (int)round(p.row()/_params->beliefMapZoomFactor);
        //int c = (int)round(p.col()/_params->beliefMapZoomFactor);
        //cv::rectangle(*image, cv::Point(c*cw,r*cw),cv::Point((c+1)*cw,(r+1)*cw),color,1/*CV_FILLED*/);
        int y = (int)round(p.rowDouble()*origCellSize);
        int x = (int)round(p.colDouble()*origCellSize);
        cv::rectangle(*image, cv::Point(x,y),cv::Point(x+origCellSizeInt,y+origCellSizeInt),color,1/*CV_FILLED*/);
    }
    //dabo
    /*double r = seekerPos.rowDouble() * origCellSize; // /_params->beliefMapZoomFactor;
    double c = seekerPos.colDouble() * origCellSize; // /_params->beliefMapZoomFactor;
    int x;
    int y;
    if (_params->useContinuousPos) {
        x = (int)round(c);
        y = (int)round(r);
    } else {
        x = (int)round(c+origCellSize/2);
        y = (int)round(r+origCellSize/2);
    }
    cv::circle(*image, cv::Point(x,y), (int)(origCellSize/2) , cv::Scalar(255,0,0), 1);
    */
    //paint seeker
    paintCircleOnImage(image, seekerPos, cv::Scalar(255,0,0), origCellSize);
    //paint seeker 2
    if (seeker2Pos!=NULL && seeker2Pos->isSet()) {
        paintCircleOnImage(image, *seeker2Pos, cv::Scalar(255,128,128), origCellSize,0.7);
    }
    //paint hider
    if (hiderPos.isSet()) {
        paintCircleOnImage(image, hiderPos, cv::Scalar(0,0,128), origCellSize);
    }
    if (hiderObsPos2!=NULL && hiderObsPos2->isSet()) {
        paintCircleOnImage(image, *hiderObsPos2, cv::Scalar(0,69,255), origCellSize,0.7);
    }

    return image;
}
#endif


double AutoPlayer::getNextDirection(Pos seekerPos, Pos hiderPos, bool opponentVisible, bool &haltAction) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::getNextDirection: not implemented, should be implemented by subclass, or call getNextAction()");
    return -1;
}

int AutoPlayer::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::getNextAction: not implemented, should be implemented by subclass, or call getNextDirection()");
    return -1;
}

std::vector<int> AutoPlayer::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone, int n) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::getNextMultipleActions: not implemented, should be implemented by subclass, or call getNextDirection()");
    return std::vector<int>();
}

bool AutoPlayer::getNextRobotPoses2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, std::vector<int>& actions,
                                    std::vector<Pos>& goalPosesVec, int actionDone, int n, std::vector<double> *goalPosesBelVec) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::getNextRobotPoses: not implemented, should be implemented by subclass");    
    return false;
}


bool AutoPlayer::useGetAction() {
    return true;
}

Pos AutoPlayer::getNextPos(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    _lastSeekerPos = seekerPos;
    _lastHiderPos = hiderPos;

    Pos nextPos = getNextPosRun(seekerPos, hiderPos, opponentVisible, actions, actionDone, n);
    _numIterations++;

    return nextPos;
}

Pos AutoPlayer::getNextPos2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p,
                            std::vector<int> &actions, int actionDone, int n) {
    _lastSeekerPos = seekerPos;
    _lastHiderPos = hiderPos;
    //TODO: lastseeker2 pos

    Pos nextPos = getNextPosRun2(seekerPos, hiderPos, opponentVisible, seeker2Pos, hiderObs2Pos, obs1p, actions, actionDone, n);
    _numIterations++;

    return nextPos;
}

Pos AutoPlayer::selectRobotPos2(Pos *otherSeekerPos1, Pos *otherSeekerPos2, double otherSeekerPos1B, double otherSeekerPos2B, int n,
                                Pos* chosenPos) {
    //not implemented
    throw CException(_HERE_, "AutoPlayer::selectRobotPos2: not implemented, should be implemented by subclass");
    return Pos();
}

Pos AutoPlayer::getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, vector<int> &actions, int actionDone, int n) {
    assert(n>=1);

    //AG140527: set pos based on if seeker/hider
    //current pos
    Pos curPos( isSeeker() ? seekerPos : hiderPos );

    //pos to return
    Pos nextPos;

    //action output
    actions.clear();

    if (useGetAction()) {
        if (n==1) {
            //get only one action
            _lastAction = getNextAction(seekerPos, hiderPos, opponentVisible, actionDone);
            //next pos
            nextPos = _map->tryMove(_lastAction, curPos);

            if (!nextPos.isSet()) {
                //if not possible, check how to fix
                DEBUG_SHS(cout<<"WARNING  Action "<<ACTION_COUT(_lastAction)<<" not possible, trying from center"<<endl;);
                if (_params->useContinuousPos) {
                    curPos.convertValuesToInt();
                    curPos.add(0.5,0.5);
                    nextPos = _map->tryMove(_lastAction,curPos);
                }
                if (!nextPos.isSet()) {
                    nextPos = curPos;
                    DEBUG_SHS(cout<<"ERROR Action i"<<ACTION_COUT(_lastAction)<<" not possible"<<endl;);
                }
                DEBUG_SHS(cout<<"    ";);
            }

            //add action
            actions.push_back(_lastAction);

        } else {
            //get several actions
            actions = getNextMultipleActions(seekerPos, hiderPos, opponentVisible, actionDone, n);

            //now execute the actions to get the end position
            for(size_t i=0; i<actions.size(); i++) {
                nextPos = _map->tryMove(actions[i],curPos);

                if (!nextPos.isSet()) {
                    //if not possible, check how to fix
                    DEBUG_SHS(cout<<"WARNING  Action "<<ACTION_COUT(actions[i])<<" not possible, trying from center"<<endl;);
                    if (_params->useContinuousPos) {
                        curPos.convertValuesToInt();
                        curPos.add(0.5,0.5);
                        nextPos = _map->tryMove(actions[i],curPos);
                    }
                    if (!nextPos.isSet()) {
                        nextPos = curPos;
                        DEBUG_SHS(cout<<"ERROR Action i"<<ACTION_COUT(actions[i])<<" not possible"<<endl;);
                    }
                    DEBUG_SHS(cout<<"    ";);
                }
            }

            //last action
            if (actions.size()>0) {
                _lastAction = actions[0];
            } else {
                _lastAction = -1;
            }

        }
    } else { //use direction
        bool haltAction = false;
        //get the action with direction
        double dir = getNextDirection(seekerPos, hiderPos, opponentVisible, haltAction);

        if (haltAction) {
            //action is halt
            nextPos = curPos;

            _lastAction = HSGlobalData::ACT_H;
        } else {
            //distance
            double dist = n*_params->seekerStepDistance;
            unsigned int tries = 0;
            do {
                //try to move
                nextPos = _map->tryMoveDir(dir, curPos, dist, true);
                tries++;
                dist /= 2; //lower distance if moving is not possible
            } while (tries<2 && !nextPos.isSet());

            if (!nextPos.isSet()) {
                cout << "WARNING: was not able to move in direction "<<(180*dir/M_PI)<<"º, last tried distance is "
                     <<dist<<" cells, now using current position."<<endl;

                _lastAction = -1;
            } else {
                _lastAction = deduceAction(dir);
            }
        }
    }


    _lastSeekerPos = seekerPos;
    _lastHiderPos = hiderPos;

    return nextPos;
}

Pos AutoPlayer::getNextPosWithFilter(Pos seekerPos, vector<IDPos> hiderPosVector, vector<int> &actions, int actionDone, int n, bool& dontExecuteIteration) {
    //choose hider pos
    Pos seekerPosOut;
    IDPos chosenHiderPos;

    checkAndFilterPoses(seekerPos, hiderPosVector, seekerPosOut, chosenHiderPos, dontExecuteIteration);

    if (dontExecuteIteration) {
        return seekerPos;
    } else {
        return getNextPos(seekerPosOut, chosenHiderPos, chosenHiderPos.isSet(), actions, actionDone, n);
    }
}

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

bool AutoPlayer::initBeliefWithFilter(GMap *gmap, Pos seekerPos, std::vector<IDPos> hiderPosVector) {
    _map = gmap;
    _numIterationsSkipped = 0;

    //choose hider pos
    IDPos chosenHiderPos;
    Pos seekerPosOut;
    bool dontExecIt;
    checkAndFilterPoses(seekerPos, hiderPosVector, seekerPosOut, chosenHiderPos, dontExecIt);

    _lastSeekerPos = seekerPos;
    _lastHiderPos = chosenHiderPos;

    return initBelief(gmap, seekerPosOut, chosenHiderPos, chosenHiderPos.isSet());
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

int AutoPlayer::deduceAction(const Pos &seekerPos) {
    return deduceAction(_lastSeekerPos, seekerPos);
}

int AutoPlayer::deduceAction(const Pos& p1, const Pos& p2) {
    if (!p1.isSet() || !p2.isSet()) return -1;

    int a = -1;
    //distance
    DEBUG_CLIENT(cout<<"Deduce action "<<p1.toString()<<"->"<<p2.toString(););

    double dist = _map->distanceEuc(p1,p2);

    DEBUG_CLIENT(cout <<"[d="<<dist<<"]:";); //<<endl;

    if (dist > _params->seekerStepDistance*_params->seekerStepDistancePartForHaltActionDeduction) {

        //deduce action base on angle
        a = deduceAction(_map->getDirection(p1,p2));
    } else {
        a = HSGlobalData::ACT_H;
    }

    DEBUG_CLIENT(cout<<ACTION_COUT(a)<<endl;);

    return a;
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
//cout << "AutoPlayer::getBeliefScore from hiderpos="<<hiderPos.toString()<<endl;//TMP
    for(int r=0; r<_map->rowCount(); r++) {
        for(int c=0; c<_map->colCount(); c++) {
            if (!_map->isObstacle(r,c)) {
                double b = getBelief(r,c);
                if (b>0) {
                    score += b*_map->distance(r,c,hiderPos.row(),hiderPos.col());
                    //cout <<"[b"<<b<<";d"<<_map->distance(r,c,hiderPos.row(),hiderPos.col())<<"]";//TMP
                } //else cout<<"[000]";//TMP
            } //else //cout <<  "[XXX]";//TMP
        } //cout <<endl;//TMP
    }
//cout<<"-->"<<score<<endl;
    return score;
}

int AutoPlayer::getLastAction() {
    return _lastAction;
}


IDPos AutoPlayer::chooseHiderPos(const Pos& seekerPos, const std::vector<IDPos>& hiderPosVector, bool checkPrev, bool& dontExecuteIteration) {
    assert(seekerPos.isSet() && _map->isPosInMap(seekerPos) && !_map->isObstacle(seekerPos));

    IDPos chosenHiderPos;
    dontExecuteIteration = false;

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
            if (_params->useDeducedAction && _lastSeekerPos.isSet()) {
                lastAction = deduceAction(_lastSeekerPos,seekerPos);
            } else {
                lastAction = -1;
            }
            /*cout <<", ded.act=";
            if (lastAction<0) cout <<"none"; else cout << ACTION_COUT(lastAction);*/
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
                    if (hiderPos.isSet() && !_map->isVisible(seekerPos,hiderPos,false)) {
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
                if (hiderPos.isSet() && _lastHiderPos.isSet()) {
                    //the closer, the higher a score, BUT we need to prevent the score from
                    //growing exponantially, therefore round them (now 0.5)
                    //AG TODO: maybe an exponential function instead of continuous
                    double d = _map->distanceEuc(_lastHiderPos,hiderPos);
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


                }  else if (obsScore==0 /*_numActions==0 || !_autoPlayer->canScoreObservations()*/) {
                    //score = (_numActions==0 ? 1 : _params->filterMinScore);
                    score = _params->filterMinScore;

                    //AG140416: first action, put a score in order to be able to compare with base score (otherwise all 0)
                    //DEBUG_SHS(cout<<(_numActions==0?", 1st action score=":", cannot score obs->score=")<<score<<flush;);
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
                    //if (visib || !visib && !visibObsFound) {
                    //AG140310: max score AND either visible OR NOT visible AND no other visible observations found
                    if (score > maxS) {
                        maxScoreObsVec.clear();
                        maxS = score;
                    }

                    maxScoreObsVec.push_back(i);
                    /*} else {
                        DEBUG_SHS(cout << " (not used since other visible obs found)";);
                    }*/
                }
                DEBUG_SHS(cout << endl;);
            } // for all poses

            //index
            int i = -1;

            if (maxScoreObsVec.size()==1) {
                i = maxScoreObsVec[0];
            } else if (maxScoreObsVec.size()>1) {
                //randomly choose a best one
                i = maxScoreObsVec[random(maxScoreObsVec.size()-1)];
            } // else: empty

            if (i>=0) {
                chosenHiderPos = hiderPosVector2[i];
            }
            /*if (chosenHiderPosIndex!=NULL) {
                *chosenHiderPosIndex = i;
            }*/


            break;
        }

        case HSGlobalData::FILTER_SCORE_USE_TAG_ONLY:
        case HSGlobalData::FILTER_SCORE_USE_ID: {
            DEBUG_SHS(
                cout << "AutoPlayer::chooseHiderPos(use track id): checking "<<hiderPosVector.size()<< " hider poses (seeker = "
                    <<seekerPos.toString()<<"), ";
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
                                _map->isVisible(seekerPos,hiderPos,true)) { //AG140526: also use dyn.obst. to occlude
                            //DEBUG_SHS(cout<<"] simulating NOT visible, ignoring"<<endl;);
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
            if (_params->useDeducedAction && _lastSeekerPos.isSet()) {
                lastAction = deduceAction(_lastSeekerPos,seekerPos);
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

        if (dontExecuteIteration && chosenHiderPos.isSet() && _lastHiderPos.isSet()) {
            double d = _map->distanceEuc(_lastHiderPos,chosenHiderPos);
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
    } else if (checkPrev && _map->distance(_lastSeekerPos,seekerNextPos)>_params->hiderStepDistance+_params->contNextSeekerStateStdDev*2) {
        //distance is higher than 1
        DEBUG_SHS(cout << "AutoPlayer::checkValidNextSeekerPos: ERROR distance to to previous position higher than step+2*std ("<<
                  _params->hiderStepDistance+_params->contNextSeekerStateStdDev*2<<")"<<endl;);
        valid = false;
    }

#ifdef DEBUG_SHS_ON
    if (!valid) {
        cout << "  NOT VALID Next seeker pos "<<seekerNextPos.toString();
        if (checkPrev)
            cout << " (current pos:"<<_lastSeekerPos.toString()<<")";
        cout<<endl;
    }
#endif

    //ok
    return valid;
}

bool AutoPlayer::checkValidNextHiderPos(const IDPos& hiderNextPos, const Pos& seekerNextPos, bool checkPrev) {
    bool lastVisib = _lastHiderPos.isSet();
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
        if (lastVisib) {
            lastVisib = _map->isVisible(_lastSeekerPos,_lastHiderPos,false);
            DEBUG_SHS(if (!lastVisib) cout << " (current visible but according to raytrace not) "<<flush;);
        }
        if (visibNext) {
            visibNext = _map->isVisible(seekerNextPos,hiderNextPos,false);
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
                for(int r=_lastHiderPos.row()-1; !valid && r<=_lastHiderPos.row()+1; r++) {
                    for(int c=_lastHiderPos.col()-1; !valid && c<=_lastHiderPos.col()+1; c++) {
                        if (_map->isPosInMap(r,c) && !_map->isObstacle(r,c)) {
                            if (!_map->isVisible(seekerNextPos.row(), seekerNextPos.col(), r, c, false)) {
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


IDPos AutoPlayer::getLastHiderPos() const {
    return _lastHiderPos;
}

Pos AutoPlayer::getLastSeekerPos() const {
    return _lastSeekerPos;
}

Pos AutoPlayer::getNextPosAsStep(const Pos& seekerPos, const Pos& goalPos, int n, bool stayAtMinWinDistFromgoal) {
    DEBUG_SHS(cout <<"AutoPlayer::getNextPosAsStep: from "<<seekerPos.toString()<<" to "<<goalPos.toString()<<": "<<flush;);

    Pos nextPos;
    if (_params->onlySendStepGoals) {

        if (seekerPos==goalPos ||
                (_map->distanceEuc(seekerPos,goalPos) <= _params->seekerStepDistance) || //AG15022: if goal within step dist, go there
                (stayAtMinWinDistFromgoal && _map->distanceEuc(seekerPos,goalPos) <= _params->winDist) ) {
            DEBUG_SHS(cout <<"returning same pos, ";
                if (seekerPos==goalPos)
                    cout << "poses are same";
                else
                    cout<< "close to goal"
            );
            nextPos = goalPos; //AG150223: if seeker closer than a step to goal, then that's the next pos
        } else {
            assert(n==1); // should be 1 for simulation, because pathplanning gives 1 step ahead
            //get next step towards goal following 'pathplanner'
            nextPos = _map->getNextStep(seekerPos, goalPos);

            //TODO SIMULATE 1 step
           // nextPos = _map->tryMoveDirStep(dir, seekerPos, dist, _params->seekerStepDistance, _params->doVisibCheckBeforeMove);

            //added because pathplanner gives discrete steps
            if (_params->useContinuousPos && !nextPos.hasDouble()) nextPos.add(0.5,0.5);

            DEBUG_SHS(cout<<"next step: "<<nextPos.toString());

            if (_params->useContinuousPos) {
                //AG140526: we have to make it the center of the cell, since the pathplanner gives discrete positions
                if (!nextPos.hasDouble()) nextPos.add(0.5,0.5);
                //get direction
                double dir = _map->getDirection(seekerPos,nextPos);

                //move in the direction only distance 1 (also in diagonal)
                nextPos = _map->tryMoveDirStep(dir, seekerPos, n, _params->seekerStepDistance, _params->doVisibCheckBeforeMove);

                DEBUG_SHS(cout<<" make sure of dist. 1: dir="<<dir);
            }
        }
    } else {
        double dist = n*_params->seekerStepDistance;
        double dir = _map->getDirection(seekerPos,goalPos);

        DEBUG_SHS(cout<<" dist="<<dist<<" dir="<<dir;);

        if (stayAtMinWinDistFromgoal) {
            double dsh = _map->distanceEuc(seekerPos,goalPos);
            if (dist>dsh-_params->winDist) {
                //if the goal is too close, put it from a certain distance from the hider
                dist = dsh-_params->winDist;
                DEBUG_SHS(cout<<" reduced dist="<<dist;);
            }
        }

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
                cout<<" equals: pos="<<pos.toString()<<", new:r"<<newRow<<"c"<<newCol<<endl;
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
            //cout << zbmap[zr][zc]<<" ";
        } //cout <<endl;
    }

    return zbmap;
}

double AutoPlayer::getReward() {
    return 0;
}


