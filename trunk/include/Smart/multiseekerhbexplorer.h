#ifndef MULTISEEKERHBEXPLORER_H
#define MULTISEEKERHBEXPLORER_H

//#include "Smart/highestbelieffollower.h"
#include "Smart/twoseekerhbexplorer.h"
#include "Base/playerinfowrapper.h"


/*!
 * \brief The MultiSeekerHBExplorer class first calculates the n_hb highest belief points (calcNextHBList), which should be sent out
 * to all seekers. Then with selectRobotPosMulti the goal location for this seeker is called.
 *
 * <TODO -> adapt for MULTI> The MultiSeekerHBExplorer class This class should implement the behaviour of a seeker doing find-and-follow.
 *  The method is based on the HighestBelief Explorer and 'hunt' Follower with two seekers.
 * Order of process:
 *  1. receive own observation(s) O[]
 *  2. filter observations with autoplayer.filter(O) -> o
 *  3. send o to seeker 2
 *  4. receive o2, of seeker2
 *  5. initBelief(o1,o2,..)
 *  6. while not finished
 *  7.   <p1,p2>=getNextPos2(o1,o2..)
 *  8.   send <p1,p2> to robot2
 *  9.   receive <p1,p2>_2 from robot2
 * 10.   p1 = chooseGoal(<p1,p2>,<p1,p2>_2,beliefs,..)
 * 11.   goto p1
 * 12. end while
 *
 *
 */
class MultiSeekerHBExplorer : public TwoSeekerHBExplorer
{
public:

    MultiSeekerHBExplorer(SeekerHSParams* params, AutoPlayer* autoPlayerWBelief);

    virtual ~MultiSeekerHBExplorer();

    virtual bool calcNextHBList(int actionDone=-1);

    virtual Pos selectRobotPosMulti();

    virtual std::string getName() const;

protected:
    virtual bool initBeliefRun();

    virtual bool chooseHiderObsFromMulti(Pos& chosenHider);

    virtual void useExplorer();

    virtual  void useFollower(const Pos& hiderPos);

    /*!
     * \brief combineHBPoses this function returns all highest belief poses, and combining them if they already
     * exist by adding their beliefs (of different players)
     * \param hbPosesVec [out] vector with all HB poses of all players
     */
    virtual void combineHBPoses(std::vector<BPos>& hbPosesVec);

    //! The list of player info wrappers, which is used to sort the players.
    //! Note that it only should contain seekers.
    std::vector<PlayerInfoWrapper> _piWrapperVec;



};


#endif // MULTISEEKERHBEXPLORER_H
