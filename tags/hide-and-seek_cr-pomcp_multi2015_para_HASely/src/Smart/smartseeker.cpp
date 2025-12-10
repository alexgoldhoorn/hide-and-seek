#include "Smart/smartseeker.h"

#include <iostream>
#include <cassert>

#include "Base/hsglobaldata.h"
#include "Utils/generic.h"

#include "exceptions.h"

using namespace std;

SmartSeeker::SmartSeeker(SeekerHSParams* params) : AutoPlayer(params)
{
    initRandomizer();
}

SmartSeeker::~SmartSeeker() {
}

bool SmartSeeker::initBeliefRun() { //(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible) {
    //_map = gmap;
    /*_seekerPlayer.setMap(_map);
    _hiderPlayer.setMap(_map);*/
    assert(_hiderPlayer!=NULL);

    _maxScore = _map->rowCount()*_map->colCount();
    _base = _map->getBase();

    return true;
}

Pos SmartSeeker::getNextPosRun(int actionDone, int *newAction) {
//int SmartSeeker::getNextAction(Pos seekerPos, Pos hiderPos, bool hiderVisible, int actionDone) {
    vector<int> maxAVector;
    double maxS = -1e99;

    //AG150605: get pos from playerinfo
    Pos seekerPos = playerInfo.currentPos;
    Pos hiderPos = playerInfo.hiderObsPosWNoise; //_hiderPlayer->currentPos;

    DEBUG_HS(cout<<"SmartSeeker::getNextPosRun: Pos s="<<seekerPos.toString()<<" h="<<hiderPos.toString()<<" visib="<<hiderPos.isSet()<<endl;);

    //try all actions and get highest score
    for(int a=0;a<HSGlobalData::NUM_ACTIONS;a++) {
        double s = 0;
        bool actOK = scoreForAction(seekerPos, hiderPos, a, s); //seekerPos.row(),seekerPos.col(),hiderPos.row(),hiderPos.col(),a,hiderVisible,s);
        if (actOK) {
            if (s>=maxS) {
                if (s>maxS) {
                    maxS = s;
                    maxAVector.clear();
                }
                maxAVector.push_back(a);
            }
            DEBUG_HS(cout <<" a="<<ACTION_COUT(a)<<"-> s="<<s<<endl;);
        }
    }

    //choose action
    int maxA = maxAVector[0]; //random(maxAVector.size()-1)]; //random should be used but causes really rough movements in real robot, therefore best always same action

    if (maxAVector.size()>1) {
        //AG150605: choose random action
        maxA = maxAVector[random(maxAVector.size()-1)];
    }

    //AG150605: get next pos
    playerInfo.nextPos = _map->tryMove(maxA, seekerPos);
    assert(playerInfo.nextPos.isSet());

    DEBUG_HS(cout<<"Best action (#"<<maxAVector.size()<<"): "<<ACTION_COUT(maxA)<<" -> "<<playerInfo.nextPos.toString()<<endl;);

    return playerInfo.nextPos;
}

double SmartSeeker::scoreForState(const Pos &seekerPos, const Pos &hiderPos, const Pos &prevSeekerPos) {
    double s = 0;
    int n=0;

    double maxS = HSGlobalData::INFTY_NEG_DBL;
    double minS = HSGlobalData::INFTY_POS_DBL;

    //try all seeker actions
    for(int ha=0;ha<HSGlobalData::NUM_ACTIONS;ha++) {
        //_hiderPlayer.setCurPos(hr,hc);
        Pos newHPos = _map->tryMove(ha,hiderPos);

        if (newHPos.isSet()) {
            //can move
            //Pos newHPos = _hiderPlayer.getCurPos();

            //calc score and update sum,min,max
            double as = 0;

            //AG131106: check for cross
            if (newHPos==prevSeekerPos && hiderPos==seekerPos) { //(newHPos.row()==srp && newHPos.col()==scp && hr==sr && hc==sc) {
                as = _maxScore;
            } else {
                as = score(seekerPos, newHPos); //(sr,sc,newHPos.row(),newHPos.col()); //we say visib because we take avg over all non-avg
            }

            s += as;
            if (as>maxS) maxS=as;
            if (as<minS) minS=as;

            n++;
        }
    }

    switch(_params->smartSeekerScoreActionType) {
    case SCORE_AVG:
        s = s/n;
        break;
    case SCORE_MIN:
        s=minS;
        break;
    case SCORE_MAX:
        s=maxS;
        break;
    default:
        throw CException(_HERE_, "unknown score type (seeker action)");
        break;
    }

    return s;
}

bool SmartSeeker::scoreForAction(const Pos &seekerPos, const Pos &hiderPos, int a, double &score) {
                              //(int sr, int sc, int hr, int hc, int a, bool visib, double& s) {

    //_seekerPlayer.setCurPos(sr,sc);
    Pos newSPos = _map->tryMove(a, seekerPos);

    //AG150624: we should initialize score
    score = 0;

    if (newSPos.isSet()) { //_seekerPlayer.move(a)) {
        //Pos newSPos = _seekerPlayer.getCurPos();

        if (hiderPos.isSet()) {
            //visible, so score can be directly calculated
            score = scoreForState(newSPos, hiderPos, seekerPos); //(newSPos.row(),newSPos.col(),hr,hc,sr,sc);
        } else {
            //not visible -> go through all possible location and get: avg/min/max based on scoreType
            //use NOW invisible cells since this is our knowledge
            vector<Pos> invisPointsVector = _map->getInvisiblePoints(seekerPos, _params->takeDynObstOcclusionIntoAccountWhenLearning);

            if (invisPointsVector.size()>0) {
                vector<Pos>::iterator it;
                double maxS =-1e99;
                double minS = 1e99;

                //ag130723
                //if we have a limit of cells to calculate and we passed the limit then
                //remove the items randomly until we arrived to the amount
                if (_params->smartSeekerMaxCalcsWhenHidden>0 && invisPointsVector.size()>_params->smartSeekerMaxCalcsWhenHidden) {
                    //cout << " choosing "<<_params->smartSeekerMaxCalcsWhenHidden<<" cells from "<<invisPointsVector.size()<<endl;
                    //passed the limit, remove until reached
                    while (invisPointsVector.size() > _params->smartSeekerMaxCalcsWhenHidden) {
                        int removeI = random(invisPointsVector.size()-1);
                        invisPointsVector.erase(invisPointsVector.begin()+removeI);
                    }
                    //cout << "cells chosen..now calc"<<endl;
                }

                //calc score for each invisible state
                for(it=invisPointsVector.begin();it!=invisPointsVector.end();it++) {

                    double as = scoreForState(newSPos, *it, seekerPos); //(newSPos.row(),newSPos.col(),it->row(),it->col(),sr,sc);
                    score += as;
                    if (as>maxS) maxS=as;
                    if (as<minS) minS=as;
                }

                switch(_params->smartSeekerScoreHiddenType) {
                case SCORE_AVG:
                    score = score/invisPointsVector.size();
                    break;
                case SCORE_MIN:
                    score=minS;
                    break;
                case SCORE_MAX:
                    score=maxS;
                    break;
                default:
                    //should not occur!!
                    throw CException(_HERE_,"unknown score type");
                    break;
                }

            } else {
                cout << "ERROR: no invisible states found, but said to be invisible"<<endl;
                assert(false);
            }
        }

        return true;

    } else {
        //can't move with a
        return false;
    }
}

double SmartSeeker::score(const Pos &seekerPos, const Pos &hiderPos) { //(int sr, int sc, int hr, int hc) {
    /*assert(_map->isPosInMap(sr,sc));
    assert(_map->isPosInMap(hr,hc));*/

    //distance seeker-hider
    double dsh = _map->distance(seekerPos, hiderPos); //(sr,sc,hr,hc);
    //for find-and-follow the score is higher when the distance is lower
    double s = -dsh;

    if (_params->gameType == HSGlobalData::GAME_HIDE_AND_SEEK) {
        //for hide-and-seek the triangle rule score is used
        double dhb = _map->distance(hiderPos, _base); //(hr,hc,_base.row(),_base.col());
        double dsb = _map->distance(seekerPos, _base); //(sr,sc,_base.row(),_base.col());

        if (dhb>=dsb) {
            s += _maxScore;
        }
    }

    return s;
}

#ifdef OLD_CODE
bool SmartSeeker::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible) {
    _map = gmap;
    _seekerPlayer.setMap(gmap);
    _hiderPlayer.setMap(gmap);
    _maxScore = _map->rowCount()*_map->colCount();
    _base = _map->getBase();

    return true;
}

int SmartSeeker::getNextAction(Pos seekerPos, Pos hiderPos, bool hiderVisible, int actionDone) {
    vector<int> maxAVector;
    double maxS = -1e99;

    DEBUG_HS(cout<<"Pos: s="<<seekerPos.toString()<<" h="<<hiderPos.toString()<<" visib="<<hiderVisible<<endl;);

    //try all actions and get highest score
    for(int a=0;a<HSGlobalData::NUM_ACTIONS;a++) {
        double s = 0;
        bool actOK = scoreForAction(seekerPos.row(),seekerPos.col(),hiderPos.row(),hiderPos.col(),a,hiderVisible,s);
        if (actOK) {
            if (s>=maxS) {
                if (s>maxS) {
                    maxS = s;
                    maxAVector.clear();
                }
                maxAVector.push_back(a);
            }
            DEBUG_HS(cout <<" a="<<ACTION_COUT(a)<<"-> s="<<s<<endl;);
        }
    }

    int maxA = maxAVector[0]; //random(maxAVector.size()-1)]; //random should be used but causes really rough movements in real robot, therefore best always same action

    DEBUG_HS(cout<<"Best action (#"<<maxAVector.size()<<"): "<<maxA<<endl;);

    return maxA;
}

vector<int> SmartSeeker::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone, int n) {
    vector<int> actions;
    Pos sPos(seekerPos);

    //get next actions
    for(int i=0; i<n; i++) {
        //get action
        int act = getNextAction(sPos, hiderPos, opponentVisible);
        //add action
        actions.push_back(act);

        //move seeker
        Pos newPos = /*_seekerPlayer*/ _map->tryMove(act,sPos);
        if (newPos.isSet()) {
            sPos = newPos;
            //else: couldn't move, so stay same pos
        }

        //NOTE: not moving hider..
    }

    return actions;
}

double SmartSeeker::scoreForState(int sr, int sc, int hr, int hc, int srp, int scp) {
    double s = 0;
    int n=0;

    double maxS = HSGlobalData::INFTY_NEG_DBL;
    double minS = HSGlobalData::INFTY_POS_DBL;

    //try all seeker actions
    for(int ha=0;ha<HSGlobalData::NUM_ACTIONS;ha++) {
        _hiderPlayer.setCurPos(hr,hc);
        if (_hiderPlayer.move(ha)) {
            //can move
            Pos newHPos = _hiderPlayer.getCurPos();

            //calc score and update sum,min,max
            double as = 0;

            //AG131106: check for cross
            if (newHPos.row()==srp && newHPos.col()==scp && hr==sr && hc==sc) {
                as = _maxScore;
            } else {
                as = score(sr,sc,newHPos.row(),newHPos.col()); //we say visib because we take avg over all non-avg
            }

            s += as;
            if (as>maxS) maxS=as;
            if (as<minS) minS=as;

            n++;
        }
    }

    switch(_params->smartSeekerScoreActionType) {
    case SCORE_AVG:
        s = s/n;
        break;
    case SCORE_MIN:
        s=minS;
        break;
    case SCORE_MAX:
        s=maxS;
        break;
    default:
        throw CException(_HERE_, "unknown score type (seeker action)");
        break;
    }

    return s;
}

bool SmartSeeker::scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double& s) {
    _seekerPlayer.setCurPos(sr,sc);
    if (_seekerPlayer.move(a)) {
        Pos newSPos = _seekerPlayer.getCurPos();

        if (visib) {
            //visible, so score can be directly calculated
            s = scoreForState(newSPos.row(),newSPos.col(),hr,hc,sr,sc);
        } else {
            //not visible -> go through all possible location and get: avg/min/max based on scoreType
            //use NOW invisible cells since this is our knowledge
            vector<Pos> invisPointsVector = _map->getInvisiblePoints(sr,sc,_params->takeDynObstOcclusionIntoAccountWhenLearning);

            if (invisPointsVector.size()>0) {
                vector<Pos>::iterator it;
                double maxS =-1e99;
                double minS = 1e99;

                //ag130723
                //if we have a limit of cells to calculate and we passed the limit then
                //remove the items randomly until we arrived to the amount
                if (_params->smartSeekerMaxCalcsWhenHidden>0 && invisPointsVector.size()>_params->smartSeekerMaxCalcsWhenHidden) {
                    cout << " choosing "<<_params->smartSeekerMaxCalcsWhenHidden<<" cells from "<<invisPointsVector.size()<<endl;
                    //passed the limit, remove until reached
                    while (invisPointsVector.size() > _params->smartSeekerMaxCalcsWhenHidden) {
                        int removeI = random(invisPointsVector.size()-1);
                        invisPointsVector.erase(invisPointsVector.begin()+removeI);
                    }
                    cout << "cells chosen..now calc"<<endl;
                }

                //calc score for each invisible state
                for(it=invisPointsVector.begin();it!=invisPointsVector.end();it++) {
                    //AG131106: FOUND ERROR: sr,sc was used instead of the newSPos !!
                    ///double as = scoreForState(sr,sc,it->row,it->col);
                    double as = scoreForState(newSPos.row(),newSPos.col(),it->row(),it->col(),sr,sc);
                    s += as;
                    if (as>maxS) maxS=as;
                    if (as<minS) minS=as;
                }

                switch(_params->smartSeekerScoreHiddenType) {
                case SCORE_AVG:
                    s = s/invisPointsVector.size();
                    break;
                case SCORE_MIN:
                    s=minS;
                    break;
                case SCORE_MAX:
                    s=maxS;
                    break;
                default:
                    //should not occur!!
                    throw CException(_HERE_,"unknown score type");
                    break;
                }

            } else {
                cout << "ERROR: no invisible states found, but said to be invisible"<<endl;
                assert(false);
            }
        }

        return true;

    } else {
        //can't move with a
        return false;
    }
}

double SmartSeeker::score(int sr, int sc, int hr, int hc) {
    assert(_map->isPosInMap(sr,sc));
    assert(_map->isPosInMap(hr,hc));

    //distance seeker-hider
    double dsh = _map->distance(sr,sc,hr,hc);
    //for find-and-follow the score is higher when the distance is lower
    double s = -dsh;

    if (_params->gameType == HSGlobalData::GAME_HIDE_AND_SEEK) {
        //for hide-and-seek the triangle rule score is used
        double dhb = _map->distance(hr,hc,_base.row(),_base.col());
        double dsb = _map->distance(sr,sc,_base.row(),_base.col());

        if (dhb>=dsb) {
            s += _maxScore;
        }
    }

    return s;
}
#endif

double SmartSeeker::getBelief(int r, int c) {
    return 0; //not to be used
}


bool SmartSeeker::tracksBelief() const {
    return false;
}


bool SmartSeeker::isSeeker() const {
    return true;
}

std::string SmartSeeker::getName() const {
    return "SmartSeeker";
}
