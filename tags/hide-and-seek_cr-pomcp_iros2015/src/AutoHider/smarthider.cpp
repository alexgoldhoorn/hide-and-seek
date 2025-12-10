#include "AutoHider/smarthider.h"

#include <iostream>
#include <cassert>

#include "Utils/generic.h"
#include "hsglobaldata.h"

using namespace std;

SmartHider::SmartHider(SeekerHSParams* params, bool assumeAlwaysVisible) : RandomHider(params)
{
    _allKnowing = assumeAlwaysVisible;

    setParameters();

    //_name.clear();
}

SmartHider::~SmartHider() {
}

void SmartHider::setParameters(double scoreRandStd, double scoreDhsFactor, double scoreStdLessRandomDist) {
    _scoreRandStd = scoreRandStd;
    _scoreDhsFactor = scoreDhsFactor;
    _scoreStdLessRandDist = scoreStdLessRandomDist;
}

int SmartHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    //AG131104: instead of 1 max actions, keep track of all max actions with max score, then choose randomly one
    vector<int> maxActVector;
    double maxS =  HSGlobalData::INFTY_NEG_DBL; //-1e99 ;

    if (!opponentVisible) opponentVisible = _allKnowing;

    DEBUG_AUTOHIDER(cout<<"Pos: s="<<seekerPos.toString()<<" h="<<hiderPos.toString()<<" visib="<<opponentVisible<<endl;);

    //try all actions and get highest score
    for(int a=0;a<HSGlobalData::NUM_ACTIONS;a++) {
        double s=0;
        bool actOK = scoreForAction(seekerPos.row(),seekerPos.col(),hiderPos.row(),hiderPos.col(),a,opponentVisible,s);
        if (actOK && s>=maxS) {
            //maxA = a;
            if (s>maxS) {
                //it's better
                maxS = s;
                maxActVector.clear();
            }

            //add action
            maxActVector.push_back(a);
        }
        DEBUG_AUTOHIDER(if (actOK) cout <<" a="<<ACTION_COUT(a)<<"-> s="<<s<<endl;);
    }


    int maxA = -1;

    //AG131104: choose action of best
    if (maxActVector.size()==1) {
        maxA = maxActVector[0];
    } else {
        maxA = maxActVector[random(maxActVector.size()-1)];
    }

    DEBUG_AUTOHIDER(cout<<"Best action (chosen): "<<ACTION_COUT(maxA)<<endl;);
    return maxA;
}

bool SmartHider::scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double& s) {
    _hiderPlayer.setCurPos(hr,hc);
    cout<<" scoreForAction: "<<ACTION_COUT(a)<<" from "<<_hiderPlayer.getCurPos().toString()<<":"<<flush;
    /*if (_hiderPlayer.move(a)) {
        Pos newHPos = _hiderPlayer.getCurPos();*/
    Pos newHPos = _map->tryMove(a, _hiderPlayer.getCurPos());
    if (newHPos.isSet()) {

        cout <<"ok: "<<newHPos.toString()<<endl;

        s = score(sr,sc,newHPos.row(),newHPos.col(),visib);

        return true;
    } else {
        cout<<"NOK"<<endl;
        return false;
    }
}

double SmartHider::score(int sr, int sc, int hr, int hc, bool visib) {
    assert(_map->isPosInMap(sr,sc));
    assert(!visib || _map->isPosInMap(hr,hc));

    //AG130206: the closer to the goal or the opponent, the less random noise

    Pos base = _map->getBase();
    double dhs = 0;
    //AG130507: added visiblity check, because this was NOT present yet!!! IT alway had full knowledge
    if (visib) dhs = _map->distance(sr,sc,hr,hc);
    double dhb = _map->distance(hr,hc,base.row(),base.col());


    double dhs_factor = 1, dhb_factor = 1;
    if (visib && dhs < _scoreStdLessRandDist)
        dhs_factor = dhs/_scoreStdLessRandDist;
    if (dhb < _scoreStdLessRandDist)
        dhb_factor = dhb/_scoreStdLessRandDist;

    double noise = 0;

    if (_scoreRandStd>0) {
        noise = (dhs_factor * dhb_factor * _scoreRandStd * randomDouble()); //rand()/RAND_MAX);
    }
    //DEBUG_HS(cout << "dhs_f="<<dhs_factor<<"; dhb_f="<<dhb_factor<< "; noise="<<noise<<endl;);

     //AG130206: removed *L because
    return -dhb + _scoreDhsFactor * dhs + noise;  //removed: L (=rows*cols) since is done for all
}

std::string SmartHider::getName() const {
    static string sname;
    if (sname.empty()) {
        QString name;
        if (_allKnowing) {
            name = "AllKnowingSmartHider";
        } else {
            name = "SmartHider";
        }

        //add params
        name += "_std=" + QString::number(_scoreRandStd) + "_DhsF=" + QString::number(_scoreDhsFactor) + "_stdLess=" + QString::number(_scoreStdLessRandDist);

        sname = name.toStdString();
    }

    return sname;
}


int SmartHider::getHiderType() const {
    return _allKnowing ? HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING : HSGlobalData::OPPONENT_TYPE_HIDER_SMART;
}
