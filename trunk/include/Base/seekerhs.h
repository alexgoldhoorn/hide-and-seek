#ifndef SEEKERHS_H
#define SEEKERHS_H

#include <string>
#include <vector>
#include <map>
#include <ostream>

#include "Utils/timer.h"

//predefine classes
//class AutoPlayer;
//class GMap;
//class Player;
//class HSLog;

#ifndef DO_NOT_USE_MOMDP
class Segmenter;
#endif
//class Pos;

#include "Base/autoplayer.h"
#include "HSGame/gmap.h"
//#include "HSGame/gplayer.h"

#include "Base/seekerhsparams.h"
#include "Base/posxy.h"

#include "Base/seekerhslog.h"

#include "mutex.h"


/*!
 * \brief The SeekerHS class runs the hide and seek game using a MOMDP/POMDP.
 *
 *The code requires an initialization:
 *  - map
 *  - params ..
 * Then on each step:
 * - get an action
 * - give the observation: position of the robot and the opponent (if visible)
 *
 * Positions are pased as a vector of double representing: x,y,(orientation)
 * Since the hider's position can be unknown, an empty position vector should be passed if this is the case.
 *
 * The win state is passed through the getNextPose function, but since the robot does not always know the state of the hider,
 * the function stopGame can be called externally with a winState. If this winState is PLAYING this means that the game has been cancelled
 * (i.e. not finished).
 *
 *  Robot axis:
 *
 *         __
 *        (..)
 *   X <---o Z (out of screen)
 *         |
 *        \/ Y
 *
 * thus orientation is anti-clockwise
 *
 */
class SeekerHS
{
public:
    /*!
     * \brief SeekerHS Initialize the autoplayer with the policy and pomdp and logfiles
     * \param mapFile map file
     * \param expName experiment name
     * \param solverType offline (requires policy file) or online
     * \param pomdpFile
     * \param policyFile
     * \param logFile
     * \param timeLogFile
     */
    SeekerHS(std::string mapFile, std::string expName, int solverType, std::string pomdpFile, std::string policyFile,
             std::string logFile, std::string timeLogFile, std::string gamelogFile);

    /*!
     * \brief SeekerHS Initialize the seeker with the params.
     * \param params
     * \param genMap (optional, default true) generate the map, otherwise it should
     * \param seekerID (optional) number of seeker, useful when several seekers are treated in this instance
     */
    SeekerHS(SeekerHSParams* params, bool genMap=true, int seekerID=0);


    ~SeekerHS();

    /*!
     * \brief init initialize with the start positions of the seeker and hider (if known).
     * \param seekerPosXY seeker position
     * \param hiderObsPosXY observation of hider
     */
    void init(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec);


    /*!
     * \brief initMultiSeeker initializes the solver.
     * \param seekerPosXY seeker position
     * \param hiderObsPosXY observation of hider
     * \param dynObstPosXYVec dyn. obst. vector
     * \param seekerOtherPosXYVec other seeker's position
     * \param hiderObsOtherPosXYVec other seeker's observation of hider
     * \param otherDynObstPosXYVec dyn. obst. vector of other seeker
     */
    void initMultiSeeker(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec,
                         const std::vector<PosXY>& seekerOtherPosXYVec, const std::vector<PosXY>& hiderObsOtherPosXYVec,
                         const std::vector<std::vector<PosXY>>& otherDynObstPosXYVecVec);


    /*!
     * \brief filterMultipleObs Filters the seekerpos and multiple hider poses. Returns the one seeker and one filtered hider pos.
     * All vectors have only two coordinates, and no orientation.
     * \param seekerPosXYIn seeker position
     * \param hiderPosXYVecIn hider observations vector
     * \param seekerPosXYOut [out] filtered seeker position
     * \param hiderPosXYOut [out] filtered hider position
     * \param dontExec [out] indicates if a consistent hider obs has been found, and therefore the iteration can be executed
     */
    void filterMultipleObs(const PosXY& seekerPosXYIn, const std::vector<PosXY>& hiderPosXYVecIn, PosXY& seekerPosXYOut,
                           PosXY& hiderPosXYOut, bool& dontExec);

    /*!
     * \brief getNextPos get the next position and pose of the user.
     * \param seekerPosXY seeker position
     * \param hiderObsPosXY hider observation
     * \param dynObstPosXYVec vector of dynamical obstacles (can include the hider) (?)
     * \param newSeekerPosXY [out] new seeker position
     * \param winState [out] win state
     * \param dontExecuteIteration [in,optional] don't execute the iteration (i.e. returns current pos as goal)
     * \param resetItStartTime [in,optional] reset timer (only internal use, default: true)
     * \return the action @see HSGlobalData
     */
    int getNextPos(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec, PosXY& newSeekerPosXY,
                   int* winState, bool *dontExecuteIteration=NULL, bool resetItStartTime=true);

    /*!
     * \brief calcNextMultiRobotPoses2 Step 1 for TwoSeekerHBExplorer: get the next poses for multiple (2 seekers).
     * The input should be already filtered ('mine')
     * using filterMultipleObs(), the 'other' poses should be received from the other seeker, or NULL if no information (e.g.
     * no connection). The other seeker's data should not be 'too old'.
     * \param seekerPosXY this seeker's position (filtered+checked)
     * \param hiderObsPosXY the (filtered) hider position as seen by this seeker
     * \param dynObstPosXYVec vector of dynamical obstacles (can include the hider) (?)
     * \param seekerOtherPosXY the other's seeker (own filtered) position (NULL if no information)
     * \param hiderObsOtherPosXY the other's seeker (own filtered) position (NULL if no information)
     * \param otherDynObstPosXYVec vector of dynamical obstacles of other
     * \param newSeekerPosMine [out] new seeker goal position for this seeker
     * \param newSeekerPosOther [out] idem for other seeker (can be empty if no information)
     * \param winState [out] the win state (indicates if hider/seeker won)
     * \param dontExecuteIteration [in,optional] don't execute iteration (set by filter)
     * \param resetItStartTime [in,optional] reset timer (only internal use, default: true)
     * \return
     */
    bool calcNextMultiRobotPoses2(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec,
                                  const PosXY* seekerOtherPosXY, const PosXY* hiderObsOtherPosXY, const std::vector<PosXY>* otherDynObstPosXYVec,
                                  PosXY& newSeekerPosMine, PosXY& newSeekerPosOther, // contains the belief
                                  int* winState, bool* dontExecuteIteration=NULL, bool resetItStartTime=true);

    /*!
     * \brief select2SeekerPose Step 2 of TwoSeekerHBExplorer: select the seeker position from the goals given by this seeker and
     * the other seeker. If the other seeker's goal for this seeker is not received, the goal of this seeker is chosen.
     * \param newSeekerPosXYMineFromOther goal for this seeker from other (NULL if not set or not received)
     * \param newSeekerPosXYOtherFromOther goal for other seeker from other (NULL if not received)
     * \param newSeekerPosXYMine [out] goal for this seeker
     * \return ok
     */
    bool select2SeekerPose(const PosXY* newSeekerPosXYMineFromOther, const PosXY* newSeekerPosXYOtherFromOther,
                               PosXY& newSeekerPosXYMine);


    /*!
     * \brief calcMultiHBs Step 1 of MultiSeekerHBExplorer: calculate the highest belief points.
     * \param seekerPosXY seeker position
     * \param hiderObsPosXY hider observation
     * \param dynObstPosXYVec vector of dynamical obstacles (can include the hider) (?)
     * \param seekerOtherPosXY other seeker's position (NULL if not set)
     * \param hiderObsOtherPosXY other seeker's observation of hider (NULL if not set)
     * \param otherDynObstPosXYVec vector of dynamical obstacles (can include the hider) (?)
     * \param hbVec [out] highest belief vector
     * \param winState [out] the win state (indicates if hider/seeker won)
     * \param dontExecuteIteration [in,optional] don't execute iteration (set by filter)
     * \param resetItStartTime [in,optional] reset timer (only internal use, default: true)
     * \return
     */
    bool calcMultiHBs(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec,
                     const PosXY* seekerOtherPosXY, const PosXY* hiderObsOtherPosXY,
                     const std::vector<PosXY>* otherDynObstPosXYVec, std::vector<PosXY>& hbVec,
                     int* winState, bool* dontExecuteIteration=NULL, bool resetItStartTime=true);


    //select pose using multi HBs
    /*!
     * \brief selectMultiSeekerPoseFromHB Step 2 of MultiSeekerHBExplorer: select the multi seeker pose using the highest beleifs.
     * \param otherHBVec other seeker's highest belief vector (can be empty if no communication)
     * \param newSeekerPosXYMine [out] new seeker position
     * \return
     */
    bool selectMultiSeekerPoseFromHB(const std::vector<PosXY>& otherHBVec, PosXY& newSeekerPosXYMine);

    /*!
     * \brief getAutoPlayer return auto player
     * \return
     */
    AutoPlayer* getAutoPlayer();

#ifndef DO_NOT_USE_MOMDP
    /*!
     * \brief getSegmenter
     * \return
     */
    Segmenter* getSegmenter();

    /*!
     * \brief getSegmenterX
     * \return
     */
    Segmenter* getSegmenterX();
#endif

    /*!
     * \brief setMap
     * \param map
     */
    void setMap(GMap* map);

    /*!
     * \brief getMap
     * \return
     */
    GMap* getMap();

#ifdef OLD_CODE
    /*!
     * \brief setWinDistToHider set the distance to hider from which the seeker wins
     * \param winDist
     *
     * Default in simulation: 0
     * In real-world: >= 1
     */
    void setWinDistToHider(double winDist);

    /*!
     * \brief getWinDistToHider get the distance to hider from which the seeker wins
     * \return
     */
    int getWinDistToHider();

    /*!
     * \brief setCellSizeM set cell width in the real world (meters)
     * \param cellSizeM
     */
    void setCellSizeM(double cellSizeM);

    /*!
     * \brief getCellSizeM cell width in the real world (meters)
     * \return
     */
    double getCellSizeM();
#endif

    /*!
     * \brief stopGame stop the game
     * \param winState
     */
    void stopGame(int winState);

    /*!
     * \brief getActionsDone the number of actions done
     * \return
     */
    unsigned int getActionsDone();

    /*!
     * \brief isObstacle check if there is an obstacle on this location of the map
     * \param posVector
     * \return
     */
    //bool isObstacle(const std::vector<double>& posVector);

    bool isObstacle(const PosXY& posXY);


    /*!
     * \brief getWinState get the win state based on the current positions of the hider and seeker
     * \param seekerPos
     * \param hiderPos
     * \param hiderVisible
     * \return
     */
    int getWinState(const Pos& seekerPos, const Pos& hiderPos, bool hiderVisible);

    /*!
     * \brief getParams get params
     * \return
     */
    SeekerHSParams* getParams();

    PosXY getChosenHiderPos();


#ifndef DO_NOT_WRITE_BELIEF_IMG
    /*!
     * \brief getMapBeliefAsImage get the OpenCV object with the image representing the belief
     * \return
     */
    cv::Mat* getMapBeliefAsImage();
#endif

    /*!
     * \brief hasTimePassedForNextIteration returns whether a minimum amount of time has passed since last run (params.minTimeBetweenIterations_ms)
     * \return
     */
    bool hasTimePassedForNextIteration();

    /*!
     * \brief getOrientation get the orientation of pos1 wrt pos2
     * \param pos1
     * \param pos2
     * \return radians between -pi and pi
     */
    static double getOrientation(const Pos& pos1, const Pos& pos2);

    /*!
     * \brief orientDiff difference between orientation
     * \param or1
     * \param or2
     * \return
     */
    static double orientDiff(double or1, double or2);

    /*!
     * \brief getPlayerInfo get player info based on id
     * \param id
     * \return
     */
    const PlayerInfo* getPlayerInfo(int id);

protected:
    //used in chooseHider to calculate score based on distance
    static constexpr double DIST_SCORE_MULT = 2;
    static constexpr double MAX_DIST_SCORE = 5;

    //! change the order of row/col sent and receive	//! change the order of row/col sent and receivedd
    //AG140124
    /*static const int ROW_INDEX = 0; //1; was switched (1) for experimetn 140124
    static const int COL_INDEX = 1; //0;*/
    static const bool CHANGE_ORIENTATION = false;

    // AG160614: added seekerNum
    /*!
     * \brief init init
     * \param params
     * \param genMap generate map, otherwise should be set
     * \param seekerID seeker id
     */
    void initClass(SeekerHSParams* params, bool genMap, int seekerID);

    //! init player info
    void initPlayerInfo();

    /*!
     * \brief addPlayerInfoToMap add playerInfo to map,
     * \param playerInfo
     */
    void addPlayerInfoToHashMap(PlayerInfo* playerInfo);

    /*!
     * \brief seekerWins seeker wins
     * \param seekerPos
     * \param hiderPos
     * \return
     */
    inline bool seekerWins(const Pos& seekerPos, const Pos& hiderPos);

    /*!
     * \brief isThereAnObstacleAt is there an obstacle at dr,dc relative to p ? i.e. -1,0 is cell at the left
     * \param p
     * \param dr
     * \param dc
     * \return whether there is an obstacle at the min obstacle distance
     */
    bool isThereAnObstacleAt(Pos p, double dr, double dc);


    /*!
     * \brief setPlayerInfoFromRobotPoses set the player info from the seeker pos and hider pos from the robot
     * it converts the positions. The hider pos is stored in the playerInfo.hiderObsWNoise, the seeker pos in playerInfo.currentPos.
     * \param playeInfo
     * \param seekerPos
     * \param hiderPos
     * \param dynObstPosXYVec
     */
    void setPlayerInfoFromRobotPoses(PlayerInfo* playerInfo, const PosXY& seekerPosXY, const PosXY* hiderPosXY, const std::vector<PosXY>* dynObstPosXYVec);

    //AG150623
    //! prepares the playerInfo structs for the next step (clears some members)
    //! should be called before starting a new step
    void prepareNextStep();

    //AG150623
    //! Calculate the orientation based on the next pos and the chosen hider pos. If the game is not running anymore
    //! the current pos is used
    double calcNextOrientation();

    //AG150623
    //! updates local belief map image (to be retrieved by getMapBeliefAsImage()
    void updateBeliefImg();

private:
    //! starts or restarts algo timer
    void startAlgoTimer();

    static double ang(double y, double x);

    //! the autoplayer
    AutoPlayer* _autoPlayer;

    //! the map
    GMap* _map;

    //AG150618: store the player info of all players
    //! player info of this player
    PlayerInfo* _thisPlayerInfo;
    //! player info of other seeker players (i.e. not this player, nor hider, all hider poses are stored in hiderObsPos field)
    std::vector<PlayerInfo> _otherPlayerInfoVec;
    //! hider player (used because required by AutoPlayer)
    PlayerInfo _hiderPlayerInfo;
    //! map of all playerInfo on id
    std::map<int,PlayerInfo*> _playerInfoMap;

    //! game log
    SeekerHSLog _gameLog;

    //! params
    SeekerHSParams* _params;

    #ifndef DO_NOT_USE_MOMDP
    //! segmenter Y-space
    Segmenter* _segmenter;
    //! segmenter X-space
    Segmenter* _segmenterX;
    #endif

    //AG150622: depricated by playerInfo.numberActions
    //! coordinates of origin of external (robot's) coordinate reference in our map coordinates
    PosXY _extOriginPosXY;

    //! coordinates of origin of my/map coordinate system in external coordinates
    PosXY _myOriginPosXY;

    //! timer for meta timing info
    Timer _timer;
    int _timerID;

    //! the chosen hider pos
    PosXY _chosenHiderPosXY;

#ifndef DO_NOT_WRITE_BELIEF_IMG
    //! belief image, used by ROS
    cv::Mat* _beliefImg;

    //! mutex for belief img
    CMutex _beliefImgMutex;
#endif
    //! timer ID for 'fixing' max freq of running algorithm
    int _algoTimerID;

    double _lastOrient;

    int _winState;

};

#endif // SEEKERHS_H
