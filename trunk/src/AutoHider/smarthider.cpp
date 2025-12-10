#include "AutoHider/smarthider.h"

#include <iostream>
#include <cassert>

#include "Utils/generic.h"
#include "Base/hsglobaldata.h"

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

//int SmartHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
Pos SmartHider::getNextPosRun(int actionDone, int *newAction) {
    //AG131104: instead of 1 max actions, keep track of all max actions with max score, then choose randomly one
    vector<int> maxActVector;
    double maxS =  HSGlobalData::INFTY_NEG_DBL; //-1e99 ;

    //AG150527: assert that the seeker and hider are set
    assert(_seekerPlayer1!=NULL);
    assert(_hiderPlayer!=NULL);
    assert(*_hiderPlayer==playerInfo);

    Pos seekerPos = _seekerPlayer1->currentPos;
    Pos hiderPos = _hiderPlayer->currentPos;

    //AG150603
    //should be own pos
    assert(hiderPos.isSet());

    //check if visible
    bool opponentVisible = _allKnowing;
    if (opponentVisible) {
        opponentVisible = seekerPos.isSet();
        if (opponentVisible)
            //check visibility taking dyn. obstacles into account
            opponentVisible = _map->isVisible(seekerPos,hiderPos,true,_params->simNotVisibDist);
    }

    DEBUG_AUTOHIDER(cout<<"Pos: s="<<seekerPos.toString()<<" h="<<hiderPos.toString()<<" visib="<<opponentVisible<<endl;);

    if (!opponentVisible)
        seekerPos.clear();

    //try all actions and get highest score
    for(int a=0;a<HSGlobalData::NUM_ACTIONS;a++) {
        double s=0;
        bool actOK = scoreForAction(seekerPos,hiderPos,a,s);
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
        maxA = maxActVector[hsutils::random(maxActVector.size()-1)];
    }

    DEBUG_AUTOHIDER(cout<<"Best action (chosen): "<<ACTION_COUT(maxA)<<endl;);

    if (newAction!=NULL)
        *newAction = maxA;

    //AG150527: set next pos
    playerInfo.nextPos = _map->tryMove(maxA, playerInfo.currentPos);
    assert(playerInfo.nextPos.isSet());

    return playerInfo.nextPos;
}

bool SmartHider::scoreForAction(const Pos& seekerPos, const Pos& hiderPos, int a, double& s) {

    DEBUG_AUTOHIDER(cout<<"SmartHider::scoreForAction: "<<ACTION_COUT(a)<<" from "<<hiderPos.toString()<<":"<<flush;);

    //try to move the hider
    Pos newHPos = _map->tryMove(a, hiderPos);
    if (newHPos.isSet()) {

        DEBUG_AUTOHIDER(cout <<"ok: "<<newHPos.toString()<<endl;);

        s = score(seekerPos, hiderPos);

        return true;
    } else {
        DEBUG_AUTOHIDER(cout<<"NOK"<<endl;);
        return false;
    }
}

double SmartHider::score(const Pos& seekerPos, const Pos& hiderPos) {
    //AG130206: the closer to the goal or the opponent, the less random noise

    Pos base = _map->getBase();
    double dhs = 0;
    //AG130507: added visiblity check, because this was NOT present yet!!! IT alway had full knowledge
    if (seekerPos.isSet()) dhs = _map->distance(seekerPos, hiderPos);//sr,sc,hr,hc);
    double dhb = _map->distance(hiderPos, base); //hr,hc,base.row(),base.col());

    double dhs_factor = 1, dhb_factor = 1;
    if (seekerPos.isSet() && dhs < _scoreStdLessRandDist)
        dhs_factor = dhs/_scoreStdLessRandDist;
    if (dhb < _scoreStdLessRandDist)
        dhb_factor = dhb/_scoreStdLessRandDist;

    double noise = 0;

    if (_scoreRandStd>0) {
        noise = (dhs_factor * dhb_factor * _scoreRandStd * hsutils::randomDouble()); //rand()/RAND_MAX);
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
