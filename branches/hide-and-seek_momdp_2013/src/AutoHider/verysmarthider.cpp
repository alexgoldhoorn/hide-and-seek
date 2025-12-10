#include "verysmarthider.h"

#include "Utils/generic.h"
#include "hsglobaldata.h"

VerySmartHider::VerySmartHider(bool allKnowing, char scoreType) : SmartHider(allKnowing)
{
    _scoreType = scoreType;

    if (_allKnowing) {
        _name = "AllKnowingVerySmartHider";
    } else {
        _name = "VerySmartHider";
    }
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
double VerySmartHider::scoreForState(int sr, int sc, int hr, int hc) {
    double s = 0;
    int n=0;

    //try all seeker actions
    for(int sa=0; sa<HSGlobalData::NUM_ACTIONS; sa++) {
        _seekerPlayer.setcurpos(sr,sc);
        if (_seekerPlayer.move(sa)) {
            //can move
            Pos newSPos = _seekerPlayer.Getcurpos();
            s += score(newSPos.row,newSPos.col,hr,hc,true); //we say visib because we take avg over all non-avg
            n++;
        }
    }

    s = s/n;

    return s;
}

//take into account all possible seeker pos when hidden
bool VerySmartHider::scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double& s) {
    _hiderPlayer.setcurpos(hr,hc);
    if (_hiderPlayer.move(a)) {

        if (visib || _allKnowing) {
            //visible -> we can score directly
            Pos newHPos = _hiderPlayer.Getcurpos();

            s = scoreForState(sr,sc,newHPos.row,newHPos.col);

        } else {
            //not visible -> go through all possible location and get: avg/min/max based on scoreType
            vector<Pos> invisPointsVector = _map->getInvisiblePoints(hr,hc);

            if (invisPointsVector.size()>0) {
                vector<Pos>::iterator it;
                double maxS =-1e99;
                double minS = 1e99;
                for(it=invisPointsVector.begin();it!=invisPointsVector.end();it++) {
                    double as = scoreForState(sr,sc,it->row,it->col);
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
                    //should not occur!!
                    break;
                }

            }  else {//if visib
                cout<<"ERROR: no invisible points found but said to be not visible."<<endl;
                assert(false);
            }
        }

        return true;

    } else {
        return false;
    }
}
