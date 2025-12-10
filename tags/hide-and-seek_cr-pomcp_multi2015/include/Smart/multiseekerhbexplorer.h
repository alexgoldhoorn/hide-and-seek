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


#ifdef OLD_CODE
    //TODO: implement methods +


    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    virtual bool initBelief2(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible, Pos seeker2InitPos, Pos hiderObs2InitPos, double obs1p);

    virtual bool getNextRobotPoses2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, std::vector<int> &actions,
                                    std::vector<Pos>& goalPosesVec, int actionDone=-1, int n=1, std::vector<double>* goalPosesBelVec=NULL);

    virtual Pos selectRobotPos2(Pos *otherSeekerPos1, Pos *otherSeekerPos2, double otherSeekerPos1B, double otherSeekerPos2B, int n, Pos* chosenPos=NULL);



    virtual bool useGetAction() const;

    virtual bool handles2Obs() const;

protected:
    virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

    virtual Pos getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p,
                               std::vector<int> &actions, int actionDone=-1, int n=1);

    /*!
     * \brief calcExplorationEvaluation calculate the exploration value, based on
     * Burgard, W., Moors, M., Stachniss, C., & Schneider, F. E. (2005). Coordinated Multi-Robot Exploration, 21(3), 376–386.
     * which we have adapted to:
     *      U_t - beta V_ts + gamma b_t
     *
     * where U is the utility which is based on whether it was already chosen as goal before, or it was close (in the visibility range) of a
     * chosen goal; beta (SeekerHSParams.multiSeekerExplorerDistWeight) is the weight of the robot-highest belief distance (V);
     * gamma (multiSeekerExplorerBeliefWeight) the weight of the belief point t.
     *
     * \param seekerPos the position of the seeker for which we want to find a target
     * \param highestBeliefPosVec list of positions with highest beliefs, it is assumed that the list is ordered from high to low belief.
     * The first item always should contain a value.
     * \param highestBeliefVec list of belief values for the highest belief vector (same order)
     * \param otherRobotGoal (optional) if set, this is used to recalculate the utility (i.e. reduce it, see Burgard et al. 2005)
     * \return index of the highest belief for the selected robot (for pos and belief)
     */
    int calcExplorationEvaluation(const Pos& seekerPos, const std::vector<Pos>& highestBeliefPosVec, const std::vector<double>& highestBeliefVec,
                                     Pos* otherRobotGoal);

    /*!
     * \brief calcOtherRobotPos Find the position for the position close to the hider (hpos) when one
     * seeker's postion (s1p) is already close to the robot. The other robot (s2p) should go to the given new
     * pos based on it's current pos and the follow distance to the hider and the minimum distance to the other robot.
     * \param s1p
     * \param hpos
     * \param s2p
     * \return
     */
    Pos calcOtherRobotPos(const Pos& s1p, const Pos& hpos, const Pos& s2p);

    //! last hider pos was consistent (with obs of other seeker)
    bool _lastHiderPosConsistent;

    //! goal pos of seeker 2
    Pos _lastSeeker2GoalPos;

    //! last seeker 2 pos
    Pos _lastSeeker2Pos;

    //! goal positions of this ([0]) and other ([1]) seeker
    std::vector<Pos> _myCalcGoalPosVec;

    //! beliefs of goal positions as calculated here (same order as _myCalcGoalPosVec)
    std::vector<double> _myCalcBeliefGoalPosVec;

#endif

};


#endif // MULTISEEKERHBEXPLORER_H
