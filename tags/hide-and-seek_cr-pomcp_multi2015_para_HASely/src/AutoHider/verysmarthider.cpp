#include "AutoHider/verysmarthider.h"

#include <iostream>
#include <cassert>

#include "Utils/generic.h"
#include "Base/hsglobaldata.h"

#include "exceptions.h"

using namespace std;

VerySmartHider::VerySmartHider(SeekerHSParams* params, bool allKnowing, char scoreType) : SmartHider(params, allKnowing)
{
    _scoreType = scoreType;

    //_name.clear();
}

VerySmartHider::~VerySmartHider() {
}

/*
int VerySmartHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible) {
    //DEBUG_HS(cout << " takenewaction: "<<a<<endl;);

    _hiderPlayer.setCurPos(hiderPos);
    int a = -1;

    do {
        a = rand() % HSGlobalData::NUM_ACTIONS;
    } while (!_hiderPlayer.MovePlayer(a));

    return a;
}
*/


//more inteligent version: take all possible seeker moves into account
double VerySmartHider::scoreForState(const Pos &seekerPos, const Pos &hiderPos, const Pos &hiderPrevPos) { //(int sr, int sc, int hr, int hc, int hrp, int hcp) {
    assert(seekerPos.isSet());
    /*assert(hiderPos.isSet());*/

    double s = 0;
    int n=0;
    //assert(_seekerPlayer1!=NULL);

    //try all seeker actions
    for(int sa=0; sa<HSGlobalData::NUM_ACTIONS; sa++) {
        //try to move
        Pos newSPos = _map->tryMove(sa, seekerPos); // _seekerPlayer1->currentPos);
        if (newSPos.isSet()) {
            //can move
            s += score(newSPos, hiderPos); //newSPos.row(), newSPos.col(),hr,hc,true); //we say visib because we take avg over all non-avg
            n++;
        }
    }

    s = s/n;

    return s;
}
/*double VerySmartHider::scoreForState(int sr, int sc, int hr, int hc, int hrp, int hcp) {
    double s = 0;
    int n=0;
    assert(_seekerPlayer1!=NULL);

    //try all seeker actions
    for(int sa=0; sa<HSGlobalData::NUM_ACTIONS; sa++) {
        _seekerPlayer1->playerInfo.currentPos.set(sr,sc);
        if (_seekerPlayer.move(sa)) {
            //can move
            Pos newSPos = _seekerPlayer.playerInfo.currentPos;
            s += score(newSPos.row(), newSPos.col(),hr,hc,true); //we say visib because we take avg over all non-avg
            n++;
        }
    }

    s = s/n;

    return s;
}*/

/*
bool VerySmartHider::scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double& s) {
    assert(isSeeker());

    _hiderPlayer.playerInfo.currentPos.set(hr,hc);
    if (_hiderPlayer.move(a)) {
        //move correct //ag131107: THIS was only for visible cases!!!!
        Pos newHPos = _hiderPlayer.playerInfo.currentPos;

        if (visib || _allKnowing) {
            //visible -> we can score directly

            s = scoreForState(sr,sc,newHPos.row(),newHPos.col(), hr, hc);

        } else {
            //not visible -> go through all possible location and get: avg/min/max based on scoreType
            std::vector<Pos> invisPointsVector = _map->getInvisiblePoints(hr,hc,_params->takeDynObstOcclusionIntoAccountWhenLearning);

            if (invisPointsVector.size()>0) {
                vector<Pos>::iterator it;
                double maxS = HSGlobalData::INFTY_NEG_DBL; //-1e99;
                double minS = HSGlobalData::INFTY_POS_DBL; // 1e99;

                //loop through invisible points
                for(it=invisPointsVector.begin(); it!=invisPointsVector.end(); it++) {
                    //double as = scoreForState(sr,sc,it->row,it->col, hr, hc);
                    //AG131107: THIS WAS CMPLETELY WRONG!! was checking for seeker pos instead hider pos
                    double as = scoreForState(it->row(),it->col(), newHPos.row(), newHPos.col(), hr, hc);
                    s += as;
                    if (as>maxS) maxS=as;
                    if (as<minS) minS=as;
                }

                switch(_scoreType) {
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
                    throw CException(_HERE_, "unknown score type");
                    break;
                }

            } else { //if visib
                cout<<"ERROR: no invisible points found but said to be not visible."<<endl;
                assert(false);
            }
        }

        return true;

    } else {
        return false;
    }
}
 */

//take into account all possible seeker pos when hidden
bool VerySmartHider::scoreForAction(const Pos &seekerPos, const Pos &hiderPos, int a, double &score) { //(int sr, int sc, int hr, int hc, int a, bool visib, double& s) {
    //_hiderPlayer.playerInfo.currentPos.set(hr,hc);
    Pos newHPos = _map->tryMove(a,hiderPos);

    //if (hPos.isSet()) { //AG150603: already asserted in getNextPosRun
        //move correct //ag131107: THIS was only for visible cases!!!!

    //AG150604: check if action was possible
    if (newHPos.isSet()) {
        if (seekerPos.isSet()) { //visib || _allKnowing) {
            //visible -> we can score directly

            score = scoreForState(seekerPos, newHPos, hiderPos); // (sr,sc,newHPos.row(),newHPos.col(), hr, hc);

        } else {
            //not visible -> go through all possible location and get: avg/min/max based on scoreType
            std::vector<Pos> invisPointsVector = _map->getInvisiblePoints(hiderPos, _params->takeDynObstOcclusionIntoAccountWhenLearning);

            if (invisPointsVector.size()>0) {
                vector<Pos>::iterator it;
                double maxS = HSGlobalData::INFTY_NEG_DBL; //-1e99;
                double minS = HSGlobalData::INFTY_POS_DBL; // 1e99;

                //loop through invisible points
                for(it=invisPointsVector.begin(); it!=invisPointsVector.end(); it++) {
                    //double as = scoreForState(sr,sc,it->row,it->col, hr, hc);
                    //AG131107: THIS WAS CMPLETELY WRONG!! was checking for seeker pos instead hider pos
                    double as = scoreForState(*it, newHPos, hiderPos); // (it->row(),it->col(), newHPos.row(), newHPos.col(), hr, hc);
                    score += as;
                    if (as>maxS) maxS=as;
                    if (as<minS) minS=as;
                }

                switch(_scoreType) {
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
                    throw CException(_HERE_, "unknown score type");
                    break;
                }

            } /*else { //if visib
                cout<<"ERROR: no invisible points found but said to be not visible."<<endl;
                assert(false);
            }*/ // //AG150604: only possible if no obstacles in case of 1 seeker,
                   //in case of more, it is probable to have obstacles and full visibility
        }

        return true;

    } else { // if (newHPos.isSet())


        return false;
    }
}


std::string VerySmartHider::getName() const {
    static string sname;
    if (sname.empty()) {
        QString name;
        if (_allKnowing) {
            name = "AllKnowingVerySmartHider";
        } else {
            name = "VerySmartHider";
        }

        //add params
        name += "_std=" + QString::number(_scoreRandStd) + "_DhsF=" + QString::number(_scoreDhsFactor) + "_stdLess=" + QString::number(_scoreStdLessRandDist);

        sname = name.toStdString();
    }

    return sname;
}


int VerySmartHider::getHiderType() const {
    return _allKnowing ? HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING : HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART;
}
