#include "smarthider.h"

#include "Utils/generic.h"
#include "hsglobaldata.h"

SmartHider::SmartHider(bool assumeAlwaysVisible) : RandomHider()
{
    _allKnowing = assumeAlwaysVisible;

    setParameters();

    if (_allKnowing) {
        _name = "AllKnowingSmartHider";
    } else {
        _name = "SmartHider";
    }
}

void SmartHider::setParameters(double scoreRandStd, double scoreDhsFactor, double scoreStdLessRandomDist) {
    _scoreRandStd = scoreRandStd;
    _scoreDhsFactor = scoreDhsFactor;
    _scoreStdLessRandDist = scoreStdLessRandomDist;
}

int SmartHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible) {
    int maxA = 0;
    double maxS = -1e99;

    if (!opponentVisible) opponentVisible = _allKnowing;

    DEBUG_AUTOHIDER(cout<<"Pos: s="<<seekerPos.toString()<<" h="<<hiderPos.toString()<<" visib="<<opponentVisible<<endl;);

    //try all actions and get highest score
    for(int a=0;a<HSGlobalData::NUM_ACTIONS;a++) {
        double s=0;
        bool actOK = scoreForAction(seekerPos.row,seekerPos.col,hiderPos.row,hiderPos.col,a,opponentVisible,s);
        if (actOK && s>maxS) {
            maxS = s;
            maxA = a;            
        }
        DEBUG_AUTOHIDER(if (actOK) cout <<" a="<<a<<"-> s="<<s<<endl;);
    }

    DEBUG_AUTOHIDER(cout<<"Best action: "<<maxA<<endl;);

    //check if movement is ok
    /*_hiderPlayer.setCurPos(hiderPos);
    if (!_hiderPlayer.MovePlayer(maxA)) {
        maxA = HSGlobalData::ACT_H;
        DEBUG_HS(cout << "Best action was: "<<maxA<<" but can't move so to halt"<<endl;);
    }*/

    return maxA;
}

bool SmartHider::scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double& s) {
    _hiderPlayer.setcurpos(hr,hc);
    if (_hiderPlayer.move(a)) {
        Pos newHPos = _hiderPlayer.Getcurpos();

        s = score(sr,sc,newHPos.row,newHPos.col,visib);

        return true;
    } else {
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
    double dhb = _map->distance(hr,hc,base.row,base.col);


    double dhs_factor = 1, dhb_factor = 1;
    if (visib && dhs < _scoreStdLessRandDist)
        dhs_factor = dhs/_scoreStdLessRandDist;
    if (dhb < _scoreStdLessRandDist)
        dhb_factor = dhb/_scoreStdLessRandDist;

    double noise = 0;

    if (_scoreRandStd>0) {
        noise = (dhs_factor*dhb_factor* _scoreRandStd *rand()/RAND_MAX);
    }
    //DEBUG_HS(cout << "dhs_f="<<dhs_factor<<"; dhb_f="<<dhb_factor<< "; noise="<<noise<<endl;);

     //AG130206: removed *L because
    return -dhb + _scoreDhsFactor * dhs + noise;  //removed: L (=rows*cols) since is done for all
}
