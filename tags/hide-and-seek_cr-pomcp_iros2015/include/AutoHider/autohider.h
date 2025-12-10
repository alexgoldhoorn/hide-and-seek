#ifndef AUTOHIDER_H
#define AUTOHIDER_H

#include "autoplayer.h"
#include "HSGame/gplayer.h"
#include <string>

/*!
 * \brief The AutoHider class abstract class for auto hiders
 */
class AutoHider : public AutoPlayer
{
public:
    AutoHider(SeekerHSParams* params);

    virtual ~AutoHider();

    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1)=0;

    //virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);


    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() const;


    virtual int getHiderType() const=0;

    virtual bool isSeeker() const;

    virtual void setMap(GMap *map);


protected:
    Player _hiderPlayer;
    Player _seekerPlayer;
};

#endif // AUTOHIDER_H
