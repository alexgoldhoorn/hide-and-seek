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
#else
#error "OpenCV 2 or 3 required"
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
     * \brief selectRobotPosMulti calculate the next positions for all players, and choose the own. The nextPos should be set, and the
     * multiChosenGoalPos should indicate the final goal.
     * \return
     */
    virtual Pos selectRobotPosMulti();


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
     * \param cellWidth cell width in pixels
     * \return
     */
    bool storeMapBeliefAsImage(std::string file, int cellWidth=20);


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
     *
     * \param cellWidth cell width in pixels
     * \return
     */
    cv::Mat* getMapBeliefAsImage(int cellWidth=20);
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

    /*!
     * \brief getChosenHiderPos get chosen hider pos as parameter, returns whether the chosenHider Pos was consistent
     * with other hider positions readings.
     * \param chosenHiderPos [out] the chosen hider pos
     * \return is this pos consistent
     */
    virtual bool getChosenHiderPos(Pos& chosenHiderPos) const;

    //AG160129
    /*!
     * \brief getParams return the parameters object
     * \return
     */
    SeekerHSParams* getParams() const;

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
