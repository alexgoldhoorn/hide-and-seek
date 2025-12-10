#ifndef AUTOHIDER_H
#define AUTOHIDER_H

#include "autoplayer.h"
#include "HSGame/gplayer.h"
#include <string>

class AutoHider : public AutoPlayer
{
public:
    AutoHider();

    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    virtual int getNextAction(Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible)=0;

    virtual Pos getInitPos()=0;

    virtual double getBelief(int r, int c) {
        return 0;
    }

    virtual bool tracksBelief() {
        return false;
    }

    virtual void setMap(GMap* map);

    virtual std::string getName()=0;

    virtual int getHiderType()=0;

protected:
    GMap* _map;
    Player _hiderPlayer;
    Player _seekerPlayer;
};

#endif // AUTOHIDER_H
