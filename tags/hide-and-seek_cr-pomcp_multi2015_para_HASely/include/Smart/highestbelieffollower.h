#ifndef HIGHESTBELIEFFOLLOWER_H
#define HIGHESTBELIEFFOLLOWER_H

#include "Smart/follower.h"

#include "Utils/timer.h"
#include "HSGame/bpos.h"

/*!
 * \brief The HighestBeliefFollower class follows the highest belief of the autoplayer. It does
 *  update the belief, i.e. run the getNextPosRun function. The position returned is the one
 *  with the highest belief. It does NOT check the hider pos, and it keeps the position during a certain time.
 */
class HighestBeliefFollower : public AutoPlayer
{
public:
    /*!
     * \brief Follower constructor
     * \param params
     */
    HighestBeliefFollower(SeekerHSParams* params, AutoPlayer* autoPlayerWBelief);

    virtual ~HighestBeliefFollower();

    //override these functions since we should init the belief to the 'belief follower' class which is either single or multi
    virtual bool initBelief(GMap* gmap, PlayerInfo* otherPlayer);
    virtual bool initBeliefMulti(GMap* gmap, std::vector<PlayerInfo*> playerVec, int thisPlayerID, int hiderPlayerID);


    //virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    //virtual double getNextDirection(Pos seekerPos, Pos hiderPos, bool opponentVisible, bool &haltAction);

    virtual bool isSeeker() const;

    virtual std::string getName() const;

    virtual bool useGetAction() const;

    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() const;

    virtual bool canScoreObservations();

    virtual double scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone=-1);

    virtual Pos getClosestSeekerObs(Pos seekerPos);

    virtual void setMap(GMap* map);

protected:    
    //virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);


    virtual bool initBeliefRun();

    /*!
     * \brief getMapPosFromZoomCel returns a map coordinate based on the 'zoomed' coordinates zr,zc. It tries the center of this 'zoomed' cell,
     * but if it is an obstacle, then the first non-obstacle map cell that is contained by the zoomed cell is used.
     * \param zr
     * \param zc
     * \return
     */
    Pos getMapPosFromZoomCel(int zr, int zc);

    //AG120123: added n, return vector of n highest pos
    /*!
     * \brief findHighestBelief finds (at max n) highest belief points with a belief
     *
     * Complexity: O(n_hb * n_gridCells) -> O(n^2)
     *
     * \param seekerPos pos from which distance is searched
     * \param n number of highest pos
     * \param maxSearchRange (optional) maximum range for searching the highest belief (0=not used)
     * \return vector of highest belief points ordered starting with the point with the highest belief
     */
    virtual std::vector<BPos> findHighestBelief(const Pos& seekerPos, unsigned int n=1, double maxSearchRange=0); //, std::vector<double>* maxBVec=NULL);

    //! last goal pos
    Pos _lastGoalPos;

    //! autoplayer that has belief
    AutoPlayer* _autoPlayerWBelief;

    //! timer
    Timer _timer;

    //! timer id
    int _timerID;

    //! number of steps done since last update
    int _stepsToLastUpdate;
};

#endif // HIGHESTBELIEFFOLLOWER_H
