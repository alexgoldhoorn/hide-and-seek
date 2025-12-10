#ifndef SEEKERHS_H
#define SEEKERHS_H

#include <string>
#include <vector>

//predefine classes
class AutoPlayer;
class GMap;
class Player;
struct Pos;
class HSLog;
//class SolverParams;
class Segmenter;

using namespace std;


/*!
 * \brief The SeekerHSParams struct Parameters for the file
 */
struct SeekerHSParams {
    //! init
    SeekerHSParams();
    //! print variables
    void printVariables(bool showExtraParams=false);
    //! set solver params for APPL
    void setSolverParams();

    //solver types
    static const char SOLVER_NOT_SET = 0;
    static const char SOLVER_OFFLINE = 1;
    static const char SOLVER_LAYERED = 2;
    static const char SOLVER_LAYERED_COMPARE = 3;
    static const char SOLVER_MCVI_OFFLINE = 4;
    static const char SOLVER_POMCP = 5;

    static const char SOLVER_SMART_SEEKER = 9; //NOTE: this is not a solver!!

    //segmenter type
    static const char SEGMENTER_NOT_SET = 0;
    static const char SEGMENTER_BASIC = 1;
    static const char SEGMENTER_KMEANS = 2;
    static const char SEGMENTER_ROBOT_CENTERED = 3;
    static const char SEGMENTER_TEST = 4;
    static const char SEGMENTER_CENTERED_COMBINED = 5;

    //upper bound init
    static const char UPPERBOUND_INIT_FIB = 0;
    static const char UPPERBOUND_INIT_MDP = 1;

    //reward types
    static const char REWARD_NOT_SET = 0;
    static const char REWARD_TRIANGLE = 3;
    static const char REWARD_FINAL = 4; //only final state (1 or -1)
    static const char REWARD_FINAL_CROSS = 14; //idem and 1 if both players cross eachother

    /* reward types, as defined in CreateMOMDP?.m (note only few used here)
                   - 1=only final states WIN score = L
                                         LOOSE score =  -2L
                   - 2=all states:       WIN score = 2L
                                         LOOSE score = -3L
                                         other score = (based distances)
                    - 3=all states
                    - 4=only final states: WIN = 1 / LOOSE = -1
                    - 5=only final states win only: WIN = 1
                    - 6=only final states: WIN = 1 / LOOSE = -0.2
    */

    //default IP
    static const int DEFAULT_SERVER_PORT = 1120;
    //static char* DEFAULT_SERVER_IP = "localhost";

    // -- files --

    //! map file name
    string mapFile;
    //! pomdp file name
    string pomdpFile;
    //! policy file name
    string policyFile;
    //! log file name
    string logFile;
    //! time log file name (csv)
    string timeLogFile;
    //! game log file
    string gameLogFile;

    //! experiment name
    string expName;

    //! solver type
    char solverType;

    //AG130613: reward type (like Matlab createMOMDPfile5.m)
    //! reward type
    char rewardType;

    //AG130613: game time
    //! max game time, after which ends in a tie (0=endless)
    unsigned int maxGameTime;

    //! win distance (to hider, default 0 i.e. on same place)
    int winDist;

    //! grid cell width (m) in the real world
    double cellSizeM;

    // -- online parameters --
    //! upper bound init type
    char ubInitType;
    //! target precision
    double targetPrecision;
    //! target init precision factor
    double targetInitPrecFact;

    //ag130503
    //! set final states in the top model
    bool setFinalStateOnTop;
    //! set simple reward on top
    bool setFinalTopRewards;


    // segmenter type
    //! segmenter type
    char segmenterType;
    //! segmenter X type
    char segmenterTypeX;
    //! segment value type;
    char segmentValueType; // = LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_ONLY;
    //! top reward aggregation
    char topRewAggr; // = LayeredHSMOMDP::TOP_REWARD_SUM;
    //! k for k-nearest neighbour segmentation
    int k;

    //robot centered segmentation params
    double rcSegmDist;
    double rcAngDist;
    double rcBaseRad;
    double rcHighResR;
    //robot centered segmentation X params
    double rcXSegmDist;
    double rcXAngDist;
    double rcXBaseRad;
    double rcXHighResR;


    // APPL options
    //! use best action look ahead
    bool useLookahead;
    //! time out for learning policy
    double timeoutSeconds;
    //! maximum search depth in tree
    int maxTreeDepth;
    //! do pruning or not
    bool doPruning;
    //! memory limit (MB), 0 is no limit
    unsigned long memoryLimit;
    //! show params
    bool showParams;


    // parameters for simulation / connection
    //! server IP
    string serverIP;
    //! server port
    int serverPort;
    //! username for database
    string userName;
    //! map ID
    int mapID;

    // opponent type and params
    //! opponent type
    int opponentType;
    //! opponent action file
    string oppActionFile;
    //! opponent std of noise (used by smart)
    double oppHiderNoiseStd;

    //POMCP params
    //! number of initial belief states
    unsigned int numInitBeliefStates;
    //! number of simulations
    unsigned int numSim;
    //! maximum search depth in tree
    //int maxTreeDepth;
    //! exploration constant
    double explorationConst;
    //! max expand count
    unsigned int expandCount;

    //AG130722
    //! allow inconsistent observations (i.e. observation which is position futher than 1 step from prev time)
    bool allowInconsistObs;
    //! also observe own position
    bool ownPosObs;

    //AG130724
    //! Smart seeker score type: avg/sum/max
    char smartSeekerScoreType;
    //! Smart seeker max number of positions for which the score has to be calculated when
    //! it the hider is hidden (0=all)
    unsigned int smartSeekerMaxCalcsWhenHidden;


};




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

    // win state of the game
    static const int STATE_INIT = -1;
    static const int STATE_PLAYING = 0;
    static const int STATE_WIN_SEEKER = 1;
    static const int STATE_WIN_HIDER = 2;
    static const int STATE_TIE = 3;




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
    SeekerHS(string mapFile, string expName, int solverType, string pomdpFile, string policyFile, string logFile, string timeLogFile, string gamelogFile);

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
    void init(vector<double> seekerPosVector, vector<double> hiderPosVector);

    /*!
     * \brief initMultipleObs
     * \param seekerPosVector
     * \param hiderPosVector
     */
    void initMultipleObs(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector);

    /*!
     * \brief getNextAction get next action based on the observed positions
     * \param seekerPosVector
     * \param hiderPosVector
     * \param winState  [out] win state
     * \return the action @see HSGlobalData
     */
    //int getNextAction(vector<double> seekerPosVector, vector<double> hiderPosVector, int* winState);

    /*!
     * \brief getNextPose get the next position and pose of the user
     * \param seekerPosVector
     * \param hiderPosVector
     * \param newSeekerPosVector [out] new seeker position after doing action
     * \param winState [out] win state
     * \return the action @see HSGlobalData
     */
    int getNextPose(vector<double> seekerPosVector, vector<double> hiderPosVector, vector<double>& newSeekerPosVector, int* winState);

    /*!
     * \brief getNextMultiplePoses get the next positions
     * \param seekerPosVector
     * \param hiderPosVector
     * \param n number of next poses (actions)
     * \param newSeekerPosVector
     * \param winState
     * \return
     */
    vector<int> getNextMultiplePoses(vector<double> seekerPosVector, vector<double> hiderPosVector, int n, vector<double>& newSeekerPosVector, int* winState);


    vector<int> getNextMultiplePosesForMultipleObs(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector, int n, vector<double>& newSeekerPosVector, int* winState);


    vector<double> chooseHiderPos(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector, bool checkPrev);


    /*!
     * \brief getAutoPlayer return auto player
     * \return
     */
    AutoPlayer* getAutoPlayer() {
        return _autoPlayer;
    }

    /*!
     * \brief getSegmenter
     * \return
     */
    Segmenter* getSegmenter() {
        return _segmenter;
    }

    /*!
     * \brief getSegmenterX
     * \return
     */
    Segmenter* getSegmenterX() {
        return _segmenterX;
    }

    /*!
     * \brief setMap
     * \param map
     */
    void setMap(GMap* map);

    /*!
     * \brief getMap
     * \return
     */
    GMap* getMap() {
       return _map;
    }

    /*!
     * \brief setWinDistToHider set the distance to hider from which the seeker wins
     * \param winDist
     *
     * Default in simulation: 0
     * In real-world: >= 1
     */
    void setWinDistToHider(int winDist);

    /*!
     * \brief getWinDistToHider get the distance to hider from which the seeker wins
     * \return
     */
    int getWinDistToHider() {
        return _params->winDist;
    }

    /*!
     * \brief setCellSizeM set cell width in the real world (meters)
     * \param cellSizeM
     */
    void setCellSizeM(double cellSizeM);

    /*!
     * \brief getCellSizeM cell width in the real world (meters)
     * \return
     */
    double getCellSizeM() {
        return _params->cellSizeM;
    }

    /*!
     * \brief stopGame stop the game
     * \param winState
     */
    void stopGame(int winState);

    /*!
     * \brief getActionsDone the number of actions done
     * \return
     */
    unsigned int getActionsDone() {
        return _numActions;
    }

    /*!
     * \brief setMaxNumActions maximum number of actions (note: default set by init!)
     * \param maxActions
     */
    void setMaxNumActions(unsigned int maxActions) {
        _maxNumActions = maxActions;
    }

    /*!
     * \brief getMaxNumActions maximum number of actions before a tie
     * \return
     */
    unsigned int getMaxNumActions() {
        return _maxNumActions;
    }





    /*!
     * \brief checkValidNextSeekerPos check if the pos is a posible next position for the seeker (according to the game rules)
     * \param seekerPosVector
     * \return
     */
    bool checkValidNextSeekerPos(vector<double> seekerPosVector, bool checkPrev=true);

    /*!
     * \brief checkValidNextHiderPos check if the pos is a posible next position for the hider (according to the game rules; it uses the belief).
     * \param hiderPosVector
     * \return
     */
    bool checkValidNextHiderPos(vector<double> hiderPosVector, vector<double> seekerPosVector, bool checkPrev=true);


    /*!
     * \brief isObstacle check if there is an obstacle on this location of the map
     * \param posVector
     * \return
     */
    bool isObstacle(vector<double> posVector);


    /*!
     * \brief getWinState get the win state based on the current positions of the hider and seeker
     * \param seekerPos
     * \param hiderPos
     * \param hiderVisible
     * \return
     */
    int getWinState(Pos seekerPos, Pos hiderPos, bool hiderVisible);

    /*!
     * \brief getParams get params
     * \return
     */
    SeekerHSParams* getParams();

protected:
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
    void getPosFromVectors(vector<double> &seekerPosVector, vector<double> &hiderPosVector, Pos &seekerPos, Pos &hiderPos, bool& hiderVisible);

    /*!
     * \brief getOrientation get the orientation of pos2 wrt pos2
     * \param pos1
     * \param pos2
     * \return
     */
    double getOrientation(Pos pos1, Pos pos2);


    /*!
     * \brief openGameLogFile open the game log file and start header if not yet exists
     * \param gamelogFile
     */
    void openGameLogFile(string gamelogFile);

    //! log line
    void logLine(vector<double> &seekerPosVector, vector<double> &hiderPosVector, Pos &seekerPos, Pos &hiderPos, bool& hiderVisible,
                 vector<double> &newSeekerPosVector, Pos &newSeekerPos, int winState);


    //! init log line (no new seeker pos)
    void logLineInit(vector<double> &seekerPosVector, vector<double> &hiderPosVector, Pos &seekerPos, Pos &hiderPos, bool& hiderVisible);

    /*!
     * \brief setVectorFromPos set position vector from pos struct, assuming that vector contains real-world coordinates, and pos grid coordinates
     * \param posVector [out]
     * \param pos [in]
     * \param orientation [in]
     * \param index [in] index of pose (in vector: i=0: 1st x, i=1: 1st y, i=2: 1st orientation, i=3: 2nd x, .. etc.)
     */
    void setVectorFromPos(vector<double> &posVector, Pos& pos, double orientation, int index=0);

    /*!
     * \brief setPosFromVector set pos struct from position vector, assuming that vector contains real-world coordinates, and pos grid coordinates
     * \param posVector [in]
     * \param pos [out]     
     */
    void setPosFromVector(Pos& pos, vector<double> &posVector);

    /*!
     * \brief setExtOriginCoords set the coordinates of the external origin. Our coordinate system is: (rows,cols) but since the rows are used
     * as first parameter, they are used as x, cols are used as y. The rotation is clockwise: how to rotate the external coordinate system to fix it with
     * our map coordinate system.
     * \param x
     * \param y
     * \param rot
     */
    void setExtOriginCoords(double x, double y, double rot);


private:
    double ang(double y, double x);

    //! the autoplayer
    AutoPlayer* _autoPlayer;

    //! the map
    GMap* _map;

    //! seeker player (to follow)
    Player* _seekerPlayer;

    //! next pos of seeker (given as action)
    Pos* _nextSeekerPos;

    //! game log
    HSLog* _gamelog;

    //! experiment name
    string _expName;

    //! params
    SeekerHSParams* _params;

    //! segmenter Y-space
    Segmenter* _segmenter;
    //! segmenter X-space
    Segmenter* _segmenterX;

    //! max number actions, if passed then it is a tie
    unsigned int _maxNumActions;

    //! counter of number of actions done
    unsigned int _numActions;

    //! coordinates of origin of external (robot's) coordinate reference in our map coordinates
    double _xExtOrigin, _yExtOrigin;
    double _rotateExtOrigin;

    //! coordinates of origin of my/map coordinate system in external coordinates
    double _xMyOrigin, _yMyOrigin;
    double _rotateMyOrigin;
};

#endif // SEEKERHS_H
