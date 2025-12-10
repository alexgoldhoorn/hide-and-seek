#include "AutoHider/autohider.h"
#include "exceptions.h"

AutoHider::AutoHider(SeekerHSParams* params) : AutoPlayer(params)
{
}

//bool AutoHider::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible)  {
//bool AutoHider::initBeliefRun() {
    //setMap(gmap);
    /*assert(_seekerPlayer1==NULL);

    //AG150525: find first seeker in list
    for (PlayerInfo* playerInfo : _playerInfoVec) {
        if (playerInfo->isSeeker()) {
            _seekerPlayer1 = playerInfo;
            break;
        }
    }*/ //AG150527: done in autoplayer


/*
    return true;
}*/

AutoHider::~AutoHider() {
}

/*void AutoHider::setMap(GMap *map) {
    //cout << "AutoHider::setMap"<<endl;
    //_gmap = map;
    AutoPlayer::setMap(map);
    _seekerPlayer.setMap(map);
    _hiderPlayer.setMap(map);
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
