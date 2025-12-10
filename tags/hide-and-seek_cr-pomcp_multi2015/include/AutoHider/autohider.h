#ifndef AUTOHIDER_H
#define AUTOHIDER_H

#include "Base/autoplayer.h"
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

    //AG150525: disabled because update to multi
    //virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);
    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1)=0;
    //virtual void setMap(GMap *map);

    //virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);


    //virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);


    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() const;

    /*!
     * \brief getHiderType return the hider type (HSGlobalData::OPPONENT_TYPE_*)
     * \return
     */
    virtual int getHiderType() const=0;

    virtual bool isSeeker() const;




protected:
    //AG150525: disabled because update to multi, now use _hiderPlayer
    /*Player _hiderPlayer;
    Player _seekerPlayer;*/

    //virtual bool initBeliefRun();


};

#endif // AUTOHIDER_H
