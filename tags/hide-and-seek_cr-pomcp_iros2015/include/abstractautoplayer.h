#ifndef ABSTRACTAUTOPLAYER_H
#define ABSTRACTAUTOPLAYER_H

#include "autoplayer.h"

/*!
 * \brief The AbstractAutoPlayer class Abstract auto player with all functions implemented, but does nothing. Only for debug/testing.
 */
class AbstractAutoPlayer : public AutoPlayer {
public:
    AbstractAutoPlayer(SeekerHSParams* params);

    AbstractAutoPlayer(SeekerHSParams* params, GMap* map);

    virtual ~AbstractAutoPlayer();

    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    virtual bool isSeeker() const;

    virtual std::string getName() const;
};

#endif // ABSTRACTAUTOPLAYER_H
