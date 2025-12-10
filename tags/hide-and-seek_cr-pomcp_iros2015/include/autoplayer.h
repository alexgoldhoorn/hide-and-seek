#ifndef AUTOPLAYER_H
#define AUTOPLAYER_H

#include "HSGame/gmap.h"
#include <string>
#include "seekerhsparams.h"

#ifndef DO_NOT_WRITE_BELIEF_IMG
//only for painting belief (todo: to global class)
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#endif

//TODO: maybe later split AutoPlayer to AutoPlayer2 where only functions for 2 seekers are present
/*!
 * This interface is required for automated seekers. Used by GameConnector to get the seeker's actions.
 *
 * \brief The AutoPlayer class
 */
class AutoPlayer {
public:

    AutoPlayer(SeekerHSParams* params);

    AutoPlayer(SeekerHSParams* params, GMap* map);

    virtual ~AutoPlayer();


    //AG130320: init hider and seeker pos
    /*! Init belief with the GMap it assumes that the gmap alread contains: initial positions of seeker and hider, and the visibility map of the seeker.
     * hiderInitPos can contain the real hider's position, even though it is not visible to the seeker. The boolean opponentVisible indicates the
     * visibility.
     * \brief initBelief
     * \param gmap  the GMap
     * \param seekerInitPos seeker init position
     * \param hiderInitPos hider position
     * \param opponentVisible visibility of hider as seen by seeker
     * \return
     */
    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible)=0;

    //AG15022
    /*! Initialize belief with the GMap and the positions of the seeker and seeker2. hiderInitPos indicates the position of the hider, and can be
     * set even though it is not visible to the seeker, the last is indicated by opponentVisible. seeker2InitPos is the position of the other seeker,
     * and hiderObs2InitPos is the observation of the hider's position by seeker 2.
     * Note: by default this function is empty, to prevent having to include them in other solver which do not use two seekers.
     * \brief initBelief2
     * \param gmap
     * \param seekerInitPos
     * \param hiderInitPos
     * \param opponentVisible
     * \param seeker2InitPos
     * \param hiderObs2InitPos
     * \param obs1p probability of choosing hiderPos (instead of hiderObs2)
     * \return
     */
    virtual bool initBelief2(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible, Pos seeker2InitPos, Pos hiderObs2InitPos, double obs1p);

    /*!
     * Get next best action to do, based on the hider pos, seeker pos and visibility of the hider
     * \brief getNextAction
     * \param visible is the hider visible to the seeker
     * \param hiderPos hider position
     * \param seekerPos seeker position
     * \param actionDone the action really done
     * \return
     */
    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone);

    //AG150122
    /*!
     * \brief getNextAction2 get the next action based on both seeker positions and their observations.
     * Note: by default this function is empty, to prevent having to include them in other solver which do not use two seekers.
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param seeker2Pos
     * \param hiderObs2Pos
     * \param obs1p probability of choosing hiderPos (instead of hiderObs2)
     * \param actionDone
     * \return
     */
    virtual int getNextAction2(Pos seekerPos, Pos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p, int actionDone);

    //AG130729
    /*!
     * Get the next multiple actions. Note that the next actions might not be the most optimal, since
     * the position of the hider is not known yet.
     * AG150122: this function is not maintained in the solvers, the main function used is getNextAction(2)
     * \brief getNextMultipleActions
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param actionDone the action really done
     * \param n number of actions required
     * \return list of next n actions
     */
    virtual std::vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone, int n);


    //virtual vector<int> getNextMultipleActions(Pos seekerPos, vector<Pos> seekerPos, int n)=0;

    //AG140325
    /*!
     * \brief getNextDirection gets the next direction (0,2pi)
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param haltAction [out] returns if the action is halt, otherwise the direction is used
     * \return
     */
    virtual double getNextDirection(Pos seekerPos, Pos hiderPos, bool opponentVisible, bool &haltAction);

    /*!
     * \brief getNextPosBasedOnDirection get the next position, n grid cells distance from current position.
     * Note: in this function the last action (_lastAction) is updated, if overwritten, it should be updated there.
     * Note2: this function calls getNextPosRun, which is overwritten by child classes
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param actions the actions returned, can be empty if direction is used
     * \param actionDone the action really done
     * \param n positions (optional, default 1)
     * \return
     */
    virtual Pos getNextPos(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

    //AG150126
    /*!
     * \brief getNextPos2 get the next position(s) for the seeker when there are 2 observations.
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param seeker2Pos
     * \param hiderObs2Pos
     * \param obs1p
     * \param actions
     * \param actionDone
     * \param n
     * \return
     */
    virtual Pos getNextPos2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos,
                    double obs1p, std::vector<int> &actions, int actionDone=-1, int n=1);

    /*!
     * \brief useGetAction if true the function getNext(Multiple)Action(s) is used, otherwise getNextDirection
     * \return
     */
    virtual bool useGetAction();


    /*!
     * \brief getBelief get the belief of a hider's state
     * \param r
     * \param c
     * \return
     */
    virtual double getBelief(int r, int c);

    /*!
     * \brief getBelief get the belief of a hider's state
     * \param pos
     * \return
     */
    double getBelief(const Pos& pos);

    /*!
     * \brief getBeliefZoomMatrix returns the belief matrix with the zoomfactor (of params)
     * should be freed
     * \return
     */
    double** getBeliefZoomMatrix(unsigned int& zrows, unsigned int& zcols);


    /*!
     * \brief tracksBelief does this method track a belief (if false-> getBelief should NOT be used)
     * \return
     */
    virtual bool tracksBelief() const;

    //AG131217
    /*!
     * \brief getInitPos get init pos
     * \return
     */
    virtual Pos getInitPos();

    /*!
     * \brief isSeeker is seeker if true, otherwise hider
     * \return
     */
    virtual bool isSeeker() const=0;

    //AG150126
    /*!
     * \brief handles2Obs indicates of the algorithm handles 2 seeker observation
     * \return
     */
    virtual bool handles2Obs() const;

    /*!
     * \brief getName get name
     * \return
     */
    virtual std::string getName() const=0;

    /*!
     * \brief setMap set the map
     * \param map
     */
    virtual void setMap(GMap* map);

    /*!
     * \brief getMap get the used map
     * \return
     */
    virtual GMap* getMap() const;

    /*!
     * \brief setInitPos set the initial pos
     * \param initPos
     */
    virtual void setInitPos(Pos initPos);

    /*!
     * \brief setRandomPosDistToBase
     * \param d
     */
    virtual void setRandomPosDistToBase(int d);

    /*!
     * \brief canScoreObservations tells whether the auto player can score observations (using scoreObservation())
     * \return
     */
    virtual bool canScoreObservations() const;

    //AG140117: the scoreObservation functions by default are set to be false or throw an exception
    /*!
     * \brief scoreObservation Scores an observation on how 'likely' it is. It is based on the learned policy. (e.g. POMCP)
     * The higher the score, the more likely an observation is. (if canScoreObservations).
     * When the filterScoreType = HSGlobalData::FILTER_SCORE_OLD, then the score does not have any limits, otherwise it should be
     * normalized, where 1.0 is highest.
     * \param seekerPos
     * \param hiderPos
     * \param actionDone (default: not set, -1) the action really done instead of the action planned the last time
     * \return
     */
    virtual double scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone=-1);

    /*!
     * \brief getClosestSeekerObs get the observation of the seeker pos that is closest to the passed position.
     * (if canScoreObservations)
     * \param seekerPos
     * \return
     */
    virtual Pos getClosestSeekerObs(Pos seekerPos);


    /*!
     * \brief getReward get the reward (for pomdps)
     * \return
     */
    virtual double getReward();

    //AG140222
    /*!
     * \brief getMapBeliefProbAsString returns a string with the map and beliefs of the hider (if the auto player tracks belief)
     * \return
     */
    std::string getMapBeliefProbAsString(int rows, int cols, int w = 5);

    /*!
     * \brief storeMapBeliefAsImage stores the belief on a map as image (if tracks belief)
     * \param file file to store image (png)
     * \param seekerPos seeker pos
     * \param seekerPos2 seeker 2 pos (NULL if not used)
     * \param hiderPos hider pos if visible
     * \param hiderObsPos2 observation of hider by seeker 2
     * \param cellWidth cell width in pixels
     * \param file
     * \return
     */
    bool storeMapBeliefAsImage(std::string file, const Pos& seekerPos, const Pos* seeker2Pos, const Pos& hiderPos,
                               const Pos* hiderObsPos2, int cellWidth=20);


#ifndef DO_NOT_WRITE_BELIEF_IMG
    /*!
     * \brief storeImage store image in file
     * \param image
     * \param file
     * \return
     */
    bool storeImage(cv::Mat* image, std::string file);

    //! paint circle with certain color on image
    void paintCircleOnImage(cv::Mat* image, const Pos& p, const cv::Scalar& color, double origCellSize, double radiusPart=1.0);

    /*!
     * \brief getMapBeliefAsImage get the belief on an image of the map
     * \param seekerPos seeker pos
     * \param seekerPos2 seeker 2 pos (NULL if not used)
     * \param hiderPos hider pos if visible
     * \param hiderObsPos2 observation of hider by seeker 2
     * \param cellWidth cell width in pixels
     * \return
     */
    cv::Mat* getMapBeliefAsImage(const Pos& seekerPos, const Pos* seeker2Pos, const Pos& hiderPos, const Pos* hiderObsPos2, int cellWidth=20);
#endif

    /*!
     * \brief getBeliefScore get the score of the belief by calculating the weighted distance to the real position (hiderPos).
     *  The result is: $score = \sum_{b \in B}{ p_b * dist(b,hiderPos) }
     * \param hiderPos
     * \return
     */
    double getBeliefScore(Pos hiderPos);

    /*!
     * \brief deduceAction deduce which action was done
     * \param p1
     * \param p2
     * \return
     */
    int deduceAction(const Pos& p1, const Pos& p2);

    /*!
     * \brief deduceAction deduce action from last position
     * \param seekerPos
     * \return
     */
    int deduceAction(const Pos& seekerPos);

    /*!
     * \brief deduceAction deduce action based on angle, it does not return the action halt
     * \param ang angle in rad
     * \return
     */
    int deduceAction(double ang);

    /*!
     * \brief getLastAction last action generated
     * \return
     */
    virtual int getLastAction();

    //AG140416: dynamic obstacles, which can be 'created' by the autoplayers
    //not realistic, but this is to prevent having to define an autoplayer for each simulated person (that can be a non-hider)
    //NO: should be a speerate class that simulates the movment
    //can be used by an automated hider AND be in the server running
    /*!
     * \brief hasDynamicObstacles is used to check wether this class has dynamic obstacles
     * \return
     */
    //virtual bool hasDynamicObstacles();

    /*!
     * \brief getDynamicObstacles get the current position of all the dynamic obstacles (could be person simulation)
     * \return
     */
    //virtual std::vector<Pos> getDynamicObstacles();

    /*!
     * \brief getNextPosWithFilter get the next position, but first filtering the possible hider positions
     * \param seekerPos
     * \param hiderPos
     * \param actions
     * \param actionDone
     * \param n
     * \return
     */
    virtual Pos getNextPosWithFilter(Pos seekerPos, std::vector<IDPos> hiderPosVector, std::vector<int> &actions,
                                     int actionDone, int n, bool& dontExecuteIteration);

    /*!
     * \brief checkAndFilterPoses check the positions and fix them, and choose hider pos
     * \param seekerPos seeker position as observed
     * \param hiderPosVector hider positions as observed
     * \param seekerPosOut [out] filtered/checked seeker pos
     * \param hiderPosOut [out] filtered/checked hider pos
     * \param dontExecuteIteration [out] outputs that iteration cannot be executed due to bad input
     */
    virtual void checkAndFilterPoses(const Pos& seekerPos, const std::vector<IDPos>& hiderPosVector, Pos &seekerPosOut,
                                     IDPos& hiderPosOut, bool& dontExecuteIteration);


    /*!
     * \brief initBeliefWithFilter Init belief with one of the past possible hider positions
     * \param gmap
     * \param seekerInitPos
     * \param hiderPosVector
     * \param chosenHiderPosIndex pointer to chosen hider pos index
     * \return
     */
    virtual bool initBeliefWithFilter(GMap* gmap, Pos seekerInitPos, std::vector<IDPos> hiderPosVector/*, int* chosenHiderPosIndex=NULL*/);

    /*!
     * \brief chooseHiderPos chooses the hider pos out of a list of possible positions
     * \param seekerPosVector
     * \param hiderPosVector
     * \param checkPrev check the previous positions
     * \param chosenHiderPosIndex pointer to chosen hider pos index
     * \param dontExecuteIteration [out] outputs that iteration cannot be executed due to bad input
     * \return
     */
    IDPos chooseHiderPos(const Pos& seekerPos, const std::vector<IDPos>& hiderPosVector, bool checkPrev, bool& dontExecuteIteration);


    /*!
     * \brief checkValidNextSeekerPos
     * \param seekerNextPos
     * \param checkPrev
     * \return
     */
    bool checkValidNextSeekerPos(const Pos& seekerNextPos, bool checkPrev) const;

    /*!
     * \brief checkValidNextHiderPos
     * \param hiderNextPos
     * \param seekerNextPos
     * \param checkPrev
     * \return
     */
    bool checkValidNextHiderPos(const IDPos& hiderNextPos, const Pos& seekerNextPos, bool checkPrev);

    /*!
     * \brief getLastHiderPos get last used hider pos
     * \return
     */
    IDPos getLastHiderPos() const;

    /*!
     * \brief getLastSeekerPos get last used seeker pos
     * \return
     */
    Pos getLastSeekerPos() const;

    /*!
     * \brief setMinDistanceToObstacle set the minimum distance to the obstacle
     * \param pos
     * \param minCellDist min. cell distance
     */
    void setMinDistanceToObstacle(Pos &pos);

    /*!
     * \brief isThereAnObstacleAt checks if there is an obstacle at the given position p + (dr,dc)
     * \param p
     * \param dr
     * \param dc
     * \return
     */
    bool isThereAnObstacleAt(Pos p, double dr, double dc) const;

    //AG150128: get next robot poses
    /*!
     * \brief getNextRobotPoses get next robot poses for 2 seekers, passing all their observations
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param seeker2Pos
     * \param hiderObs2Pos
     * \param actions
     * \param goalPosesVec [out] the goal poses vector
     * \param actionDone (optional) real action done by the robot
     * \param n
     * \param goalPosesBelVec [out] (optional) vector of belief values for the goal poses
     * \return ok or not
     */
    virtual bool getNextRobotPoses2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, std::vector<int> &actions,
                                    std::vector<Pos>& goalPosesVec, int actionDone=-1, int n=1, std::vector<double>* goalPosesBelVec=NULL);

    /*!
     * \brief selectRobotPos2 select the goal position of the robot based on the previously calculated destination positions for this robot,
     * and the other and the beliefs. The passed parameters should come from the other seeker. The chosenPos can be different than the returned
     * pos in case of simulation (steps).
     * \param otherSeekerPos1 goal position for this robot
     * \param otherSeekerPos2 goal position for itself (the other robot, seeker 2)
     * \param otherSeekerPos2B the belief of pos 1 (in the other's model) (-1 if not available)
     * \param otherSeekerPos2B the belief of pos 2 (in the other's model) (-1 if not available)
     * \param n number of poses (same as getNextPos)
     * \param chosenPos [out] (optional) the position chosen by the algorithm
     * \return the next pos for the robot
     */
    virtual Pos selectRobotPos2(Pos* otherSeekerPos1, Pos* otherSeekerPos2, double otherSeekerPos1B, double otherSeekerPos2B,
                                int n, Pos* chosenPos=NULL);

protected:
    /*!
     * \brief getNextPosRun this function is called by getNextPos. From outside getNext is run. All child classes should overwrite getNextPosRun
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param actions
     * \param actionDone
     * \param n
     * \return
     */
    virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

    //AG150126
    /*!
     * \brief getNextPosRun2 this function is called by getNextPos2 (for 2 seekers). From outside getNext is run. Child classes
     *  should overwrite getNextPosRun2. It is used to update information using two observations.
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param seeker2Pos
     * \param hiderObs2Pos
     * \param obs1p probability of choosing hiderPos (instead of hiderObs2)
     * \param actions
     * \param actionDone
     * \param n
     * \return
     */
    virtual Pos getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos,
                               double obs1p, std::vector<int> &actions, int actionDone=-1, int n=1);

    /*!
     * \brief getNextPosAsStep get the next pos in the goalPos direction of seekerStepSize.
     * \param seekerPos
     * \param goalPos
     * \param n number of steps
     * \param stayAtMinWinDistFromgoal if true the returned pos will be atleast winDist from goal pos
     * \return
     */
    virtual Pos getNextPosAsStep(const Pos& seekerPos, const Pos& goalPos, int n, bool stayAtMinWinDistFromgoal);

    //! GMap (to be set)
    GMap* _map;

    //! the params
    SeekerHSParams* _params;

    //! init pos
    Pos _initPos;

    //! random pos distance to base (to initialize on a pos at a certian distance from the base)
    int _randomPosDistToBase;

    //AG140410: last action to send to server
    //! last action to send to server
    int _lastAction;

    //! last pos of seeker (used by filter and check)
    Pos _lastSeekerPos;
    //! last pos of hider (used by filter and check)
    IDPos _lastHiderPos;

    //! number of iterations skipped (because bad input)
    unsigned int _numIterationsSkipped;

    //! number of iterations
    unsigned int _numIterations;
};


#endif // AUTOPLAYER_H
