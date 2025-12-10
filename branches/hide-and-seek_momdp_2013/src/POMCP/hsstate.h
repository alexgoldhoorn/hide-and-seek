#ifndef HSSTATE_H
#define HSSTATE_H

#include "HSGame/gmap.h"

#include "state.h"

namespace pomcp {

/*!
 * \brief The HSState class position of hider and seeker
 */
class HSState : public State {
public:
    HSState();
    HSState(Pos seekerPos, Pos hiderPos);
    HSState(int seekerRow, int seekerCol, int hiderRow, int hiderCol);

    Pos hiderPos;
    Pos seekerPos;

    bool hiderVisible();

    virtual State* copy();

    virtual std::string toString();
};

}

#endif // HSSTATE_H
