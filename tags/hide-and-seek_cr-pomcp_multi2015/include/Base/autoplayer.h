#ifndef AUTOPLAYER_H
#define AUTOPLAYER_H

#include "HSGame/gmap.h"
#include <string>
#include "seekerhsparams.h"
#include "Base/playerinfo.h"

#ifndef DO_NOT_WRITE_BELIEF_IMG
//only for painting belief (todo: to global class)

//AG151118: made compatible to OpenCV 3
//(see http://docs.opencv.org/master/db/dfa/tutorial_transition_guide.html)
#include "opencv2/core/version.hpp"
#if CV_MAJOR_VERSION == 2
#include "opencv2/core/core.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#elif CV_MAJOR_VERSION == 3
#include "opencv2/core.hpp"
//#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
//#else:fail
#endif

#endif

//TODO: maybe later split AutoPlayer to AutoPlayer2 where only functions for 2 seekers are present
/*!
 * This interface is required for automated seekers. Used by both the simulator (through the GameConnector) and the real robot interface (SeekerHS).
 * AG150521:
 * - The initialization (of the belief) is done by: initBelief()
 *   When multiple players are present: initBeliefMulti()
 * - the next step is to get the next poses: getNextPos()
 *
 * The subclasses should override the functions: initBeliefRun() and getNextPosRun()
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
    //virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible)=0;
    //AG150519: depricated -> now protected virtual initBelief() called by initBeliefMulti(...)


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
    //virtual bool initBelief2(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible, Pos seeker2InitPos, Pos hiderObs2InitPos, double obs1p);

    /*!
     * \brief initBelief Initialize belief with the GMap and the player info objects which contain the current pos of all players, and the hider pos as
     * they are (to be) observed by the seeker.
     * The player id also indicates the index of the player object in the vector which contains all information of the current player.
     * Note: by default this function is empty, to prevent having to include them in other solvers which do not use multiple seekers
     * Note2: seeker player should already be set
     * \param gmap
     * \param otherPlayer
     * \return
     */
    virtual bool initBelief(GMap* gmap, /*PlayerInfo* seekerPlayer,*/ PlayerInfo* otherPlayer);

    /*! Initialize belief with the GMap and the player info objects which contain the current pos of all players, and the hider pos as
     * they are (to be) observed by the seeker.
     * The player id also indicates the index of the player object in the vector which contains all information of the current player.
     * Note: by default this function is empty, to prevent having to include them in other solvers which do not use multiple seekers
     * \brief initBeliefMulti
     * \param gmap the map
     * \param playerVec the vector of player information objects
     * \param thisPlayerID the id of this player
     * \param hiderPlayerID the id of the hider
     * \return
     */
    virtual bool initBeliefMulti(GMap* gmap, std::vector<PlayerInfo*> playerVec, int thisPlayerID, int hiderPlayerID);


    //AG150521: all getNextAction/getNextPos depricated by this getNextPos
    //now all meta data is assumed to be updated in the local playerInfo

    /*!
     * \brief getNextPos get the next position as calculated by the algorithm. It assumes that the new positions are updated
     * in the playerInfo struct. The actionDone indicates the last action done, or it uses the internal known last action
     * if it was set to -1. Note that some algorithms require the last action done to update their belief (e.g. POMCP). The
     * last action done be calculated by deduceAction().
     * The new action done is also returned.
     * \param actionDone (optional) the real last done action by the agent (not used if -1)
     * \param newAction (optional) [out] the action to do
     * \return the new position
     */
    virtual Pos getNextPos(int actionDone=-1, int* newAction=NULL);


    /*!
     * Get next best action to do, based on the hider pos, seeker pos and visibility of the hider
     * \brief getNextAction
     * \param visible is the hider visible to the seeker
     * \param hiderPos hider position
     * \param seekerPos seeker position
     * \param actionDone the action really done
     * \return
     */
    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone);

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
    //virtual int getNextAction2(Pos seekerPos, Pos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p, int actionDone);

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
    //virtual std::vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone, int n);


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
    //virtual double getNextDirection(Pos seekerPos, Pos hiderPos, bool opponentVisible, bool &haltAction);

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
    //virtual Pos getNextPos(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

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
    //virtual Pos getNextPos2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos,
    //                double obs1p, std::vector<int> &actions, int actionDone=-1, int n=1);

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
    //virtual bool getNextRobotPoses2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, std::vector<int> &actions,
     //                               std::vector<Pos>& goalPosesVec, int actionDone=-1, int n=1, std::vector<double>* goalPosesBelVec=NULL);

    //virtual bool calcMultiRobotGoals


    //AG150522: depricated prev function to:
    /*!
     * \brief calcNextRobotPoses2 calculate the next robot poses for the highest belief 2 seeker solver.
     * It stores the goal poses in playerInfo.multiGoalPosesVec and their belief in playerInfo.multiGoalBeliefVec.
     * \param actionDone
     * \return ok or not
     */
    virtual bool calcNextRobotPoses2(int actionDone=-1);


    /*!
     * \brief getNextHBList get next highest belief list: points and beliefs for the multi seeker find-and-follow. It
     * stores the highest belief points in playerInfo.multiHBPosVec and the belief in playerInfo.multiHBBeliefVec.
     * \param actionDone (optional) real action done by the agent
     * \return ok or not
     */
    virtual bool calcNextHBList(int actionDone=-1);

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
    //virtual Pos selectRobotPos2(Pos* otherSeekerPos1, Pos* otherSeekerPos2, double otherSeekerPos1B, double otherSeekerPos2B,
    //                            int n, Pos* chosenPos=NULL);

    /*!
     * \brief selectRobotPos2 select the robot pos for the 2 robot HB find-and-follow. The nextPos should be set, and the
     * multiChosenGoalPos should indicate the final goal.
     * \return the next pos
     */ //AG150605: should  use selectRobotPosMulti
    //virtual Pos selectRobotPos2();

    /*!
     * \brief selectRobotPosMulti calculate the next positions for all players, and choose the own. The nextPos should be set, and the
     * multiChosenGoalPos should indicate the final goal.
     * \return
     */
    virtual Pos selectRobotPosMulti();


    //AG150521: depricated, not used anymore, only getnextPos
    /*!
     * \brief useGetAction if true the function getNext(Multiple)Action(s) is used, otherwise getNextDirection
     * \return
     */
    //virtual bool useGetAction() const;


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
    //virtual void setRandomPosDistToBase(int d);

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
    bool storeMapBeliefAsImage(std::string file, /*const Pos& seekerPos, const Pos* seeker2Pos, const Pos& hiderPos,
                               const Pos* hiderObsPos2,*/ int cellWidth=20);


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

    //! paint cross with certain color on image
    void paintCrossOnImage(cv::Mat* image, const Pos& p, const cv::Scalar& color, double origCellSize, double radiusPart=1.0);

    /*!
     * \brief getMapBeliefAsImage get the belief on an image of the map
     * \param seekerPos seeker pos
     * \param seekerPos2 seeker 2 pos (NULL if not used)
     * \param hiderPos hider pos if visible
     * \param hiderObsPos2 observation of hider by seeker 2
     * \param cellWidth cell width in pixels
     * \return
     */
    cv::Mat* getMapBeliefAsImage(/*const Pos& seekerPos, const Pos* seeker2Pos, const Pos& hiderPos, const Pos* hiderObsPos2,*/ int cellWidth=20);
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
    int deduceAction(const Pos& fromPos, const Pos& toPos);

    /*!
     * \brief deduceAction deduce action from last position
     * \param seekerPos
     * \return
     */
    //int deduceAction(const Pos& seekerPos);

    /*!
     * \brief deduceAction deduce action based on angle, it does not return the action halt
     * \param ang angle in rad
     * \return
     */
    int deduceAction(double ang);

    /*!
     * \brief deduceAction deduce the action done, using the playerInfo struct from previousPos to currentPos
     * \return
     */
    int deduceAction();

    /*!
     * \brief getLastAction last action generated
     * \return
     */
    //virtual int getLastAction();

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


    //AG150527: depricated, use chekandFilterPoses, then getNextPos
    /*!
     * \brief getNextPosWithFilter get the next position, but first filtering the possible hider positions
     * \param seekerPos
     * \param hiderPos
     * \param actions
     * \param actionDone
     * \param n
     * \return
     */
    /*virtual Pos getNextPosWithFilter(Pos seekerPos, std::vector<IDPos> hiderPosVector, std::vector<int> &actions,
                                     int actionDone, int n, bool& dontExecuteIteration);*/

    /*!
     * \brief checkAndFilterPoses checks the positions and fixes them, and chooses the hider pos.
     * Note: it uses previousPos!
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
    //virtual bool initBeliefWithFilter(GMap* gmap, Pos seekerInitPos, std::vector<IDPos> hiderPosVector/*, int* chosenHiderPosIndex=NULL*/);

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
    //IDPos getLastHiderPos() const;

    /*!
     * \brief getLastSeekerPos get last used seeker pos
     * \return
     */
    //Pos getLastSeekerPos() const;

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


    //AG150515
    /*!
     * \brief set playerinfo
     * \param id
     */
    //void setPlayerInfo(PlayerInfo* playerInfo);

    /*!
     * \brief get
     * \return
     */
    //PlayerInfo* getPlayerInfo();

    /*!
     * \brief getChosenHiderPos get chosen hider pos as parameter, returns whether the chosenHider Pos was consistent
     * with other hider positions readings.
     * \param chosenHiderPos [out] the chosen hider pos
     * \return is this pos consistent
     */
    virtual bool getChosenHiderPos(Pos& chosenHiderPos) const;

    //AG150521
    //! contains all meta data of the player
    PlayerInfo playerInfo;




protected:
    /*!
     * \brief initBeliefRun this function should be overriden by subclasses to initialize the belief.
     * It can use the local struct playerInfo which contains the current (currentPos) location and hider observation, and the other observations: _playerInfoVec.
     * \return
     */
    virtual bool initBeliefRun()=0;

    /*!
     * \brief getNextPosRun this function should be overriden by subclasses to calculate the new position and action.
     * It can use the local struct playerInfo which contains the location and hider observation, and the other observations: _playerInfoVec.
     * \param actionDone
     * \param newAction
     * \return
     */
    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);

    /*!
     * \brief getNextDirection can be overriden by the subclass and is used if the getNextPosRun function is not implemented
     * \param haltAction
     * \return
     */
    virtual double getNextDirection(bool &haltAction);

    /*!
     * \brief calcNextRobotPoses2Run function run by calcNextRobotPoses2, should be implemented by subclass to use it
     * \param actionDone
     * \return
     */
    virtual bool calcNextRobotPoses2Run(int actionDone=-1);


    /*!
     * \brief calcNextRobotPoses2Run function run by calcNextRobotPoses2, should be implemented by subclass to use it
     * \param actionDone
     * \return
     */
    virtual bool calcNextHBListRun(int actionDone=-1);


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
    //virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

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
    //virtual Pos getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos,
    //                           double obs1p, std::vector<int> &actions, int actionDone=-1, int n=1);

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
    //int _randomPosDistToBase; //AG150521: is in params

    //AG150520: depricated b _thisPlayer.lastAction
    //AG140410: last action to send to server
    //! last action to send to server
    //int _lastAction;

    //AG150520: depricated by _thisPlayer.currentPos / _hiderPlayer.currentpos
    //! last pos of seeker (used by filter and check)
    //Pos _lastSeekerPos;
    //! last pos of hider (used by filter and check)
    //IDPos _lastHiderPos;

    //! number of iterations skipped (because bad input)
    unsigned int _numIterationsSkipped;

    //! number of iterations
    unsigned int _numIterations;

    //AG150519
    //! Player info vector of all players
    std::vector<PlayerInfo*> _playerInfoVec;

    //! id of this player, and index of the player info object in the vector
    int _thisPlayerID;

    //! id of hider player, and index of the player info object in the vector
    int _hiderPlayerID;

    // pointer to this player's info
    //PlayerInfo _thisPlayer;

    //! pointer to hider's player info
    //! WARNING this should not be used by the algorithm directly, instead it should use the
    PlayerInfo* _hiderPlayer;

    //! pointer to seeker player (asuming there is only 1, otherwise refers to first)
    PlayerInfo* _seekerPlayer1;

protected:
    //! check if the sum of obs prob. over all seekers is 1
    virtual bool checkSumObsProbIs1();

};


#endif // AUTOPLAYER_H
