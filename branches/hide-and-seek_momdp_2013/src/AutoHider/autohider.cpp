#include "autohider.h"


AutoHider::AutoHider() : AutoPlayer()
{
}

bool AutoHider::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible)  {
    setMap(gmap);

    return true;
}

void AutoHider::setMap(GMap *map) {
    _map = map;
    _seekerPlayer.setMap(map);
    _hiderPlayer.setMap(map);
}
