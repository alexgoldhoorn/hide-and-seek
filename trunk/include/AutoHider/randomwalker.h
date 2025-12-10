#ifndef RANDOMWALKER_H
#define RANDOMWALKER_H

#include <vector>

#include "AutoHider/autohider.h"
#include "AutoHider/autowalker.h"

/*!
 * \brief The RandomWalker class Randomly walks to another point by taking a random goal on the map and going there
 * following the PathPlanner (shortest path) and when the goal is reached another random goal is chosen.
 */
class RandomWalker : /*public AutoHider,*/ public AutoWalker {
public:
    RandomWalker(SeekerHSParams* params, std::size_t n = 0);

    virtual ~RandomWalker();

    //AG150525: override from AutoPlayer, because they are overridden from AutoWalker
    //virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);
    /*virtual bool initBelief(GMap* gmap, PlayerInfo* otherPlayer);
    virtual bool initBeliefMulti(GMap* gmap, std::vector<PlayerInfo*> playerVec, int thisPlayerID, int hiderPlayerID);*/

    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1);


    //virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);

    virtual int getHiderType() const;

    virtual std::string getName() const;

    //virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos);

    /*virtual void setMap(GMap* map);

    virtual GMap* getMap() const;

    virtual SeekerHSParams* getParams() const;*/

protected:
    virtual bool initBeliefRun();

    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);


    //! next goal for this walker
    Pos _nextGoal;

    //! list of next goals for auto walker
    std::vector<Pos> _autoWalkerGoalVec;


};

#endif // RANDOMWALKER_H
