#ifndef RANDOMHIDER_H
#define RANDOMHIDER_H

#include "autohider.h"
#include "hsglobaldata.h"

class RandomHider : public AutoHider
{
public:
    RandomHider();

    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible);

    virtual Pos getInitPos();

    virtual std::string getName() {
        return "RandomHider";
    }

    virtual int getHiderType() {
        return HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM;
    }
};

#endif // RANDOMHIDER_H
