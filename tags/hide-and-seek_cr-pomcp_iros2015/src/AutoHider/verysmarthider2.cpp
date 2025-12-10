#include "AutoHider/verysmarthider2.h"

#include <cassert>
#include <cmath>

#include "exceptions.h"

#include "Utils/generic.h"

using namespace std;

VerySmartHider2::VerySmartHider2(SeekerHSParams* params, bool allKnowing, char scoreTypeHidden, char scoreTypeSeekerActions) :
    VerySmartHider(params, allKnowing, scoreTypeHidden) {

    _scoreTypeSeekerActions = scoreTypeSeekerActions;

    //_name.clear();

    _maxScore = 0;
}

VerySmartHider2::~VerySmartHider2() {
}

void VerySmartHider2::setParameters2(double D2, double f1, double f2, double k, double f3, bool allKnowing,
                                     char scoreTypeHidden, char scoreTypeSeekerActions) {
    _D2 = D2;
    _f1 = f1;
    _f2 = f2;
    _k = k;
    _f3 = f3;
    _allKnowing = allKnowing;
    _scoreType = scoreTypeHidden;
    _scoreTypeSeekerActions = scoreTypeSeekerActions;
}

//more inteligent version: take all possible seeker moves into account
double VerySmartHider2::scoreForState(int sr, int sc, int hr, int hc, int hrp, int hcp) {

    double s = 0;
    int n=0;

    double maxS = HSGlobalData::INFTY_NEG_DBL;
    double minS = HSGlobalData::INFTY_POS_DBL;

    //try all seeker actions
    for(int sa=0; sa<HSGlobalData::NUM_ACTIONS; sa++) {
        _seekerPlayer.setCurPos(sr,sc);
        if (_seekerPlayer.move(sa)) {
            //can move
            Pos newSPos = _seekerPlayer.getCurPos();

            //calc score and update sum,min,max
            double as = score(newSPos.row(), newSPos.col(), hr,hc,true); //we say visib because we take avg over all non-avg

            //AG131106: check for cross
            if (newSPos.row()==hrp && newSPos.col()==hcp && hr==sr && hc==sc) {
                as -= _D2;
            }

            s += as;
            if (as>maxS) maxS=as;
            if (as<minS) minS=as;

            n++;
        }
    }

    switch(_scoreTypeSeekerActions) {
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


double VerySmartHider2::score(int sr, int sc, int hr, int hc, bool visib) {
    assert(_map->isPosInMap(sr,sc));
    assert(!visib || _map->isPosInMap(hr,hc));

    //AG130206: the closer to the goal or the opponent, the less random noise

    Pos base = _map->getBase();
    double dhs = 0;
    //AG130507: added visiblity check, because this was NOT present yet!!! IT alway had full knowledge
    if (visib) dhs = _map->distance(sr,sc,hr,hc);
    double dhb = _map->distance(hr,hc,base.row(),base.col());

    double noise = 0;


    if (_scoreRandStd>0) {
        double dhs_factor = 1, dhb_factor = 1;
        if (visib && dhs < _scoreStdLessRandDist)
            dhs_factor = dhs/_scoreStdLessRandDist;
        if (dhb < _scoreStdLessRandDist)
            dhb_factor = dhb/_scoreStdLessRandDist;

        noise = ( dhs_factor*dhb_factor*_scoreRandStd * ( randomDouble(0,2.0) /*2.0*rand()/RAND_MAX*/ - 1 ) ); //AG131113: noise with 0 as center

        //DEBUG_AUTOHIDER(cout << "dhs_f="<<dhs_factor<<"; dhb_f="<<dhb_factor<< "; noise="<<noise<<endl;);
    }


    //DEBUG_HS(cout << "dhs_f="<<dhs_factor<<"; dhb_f="<<dhb_factor<< "; noise="<<noise<<endl;);

     //AG130206: removed *L because
    //return -dhb + _scoreDhsFactor * dhs + noise;  //removed: L (=rows*cols) since is done for all


    double s = noise;

    /*if (dhs > _k) {       //AG131120: THIS WAS THE CORRECTLY WORKING VERSION!
        s += _D2 - dhb + _f1*dhs; //cout<<"[r1";
    } else if (dhs==0) { //TODO: should be minDistToWin (but not used in simulation)
        if (_maxScore<=0) {
            _maxScore = max(_map->rowCount(),_map->colCount());
        }
        s += -_maxScore - _f3 * dhb + _f2*(dhs);
    } */
    if (_maxScore<=0) {
        _maxScore = max(_map->rowCount(),_map->colCount());
    }
    if (dhs > _k) {
        //s += _D2 /*_maxScore*/ - dhb + _f1*dhs; //cout<<"[r1";
        s += _maxScore - dhb + _f1 * dhs;
    } else {//if (dhs==0) { //TODO: should be minDistToWin (but not used in simulation)

        //s += -_maxScore + _f2 * -dhb + _f3 * (dhs);
        s +=  _f2 * -dhb + _f3 * (dhs);
    }



    /*else {
        s = _f3 * -dhb + _f2*(dhs); // -_D2 ); //cout<<"[r2";
    }*/

    //cout << " (Sr"<<sr<<"c"<<sc<<",Hr"<<hr<<"c"<<hc <<") dhs="<<dhs<<";dhb="<<dhb<<"->s="<<s<<"] "<<flush;

    return s;
}


/*void VerySmartHider2::setMap(GMap *map) {
    cout << "VerySmartHider2::setMap"<<endl;
    VerySmartHider::setMap(map);

    _minScore = -max(map->rowCount(),map->colCount());
    DEBUG_AUTOHIDER(cout << "Min score: "<<_minScore<<endl;);
}*/




std::string VerySmartHider2::getName() const {
    static string sname;
    if (sname.empty()) {
        QString name;
        if (_allKnowing) {
            name = "AllKnowingVerySmartHider2";
        } else {
            name = "VerySmartHider2";
        }

        //add params
        name += "_f1=" + QString::number(_f1) + "_f2=" + QString::number(_f2) + "_f3=" + QString::number(_f3) + "_k=" + QString::number(_k)
                + "_nmax=" + QString::number(_scoreRandStd) + "_cless=" + QString::number(_scoreStdLessRandDist);

        sname = name.toStdString();
    }

    return sname;
}
