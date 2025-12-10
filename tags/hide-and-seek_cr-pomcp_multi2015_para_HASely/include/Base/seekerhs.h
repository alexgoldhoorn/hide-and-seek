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
     */
    SeekerHS(SeekerHSParams* params, bool genMap=true);


    ~SeekerHS();


    //void init(std::vector<double> seekerPosVector, std::vector<double> hiderPosVector);

    /*!
     * \brief init initialize with the start positions of the seeker and hider (if known).
     * \param seekerPosXY seeker position
     * \param hiderObsPosXY observation of hider
     */
    void init(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY);

    //void initMultiSeekerObs(const std::vector<double>& seekerPosVectorMine, const std::vector<double>& hiderPosVectorMine,
   //                         const std::vector<double>& seekerPosVectorOther, const std::vector<double>& hiderPosVectorOther);


    /*!
     * \brief initMultiSeeker initializes the solver.
     * \param seekerPosXY seeker position
     * \param hiderObsPosXY observation of hider
     * \param seekerOtherPosXYVec other seeker's position
     * \param hiderObsOtherPosXYVec other seeker's observation of hider
     */
    void initMultiSeeker(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY,
                            const std::vector<PosXY>& seekerOtherPosXYVec, const std::vector<PosXY>& hiderObsOtherPosXYVec);



    //void filterMultipleObs(const std::vector<double>& seekerPosVectorIn, const std::vector< std::vector<double> >& hiderPosVectorsIn,
    //                       std::vector<double>& seekerPosVectorOut, std::vector<double>& hiderPosVectorOut);

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

    //int getNextPose(std::vector<double> seekerPosVector, std::vector<double> hiderPosVector, std::vector<double>& newSeekerPosVector, int* winState,
    //              bool resetItStartTime=true); //TODO!!

    /*!
     * \brief getNextPos get the next position and pose of the user.
     * \param seekerPosXY seeker position
     * \param hiderObsPosXY hider observation
     * \param newSeekerPosXY [out] new seeker position
     * \param winState [out] win state
     * \param dontExecuteIteration [in,optional] don't execute the iteration (i.e. returns current pos as goal)
     * \param resetItStartTime [in,optional] reset timer (only internal use, default: true)
     * \return the action @see HSGlobalData
     */
    int getNextPos(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, PosXY& newSeekerPosXY, int* winState,
                    bool *dontExecuteIteration=NULL, bool resetItStartTime=true);

    //(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY);


    //std::vector<int> getNextMultiplePoses(std::vector<double> seekerPosVector, std::vector<double> hiderPosVector, int n,
    //                                 std::vector<double>& newSeekerPosVector, int* winState, bool resetItStartTime=true);

    /*std::vector<int> getNextMultiplePosesForMultipleObs(std::vector<double> seekerPosVector, std::vector< std::vector<double> > hiderPosVector,
                                                   int n, std::vector<double>& newSeekerPosVector, int* winState,
                                                        bool* dontExecuteIteration=NULL, bool resetItStartTime=true);*/


    /*bool getNextMultiSeekerPoses(const  std::vector<double>& seekerPosVectorMine, const std::vector<double>& hiderPosVectorMine,
                                 const std::vector<double>& seekerPosVectorOther, const std::vector<double>& hiderPosVectorOther,
                                       int n, std::vector<double>& newSeekerPosVectorMine, std::vector<double>& newSeekerPosVectorOther,
                                        double &newSeekerPosMineBelief, double &newSeekerPosOtherBelief,
                                        int* winState, bool* dontExecuteIteration=NULL, bool resetItStartTime=true);*/

    /*!
     * \brief calcNextMultiRobotPoses2 Step 1 for TwoSeekerHBExplorer: get the next poses for multiple (2 seekers).
     * The input should be already filtered ('mine')
     * using filterMultipleObs(), the 'other' poses should be received from the other seeker, or NULL if no information (e.g.
     * no connection). The other seeker's data should not be 'too old'.
     * \param seekerPosXY this seeker's position (filtered+checked)
     * \param hiderObsPosXY the (filtered) hider position as seen by this seeker
     * \param seekerOtherPosXY the other's seeker (own filtered) position (NULL if no information)
     * \param hiderObsOtherPosXY the other's seeker (own filtered) position (NULL if no information)
     * \param newSeekerPosMine [out] new seeker goal position for this seeker
     * \param newSeekerPosOther [out] idem for other seeker (can be empty if no information)
     * \param winState [out] the win state (indicates if hider/seeker won)
     * \param dontExecuteIteration [in,optional] don't execute iteration (set by filter)
     * \param resetItStartTime [in,optional] reset timer (only internal use, default: true)
     * \return
     */
    bool calcNextMultiRobotPoses2(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY,
                                  const PosXY* seekerOtherPosXY, const PosXY* hiderObsOtherPosXY,
                                  PosXY& newSeekerPosMine, PosXY& newSeekerPosOther, // contains the belief
                                  int* winState, bool* dontExecuteIteration=NULL, bool resetItStartTime=true);

    /* (const PosXY& seekerPosXY, const PosXY& hiderObsPosXY,
                            const std::vector<PosXY>& seekerOtherPosXYVec, const std::vector<PosXY>& hiderObsOtherPosXYVec);
     */


    /*bool selectMultiSeekerPose(const std::vector<double>& newSeekerPosVectorMineFromOther, const std::vector<double>& newSeekerPosVectorOtherFromOther,
                               double newSeekerPosMineBeliefFromOther, double newSeekerPosOtherBeliefFromOther, int n,
                               std::vector<double>& newSeekerPosVector);*/

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
     * \param seekerOtherPosXY other seeker's position (NULL if not set)
     * \param hiderObsOtherPosXY other seeker's observation of hider (NULL if not set)
     * \param hbVec [out] highest belief vector
     * \param winState [out] the win state (indicates if hider/seeker won)
     * \param dontExecuteIteration [in,optional] don't execute iteration (set by filter)
     * \param resetItStartTime [in,optional] reset timer (only internal use, default: true)
     * \return
     */
    bool calcMultiHBs(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY,
                     const PosXY* seekerOtherPosXY, const PosXY* hiderObsOtherPosXY,
                     std::vector<PosXY>& hbVec,
                     int* winState, bool* dontExecuteIteration=NULL, bool resetItStartTime=true);


    //select pose using multi HBs
    /*!
     * \brief selectMultiSeekerPoseFromHB Step 2 of MultiSeekerHBExplorer: select the multi seeker pose using the highest beleifs.
     * \param otherHBVec other seeker's highest belief vector (can be empty if no communication)
     * \param newSeekerPosXYMine [out] new seeker position
     * \return
     */
    bool selectMultiSeekerPoseFromHB(const std::vector<PosXY>& otherHBVec, PosXY& newSeekerPosXYMine);



    //int getNextPoseForMultipleObs(std::vector<double> seekerPosVector, std::vector< std::vector<double> > hiderPosVector,
    //                                               std::vector<double>& newSeekerPosVector, int* winState, bool resetItStartTime=true);
    //std::vector<double> chooseHiderPos(std::vector<double> seekerPosVector, std::vector< std::vector<double> > hiderPosVector, bool checkPrev);
    //AG140513: to AutoPlayer


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

    /*!
     * \brief getChosenHiderPos get the chosen hider pos, hidden if empty
     * \return
     */
    //std::vector<double> getChosenHiderPos();

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
     * \brief setVectorFromPos set position vector from pos struct, assuming that vector contains real-world coordinates, and pos grid coordinates
     * \param posVector [out]
     * \param pos [in]
     * \param orientation [in]
     * \param index [in] index of pose (in vector: i=0: 1st x, i=1: 1st y, i=2: 1st orientation, i=3: 2nd x, .. etc.)
     */
    //void setVectorFromPos(std::vector<double> &posVector, const Pos& pos, double orientation, int index=0, bool setOrientation=true);

    /*!
     * \brief setPosFromVector set pos struct from position vector, assuming that vector contains real-world coordinates, and pos grid coordinates
     * \param pos [out]
     * \param posVector [in]
     */
    //void setPosFromVector(Pos& pos, const std::vector<double> &posVector);

    /*!
     * \brief setPosFromVector set pos struct from position vector, assuming that vector contains real-world coordinates, and pos grid coordinates
     * \param pos [out]
     * \param posVector [in]
     */
    //void setPosFromVector(IDPos& pos, const std::vector<double> &posVector);



    //---TODO HERE, also from Pos??
    /*void setPosFromRobotPosXY(IDPos& pos, const PosXY& robotPosXY);

    void setRobotPosXYFromPos(PosXY& robotPosXY, const IDPos& pos);*/



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


    /*!
     * \brief init init
     * \param params
     */
    void initClass(SeekerHSParams* params, bool genMap);

    //! init player info
    void initPlayerInfo();

    /*!
     * \brief addPlayerInfoToMap add playerInfo to map,
     * \param playerInfo
     */
    void addPlayerInfoToHashMap(PlayerInfo* playerInfo);

    /*!
     * \brief getPosFromVectors returns the pos of the seeker and hider (if visible)
     * \param seekerPosVector
     * \param hiderPosVector
     * \param seekerPos [out]
     * \param hiderPos [out]
     * \param hiderVisible [out]
     */
    //void getPosFromVectors(std::vector<double> &seekerPosVector, std::vector<double> &hiderPosVector, Pos &seekerPos, Pos &hiderPos, bool& hiderVisible);

    /*!
     * \brief getPosFromVectors returns the position of the vector as Pos and the hider positions as vector of IDPos
     * NOTE: it does filter
     * \param seekerPosVectorIn seeker position vector
     * \param hiderPosVectorIn hider position vector of vector
     * \param seekerPosOut [out] seeker position
     * \param hiderPosVectorOut [out] hider positions
     * \param filterHidden [in] do not include hidden (by raytrace) in list of hider positions
     */
    //void getPosFromVectors(const std::vector<double> &seekerPosVectorIn, const std::vector<std::vector<double> > &hiderPosVectorIn,
    //                       Pos &seekerPosOut, std::vector<IDPos> &hiderPosVectorOut, bool filterHidden);



    /*!
     * \brief setExtOriginCoords set the coordinates of the external origin. Our coordinate system is: (rows,cols) but since the rows are used
     * as first parameter, they are used as x, cols are used as y. The rotation is clockwise: how to rotate the external coordinate system to fix it with
     * our map coordinate system.
     * \param x
     * \param y
     * \param rot
     */
    //void setExtOriginCoords(double x, double y, double rot);
    //void setExtOriginCoords(const PosXY& posXY); //AG150623: not used, but if used, should use this struct as param

    /*!
     * \brief seekerWins seeker wins
     * \param seekerPos
     * \param hiderPos
     * \return
     */
    inline bool seekerWins(const Pos& seekerPos, const Pos& hiderPos);

    /*!
     * \brief checkAndFixSeekerPosVector checks if the seekerposvector is ok, if not it is 'fixed' by using the observations or if no the closest pos
     * \param seekerPosVector
     */
    //void checkAndFixSeekerPosVector(std::vector<double>& seekerPosVector);
    //AG140513: to AutoPlayer

    /*!
     * \brief setMinDistanceToObstacle set goal to be at a minimum distance
     * \param pos
     */
    //void setMinDistanceToObstacle(Pos& pos);

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
     */
    void setPlayerInfoFromRobotPoses(PlayerInfo* playerInfo, const PosXY& seekerPosXY, const PosXY* hiderPosXY);

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

    // ! seeker player (to follow)
    //Player* _seekerPlayer;

    //AG150618: depricated by _autoplayer.playerInfo.nextPos
    // ! next pos of seeker (given as action)
    //Pos* _nextSeekerPos;

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

    //AG150624: already in params
    //! experiment name
    //std::string _expName;

    //! params
    SeekerHSParams* _params;

    #ifndef DO_NOT_USE_MOMDP
    //! segmenter Y-space
    Segmenter* _segmenter;
    //! segmenter X-space
    Segmenter* _segmenterX;
    #endif

    //AG150622: depricated by playerInfo.numberActions
    // ! counter of number of actions done
    //unsigned int _numActions;

    //! coordinates of origin of external (robot's) coordinate reference in our map coordinates
    /*double _xExtOrigin, _yExtOrigin;
    double _rotateExtOrigin;*/
    PosXY _extOriginPosXY;

    //! coordinates of origin of my/map coordinate system in external coordinates
    /*double _xMyOrigin, _yMyOrigin;
    double _rotateMyOrigin;*/
    PosXY _myOriginPosXY;

    //! timer for meta timing info
    Timer _timer;
    int _timerID;

    //! the chosen hider pos
    //std::vector<double> _chosenHiderPosVec;
    PosXY _chosenHiderPosXY;

#ifndef DO_NOT_WRITE_BELIEF_IMG
    //! belief image, used by ROS
    cv::Mat* _beliefImg;

    //! mutex for belief img
    CMutex _beliefImgMutex;
#endif
    //! timer ID for 'fixing' max freq of running algorithm
    int _algoTimerID;

    //! last time starting the iterationg (getNext*Action*)
    //struct timeval _tvLastItStartTime;

    //AG140403
    //! last position, used to deduce pos
    //Pos _lastPos;

    double _lastOrient;

    int _winState;

};

#endif // SEEKERHS_H
