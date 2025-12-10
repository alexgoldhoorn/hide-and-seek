#include "abstractautoplayer.h"

#include <iostream>
//#include "exceptions.h"

using namespace  std;

AbstractAutoPlayer::AbstractAutoPlayer(SeekerHSParams* params) : AutoPlayer(params) {
}

AbstractAutoPlayer::AbstractAutoPlayer(SeekerHSParams* params, GMap* map) : AutoPlayer(params, map) {
}

AbstractAutoPlayer::~AbstractAutoPlayer() {
}

bool AbstractAutoPlayer::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    cout<<"AbstractAutoPlayer::initBelief: not implemented"<<endl;
    return false;
}

bool AbstractAutoPlayer::isSeeker() const {
    return true;
}

std::string AbstractAutoPlayer::getName() const {
    return "AbstractAutoPlayer";
}
