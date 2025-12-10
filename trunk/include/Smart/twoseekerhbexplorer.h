#ifndef TWOSEEKERHBEXPLORER_H
#define TWOSEEKERHBEXPLORER_H

#include "Smart/highestbelieffollower.h"
#include "Utils/posscoremat.h"

/*!
 * \brief The TwoSeekerHBExplorer class This class should implement the behaviour of a seeker doing find-and-follow.
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
class TwoSeekerHBExplorer : public HighestBeliefFollower
{
public:

    TwoSeekerHBExplorer(SeekerHSParams* params, AutoPlayer* autoPlayerWBelief);
    virtual ~TwoSeekerHBExplorer();


    virtual std::string getName() const;

    virtual bool useGetAction() const;

    virtual bool handles2Obs() const;

    virtual bool getChosenHiderPos(Pos& chosenHiderPos) const;

    virtual bool calcNextRobotPoses2Run(int actionDone);

    virtual Pos selectRobotPosMulti();

    //AG160725
    //! Set score matrix
    void setScoreMat(PosScoreMat* scoreMat);

protected:
    virtual bool initBeliefRun();

    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);

    /*!
     * \brief chooseHiderObsFromMulti Choose the hider observation from all the observations. It returns false
     * if no consistent hider value has been found. Note that the chosenHider pos can be not set (i.e. hidden) and consistent.
     * \param chosenHider [out] the chosen hider pos
     * \return is the observation consistent among the seekers
     */
    virtual bool chooseHiderObsFromMulti(Pos& chosenHider);

    //! use the explorer to find the next poses
    virtual void useExplorer();

    //! use the follower
    virtual  void useFollower(const Pos& hiderPos);

    /*!
     * \brief fixSeekersNotToBeClose ensures that the seeker positions are not too close (SeekerHSParams.minDistBetweenRobots)
     * \param nextPos1In suggested pos for this seeker
     * \param nextPos2In suggested goal pos for other seeker
     * \param nextPos1Out [out] fixed goal pos for this seeker
     * \param nextPos2Out [out] fixed goal pos for other seeker
     */
    void fixSeekersNotToBeClose(const Pos& nextPos1In, const Pos& nextPos2In, Pos& nextPos1Out, Pos& nextPos2Out);

    /*!
     * \brief calcExplorationEvaluation calculate the exploration value, based on
     *          Burgard, W., Moors, M., Stachniss, C., & Schneider, F. E. (2005). Coordinated multi-robot exploration.
     *          IEEE Transactions on Robotics, 21(3), 376–386. http://doi.org/10.1109/TRO.2004.839232
     *
     * which we have adapted to:
     *      U_t - beta V_ts + gamma b_t
     *
     * where U is the utility which is based on whether it was already chosen as goal before, or if it was close (in the visibility range) of a
     * chosen goal; beta (SeekerHSParams.multiSeekerExplorerDistWeight) is the weight of the robot-highest belief distance (V);
     * gamma (multiSeekerExplorerBeliefWeight) the weight of the belief point t.
     *
     * Complexity: (AG150609)
     * O(highestBeliefPosVec.size) if we assume the distances have been precalculated (only lookup),
     * othwerwise: O(highestBeliefPosVec*O_distance)
     *
     * \param seekerPos the position of the seeker for which we want to find a target
     * \param highestBeliefPosVec list of positions with highest beliefs, it is assumed that the list is ordered from high to low belief.
     * The first item always should contain a value.
     * \param uVec [in|out] the utility (see Burgard et al. 2005) for each of the targets (highestBeliefPosVec); the initial value for
     *  each target should be 1; if updateU is true the algorithm will update the values.
     * \param updateU indicates if the U vector has to be updated (default true, but if it is the last goal to assign
     * \return index of the highest belief for the selected robot (for pos and belief)
     */
     virtual int calcExplorationEvaluation(const Pos& seekerPos, const std::vector<BPos>& highestBeliefPosVec, std::vector<double>& uVec,
                                  bool updateU = true /*const std::vector<double>& highestBeliefVec,
                                     Pos* otherRobotGoal*/);

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



    //! autoplayer that has belief
    PlayerInfo* _otherSeekerPlayer;

    //AG150609: has to be stored from when the observations are read to calculate the goals, to the step where
    // all goals have been received and the final goal has to be selected
    //! chosen hider pos
    Pos _chosenHiderPos;
    //! was the chosen hider pos consistent, if not don't use it
    bool _chosenHiderPosConsist;

    //indices of this seeker, and other seeker in the multiGoalPosesVec/multiGoalBeliefVec
    static std::size_t MULTI_SEEKER_VEC_OWN_INDEX;
    static std::size_t MULTI_SEEKER_VEC_OTHER_INDEX;

    //AG160725
    //! score matrix
    PosScoreMat* _posScoreMat;


};

#endif // TWOSEEKERHBEXPLORER_H
