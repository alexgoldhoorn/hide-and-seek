#ifndef HIGHESTBELIEFFOLLOWER_H
#define HIGHESTBELIEFFOLLOWER_H

#include "Smart/follower.h"

#include "Utils/timer.h"

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


    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    virtual double getNextDirection(Pos seekerPos, Pos hiderPos, bool opponentVisible, bool &haltAction);

    virtual bool isSeeker() const;

    virtual std::string getName() const;

    virtual bool useGetAction() const;

    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() const;

    virtual bool canScoreObservations();

    virtual double scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone=-1);

    virtual Pos getClosestSeekerObs(Pos seekerPos);

protected:    
    virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);


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
     * \brief findHighestBelief finds highest belief or a non-set Pos if
     * \param seekerPos pos from which distance is searched
     * \param n number of highest pos
     * \param maxSearchRange (optional) maximum range for searching the highest belief
     * \param maxBVec [out] (optional) vector of the belief values corresponding to the highest belief points
     * \return vector of highest belief points ordered starting with the point with the highest belief
     */
    virtual std::vector<Pos> findHighestBelief(const Pos& seekerPos, unsigned int n=1, double maxSearchRange=0, std::vector<double>* maxBVec=NULL);

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
