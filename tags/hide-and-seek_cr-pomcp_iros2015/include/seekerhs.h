#ifndef SEEKERHS_H
#define SEEKERHS_H

#include <string>
#include <vector>
#include <ostream>

#include "Utils/timer.h"

//predefine classes
//class AutoPlayer;
//class GMap;
class Player;
class HSLog;
#ifndef DO_NOT_USE_MOMDP
class Segmenter;
#endif
//class Pos;

#include "autoplayer.h"
#include "HSGame/gmap.h"
#include "HSGame/gplayer.h"

#include "seekerhsparams.h"

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

    /*!
     * \brief init initialize with the start positions of the seeker and hider (if known)
     * \param seekerPosVector
     * \param hiderPosVector
     */
    void init(std::vector<double> seekerPosVector, std::vector<double> hiderPosVector);

    /*!
     * \brief initMultipleObs
     * \param seekerPosVector
     * \param hiderPosVector
     */
    void initMultipleObs(std::vector<double> seekerPosVector, std::vector< std::vector<double> > hiderPosVector);

    //AG150204: TODO depricate initMultipleObs, ALL should be using filter (external call), and then pass the chosen/filtered
    //          to init, or getnextpos, etc...

    //MULTIS
    /*!
     * \brief initMultiSeekerObs init the seeker
     * \param seekerPosVectorMine
     * \param hiderPosVectorMine
     * \param seekerPosVectorOther
     * \param hiderPosVectorOther
     */
    void initMultiSeekerObs(const std::vector<double>& seekerPosVectorMine, const std::vector<double>& hiderPosVectorMine,
                            const std::vector<double>& seekerPosVectorOther, const std::vector<double>& hiderPosVectorOther);


    //MULTIS
    /*!
     * \brief filterMultipleObs Filters the seekerpos and multiple hider poses. Returns the one seeker and one filtered hider pos.
     * All vectors have only two coordinates, and no orientation.
     * \param seekerPosVectorIn
     * \param hiderPosVectorsIn
     * \param seekerPosVectorOut
     * \param hiderPosVectorOut
     */
    void filterMultipleObs(const std::vector<double>& seekerPosVectorIn, const std::vector< std::vector<double> >& hiderPosVectorsIn,
                           std::vector<double>& seekerPosVectorOut, std::vector<double>& hiderPosVectorOut);


    /*
     * \brief getNextAction get next action based on the observed positions
     * \param seekerPosVector
     * \param hiderPosVector
     * \param winState  [out] win state
     * \return the action @see HSGlobalData
     */
    //int getNextAction(std::vector<double> seekerPosVector, std::vector<double> hiderPosVector, int* winState);

    /*!
     * \brief getNextPose get the next position and pose of the user
     * \param seekerPosVector
     * \param hiderPosVector
     * \param newSeekerPosVector [out] new seeker position after doing action
     * \param winState [out] win state
     * \param resetItStartTime reset timer (only internal use, default: true)
     * \return the action @see HSGlobalData
     */
    int getNextPose(std::vector<double> seekerPosVector, std::vector<double> hiderPosVector, std::vector<double>& newSeekerPosVector, int* winState,
                    bool resetItStartTime=true);

    /*!
     * \brief getNextMultiplePoses get the next positions
     * \param seekerPosVector
     * \param hiderPosVector
     * \param n number of next poses (actions)
     * \param newSeekerPosVector
     * \param winState
     * \param resetItStartTime reset timer (only internal use, default: true)
     * \return
     */
    std::vector<int> getNextMultiplePoses(std::vector<double> seekerPosVector, std::vector<double> hiderPosVector, int n,
                                     std::vector<double>& newSeekerPosVector, int* winState, bool resetItStartTime=true);

    /*!
     * \brief getNextMultiplePosesForMultipleObs get next n poses for multiple observations
     * \param seekerPosVector
     * \param hiderPosVector
     * \param n
     * \param newSeekerPosVector
     * \param winState
     * \param resetItStartTime reset timer (only internal use, default: true)
     * \param dontExecuteIteration don't execute iteration
     * \return
     */
    std::vector<int> getNextMultiplePosesForMultipleObs(std::vector<double> seekerPosVector, std::vector< std::vector<double> > hiderPosVector,
                                                   int n, std::vector<double>& newSeekerPosVector, int* winState,
                                                        bool* dontExecuteIteration=NULL, bool resetItStartTime=true);


    //MULTIS
    /*!
     * \brief getNextMultiSeekerPoses get the next poses for multiple (2 seekers). The input should be already filtered ('mine')
     * using filterMultipleObs(), the 'other' poses should be received from the other seeker, or empty if no information (e.g.
     * no connection). The other seeker's data should not be 'too old'. The output
     * \param seekerPosVectorMine this seeker's position (filtered+checked)
     * \param hiderPosVectorMine the (filtered) hider position as seen by this seeker (empty if not visible)
     * \param seekerPosVectorOther the other's seeker (own filtered) position (empty is no information)
     * \param hiderPosVectorOther the other's hider position observation (filtered, empty is not visible or no information)
     * \param n (number of steps, i.e. distance, but not used for all algorithms)
     * \param newSeekerPosVectorMine [out] new seeker goal position for this seeker
     * \param newSeekerPosVectorOther [out] idem for other seeker (can be empty if no information)
     * \param newSeekerPosMineBelief [out] belief for current seeker's new goal (-1 is not used)
     * \param newSeekerPosOtherBelief [out] idem for other seeker's goal
     * \param winState [out] the win state (indicates if hider/seeker won)
     * \param dontExecuteIteration dontExecuteIteration don't execute iteration (set by filter)
     * \param resetItStartTime reset timer (only internal use, default: true)
     * \return
     */
    bool getNextMultiSeekerPoses(const std::vector<double>& seekerPosVectorMine, const std::vector<double>& hiderPosVectorMine,
                                 const std::vector<double>& seekerPosVectorOther, const std::vector<double>& hiderPosVectorOther,
                                       int n, std::vector<double>& newSeekerPosVectorMine, std::vector<double>& newSeekerPosVectorOther,
                                        double &newSeekerPosMineBelief, double &newSeekerPosOtherBelief,
                                        int* winState, bool* dontExecuteIteration=NULL, bool resetItStartTime=true);


    bool selectMultiSeekerPose(const std::vector<double>& newSeekerPosVectorMineFromOther, const std::vector<double>& newSeekerPosVectorOtherFromOther,
                               double newSeekerPosMineBeliefFromOther, double newSeekerPosOtherBeliefFromOther, int n,
                               std::vector<double>& newSeekerPosVector);


    /*!
     * \brief getNextPoseForMultipleObs get next pos for multiple observations
     * \param seekerPosVector
     * \param hiderPosVector
     * \param newSeekerPosVector
     * \param winState
     * \param resetItStartTime reset timer (only internal use, default: true)
     * \return
     */
    int getNextPoseForMultipleObs(std::vector<double> seekerPosVector, std::vector< std::vector<double> > hiderPosVector,
                                                   std::vector<double>& newSeekerPosVector, int* winState, bool resetItStartTime=true);

    /*!
     * \brief chooseHiderPos choose the hider position from a list of observed positions
     * \param seekerPosVector the seeker position
     * \param hiderPosVector the list of possible hider positions
     * \param checkPrev check if it consistent witht the previous step
     * \return
     */
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
     * \brief checkValidNextSeekerPos check if the pos is a posible next position for the seeker (according to the game rules)
     * \param seekerPosVector
     * \return
     */
    //bool checkValidNextSeekerPos(std::vector<double> seekerPosVector, bool checkPrev=true);

    /*!
     * \brief checkValidNextHiderPos check if the pos is a posible next position for the hider (according to the game rules; it uses the belief).
     * \param hiderPosVector
     * \return
     */
    //bool checkValidNextHiderPos(std::vector<double> hiderPosVector, std::vector<double> seekerPosVector, bool checkPrev=true);


    /*!
     * \brief isObstacle check if there is an obstacle on this location of the map
     * \param posVector
     * \return
     */
    bool isObstacle(const std::vector<double>& posVector);


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
    std::vector<double> getChosenHiderPos();

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
    void setVectorFromPos(std::vector<double> &posVector, const Pos& pos, double orientation, int index=0, bool setOrientation=true);

    /*!
     * \brief setPosFromVector set pos struct from position vector, assuming that vector contains real-world coordinates, and pos grid coordinates
     * \param pos [out]
     * \param posVector [in]
     */
    void setPosFromVector(Pos& pos, const std::vector<double> &posVector);

    /*!
     * \brief setPosFromVector set pos struct from position vector, assuming that vector contains real-world coordinates, and pos grid coordinates
     * \param pos [out]
     * \param posVector [in]
     */
    void setPosFromVector(IDPos& pos, const std::vector<double> &posVector);


protected:
    //used in chooseHider to calculate score based on distance
    static constexpr double DIST_SCORE_MULT = 2;
    static constexpr double MAX_DIST_SCORE = 5;

    //! change the order of row/col sent and receive	//! change the order of row/col sent and receivedd
    //AG140124
    static const int ROW_INDEX = 0; //1; was switched (1) for experimetn 140124
    static const int COL_INDEX = 1; //0;
    static const bool CHANGE_ORIENTATION = false;


    /*!
     * \brief init init
     * \param params
     */
    void initClass(SeekerHSParams* params, bool genMap);

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
    void getPosFromVectors(const std::vector<double> &seekerPosVectorIn, const std::vector<std::vector<double> > &hiderPosVectorIn,
                           Pos &seekerPosOut, std::vector<IDPos> &hiderPosVectorOut, bool filterHidden);



    /*!
     * \brief openGameLogFile open the game log file and start header if not yet exists
     * \param gamelogFile
     */
    void openGameLogFile(std::string gamelogFile);

    //! init log line (no new seeker pos)
    void logLineInit(const std::vector<double> &seekerPosVector, const std::vector<double> &hiderPosVector, const Pos &seekerPos,
                     const IDPos &hiderPos, bool hiderVisible);

    //! log line
    void logLine(std::vector<double> &seekerPosVector, std::vector<double> &hiderPosVector, Pos &seekerPos,
                 IDPos &hiderPos, bool& hiderVisible, std::vector<double> &newSeekerPosVector, Pos &newSeekerPos, int winState);


    //! init log line (no new seeker pos) for 2 seekers
    void logLineInit2(const std::vector<double>& seekerPosVectorMine,  const std::vector<double>& hiderPosVectorMine,
                     const std::vector<double>& seekerPosVectorOther, const std::vector<double>& hiderPosVectorOther,
                     const Pos& seekerPosMine,  const IDPos &hiderPosMine, bool hiderVisible,
                     const Pos& seekerPosOther, const Pos& hiderPosOther);

    //! log line for 2 seekers
    void logLine2(const std::vector<double>& seekerPosVectorMine,  const std::vector<double>& hiderPosVectorMine,
                 const std::vector<double> seekerPosVectorOther, const std::vector<double> hiderPosVectorOther,
                 const Pos& seekerPosMine,  const IDPos &hiderPosMine, bool hiderVisible,
                 const Pos* seekerPosOther, const Pos* hiderPosOther,
                 const std::vector<double>& newSeekerPosVectorMine, const std::vector<double>& newSeekerPosVectorOther,
                 const Pos& newSeekerPosMine, const Pos& newSeekerPosOther,
                 double newSeekerPosMineBelief, double newSeekerPosOtherBelief, int winState);

    //! log line 2 seekers, when selecting action based on both seeker's decisions
    void logLine2Select(const std::vector<double>& newSeekerPosVectorMineFromOther, const std::vector<double>& newSeekerPosVectorOtherFromOther,
                        const Pos& newSeekerPosMineFromOther, const Pos& newSeekerPosOtherFromOther,
                        double newSeekerPosMineBeliefFromOther, double newSeekerPosOtherBeliefFromOther,
                        const std::vector<double>& selectedGoalPosVectorMine, const Pos& selectedGoalPosMine);



    /*!
     * \brief setExtOriginCoords set the coordinates of the external origin. Our coordinate system is: (rows,cols) but since the rows are used
     * as first parameter, they are used as x, cols are used as y. The rotation is clockwise: how to rotate the external coordinate system to fix it with
     * our map coordinate system.
     * \param x
     * \param y
     * \param rot
     */
    void setExtOriginCoords(double x, double y, double rot);

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

private:
    //! starts or restarts algo timer
    void startAlgoTimer();

    static double ang(double y, double x);

    //! the autoplayer
    AutoPlayer* _autoPlayer;

    //! the map
    GMap* _map;

    //! seeker player (to follow)
    //Player* _seekerPlayer;

    //! next pos of seeker (given as action)
    Pos* _nextSeekerPos;

    //! game log
    HSLog* _gamelog;

    //! experiment name
    std::string _expName;

    //! params
    SeekerHSParams* _params;

    #ifndef DO_NOT_USE_MOMDP
    //! segmenter Y-space
    Segmenter* _segmenter;
    //! segmenter X-space
    Segmenter* _segmenterX;
    #endif

    //! counter of number of actions done
    unsigned int _numActions;

    //! coordinates of origin of external (robot's) coordinate reference in our map coordinates
    double _xExtOrigin, _yExtOrigin;
    double _rotateExtOrigin;

    //! coordinates of origin of my/map coordinate system in external coordinates
    double _xMyOrigin, _yMyOrigin;
    double _rotateMyOrigin;

    Timer _timer;
    int _timerID;

    //! the chosen hider pos
    std::vector<double> _chosenHiderPosVec;

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
