#ifndef RANDOMHIDER_H
#define RANDOMHIDER_H

#include <vector>

#include "AutoHider/autohider.h"


#include "autowalker.h"

/*!
 * \brief The RandomHider class Initializes at a random hidden position (if available), then choses actions randomly.
 */
class RandomHider : /*public AutoHider,*/ public AutoWalker
{
public:
    /*!
     * \brief RandomHider
     * \param params
     * \param n number of auto walkers, default 0, and only 1 'hider'
     */
    RandomHider(SeekerHSParams* params, std::size_t n = 0);

    virtual ~RandomHider();

    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1);

    virtual std::string getName() const;

    virtual int getHiderType() const;
    
    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos);

    /*virtual void setMap(GMap* map);

    virtual GMap* getMap() const;

    //AG150525: override from AutoPlayer, because they are overridden from AutoWalker
    //virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);
    virtual bool initBelief(GMap* gmap, PlayerInfo* otherPlayer);
    virtual bool initBeliefMulti(GMap* gmap, std::vector<PlayerInfo*> playerVec, int thisPlayerID, int hiderPlayerID);

    virtual SeekerHSParams* getParams() const;*/
    
protected:
    /*!
     * \brief initRandomWalkers initializes the random walkers
     * \param n
     */
    //virtual void initRandomWalkers(int n);

    virtual bool initBeliefRun();

    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);

};

#endif // RANDOMHIDER_H
