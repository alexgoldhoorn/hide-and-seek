#include "AutoHider/autohider.h"
#include "exceptions.h"

AutoHider::AutoHider(SeekerHSParams* params) : AutoPlayer(params)
{    
}

bool AutoHider::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible)  {
    setMap(gmap);

    return true;
}

AutoHider::~AutoHider() {
}

void AutoHider::setMap(GMap *map) {
    //cout << "AutoHider::setMap"<<endl;
    //_gmap = map;
    AutoPlayer::setMap(map);
    _seekerPlayer.setMap(map);
    _hiderPlayer.setMap(map);
}

/*vector<int> AutoHider::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n) {
    vector<int> act;
    throw CException("AutoHider::getNextMultipleActions: not implemented yet",_HERE_);
    return act;
}*/

double AutoHider::getBelief(int r, int c) {
    return 0;
}

bool AutoHider::tracksBelief() const {
    return false;
}


bool AutoHider::isSeeker() const {
    return false;
}
