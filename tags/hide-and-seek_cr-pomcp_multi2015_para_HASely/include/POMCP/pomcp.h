#ifndef POMCP_H
#define POMCP_H

#include "autoplayer.h"

namespace pomcp {


class POMCP : public AutoPlayer
{
public:
    POMCP();


    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);


    virtual int getNextAction(Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);



    //TODO: maybe later can do a check of belief
    virtual double getBelief(int r, int c) {
        return 0;
    }

    virtual bool tracksBelief() {
        return false;
    }


};

}
#endif // POMCP_H
