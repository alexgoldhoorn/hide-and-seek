#include "smartseeker.h"

#include "hsglobaldata.h"
#include "Utils/generic.h"

SmartSeeker::SmartSeeker(SeekerHSParams* params) //char scoreType)
{
    //_scoreType = scoreType;
    _params = params;
    initRandomizer();
}


bool SmartSeeker::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible) {
    _map = gmap;
    _seekerPlayer.setMap(gmap);
    _hiderPlayer.setMap(gmap);
    _maxScore = _map->rowCount()*_map->colCount();
    _base = _map->getBase();
}


int SmartSeeker::getNextAction(Pos seekerPos, Pos hiderPos, bool hiderVisible) {
    int maxA = 0;
    double maxS = -1e99;

    DEBUG_HS(cout<<"Pos: s="<<seekerPos.toString()<<" h="<<hiderPos.toString()<<" visib="<<hiderVisible<<endl;);

    //try all actions and get highest score
    for(int a=0;a<HSGlobalData::NUM_ACTIONS;a++) {
        double s = 0;
        bool actOK = scoreForAction(seekerPos.row,seekerPos.col,hiderPos.row,hiderPos.col,a,hiderVisible,s);
        if (actOK) {
            if (s>maxS) {
                maxS = s;
                maxA = a;
            }
            DEBUG_HS(cout <<" a="<<a<<"-> s="<<s<<endl;);
        }
    }

    DEBUG_HS(cout<<"Best action: "<<maxA<<endl;);

    return maxA;
}


vector<int> SmartSeeker::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n) {
    vector<int> actions;
    Pos sPos;
    sPos.set(seekerPos);

    //get next actions
    for(int i=0; i<n; i++) {
        //get action
        int act = getNextAction(sPos, hiderPos, opponentVisible);
        //add action
        actions.push_back(act);

        //move seeker
        Pos newPos = _seekerPlayer.tryMove(act,sPos);
        if (newPos.isSet()) {
            sPos.set(newPos);
            //else: couldn't move, so stay same pos
        }

        //NOTE: not moving hider..
    }

    return actions;
}


double SmartSeeker::scoreForState(int sr, int sc, int hr, int hc) {
    double s = 0;
    int n = 0;

    //try all hider actions
    for(int ha=0;ha<HSGlobalData::NUM_ACTIONS;ha++) {
        _hiderPlayer.setcurpos(hr,hc);
        if (_hiderPlayer.move(ha)) {
            //if it can move
            Pos newHPos = _hiderPlayer.Getcurpos();
            s += score(sr,sc,newHPos.row,newHPos.col);
            n++;
        }
    }

    return s/n; //note: n>0 since H action always is possible
}

bool SmartSeeker::scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double& s) {
    _seekerPlayer.setcurpos(sr,sc);
    if (_seekerPlayer.move(a)) {
        Pos newSPos = _seekerPlayer.Getcurpos();

        if (visib) {
            //visible, so score can be directly calculated
            s = scoreForState(newSPos.row,newSPos.col,hr,hc);
        } else {
            //not visible -> go through all possible location and get: avg/min/max based on scoreType
            //use NOW invisible cells since this is our knowledge
            vector<Pos> invisPointsVector = _map->getInvisiblePoints(sr,sc);

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
                        int removeI = random(invisPointsVector.size());
                        invisPointsVector.erase(invisPointsVector.begin()+removeI);
                    }
                    cout << "cells chosen..now calc"<<endl;
                }

                //calc score for each invisible state
                for(it=invisPointsVector.begin();it!=invisPointsVector.end();it++) {
                    double as = scoreForState(sr,sc,it->row,it->col);
                    s += as;
                    if (as>maxS) maxS=as;
                    if (as<minS) minS=as;
                }

                switch(_params->smartSeekerScoreType) {
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
                    assert("unknown score type");
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

    //triangle rule score
    double dsh = _map->distance(sr,sc,hr,hc);
    double dhb = _map->distance(hr,hc,_base.row,_base.col);
    double dsb = _map->distance(sr,sc,_base.row,_base.col);

    double s = -dsh;

    if (dhb>=dsb) {
        s += _maxScore;
    }

    return s;
}

double SmartSeeker::getBelief(int r, int c) {
    return 0; //not to be used
}
