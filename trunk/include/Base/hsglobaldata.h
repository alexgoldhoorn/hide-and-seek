#ifndef HSGLOBALDATA_H
#define HSGLOBALDATA_H

#ifdef USE_QT

#include <QStringList>
#include <QDataStream>

//AG131016: function to get action name if using Qt, otherwise only the action ID
#define ACTION_NAME(a) HSGlobalData::ACTIONS_SHORT[a].toStdString()
#define ACTION_COUT(a) ACTION_NAME(a)<<"("<<a<<")"
//AG131204: name of the winstates
#define WINSTATE_NAME(w) HSGlobalData::WINSTATE_LIST[w].toStdString()
#define WINSTATE_COUT(w) WINSTATE_NAME(w)<<"("<<w<<")"

class HSGlobalData {
public:
    //! list of MAPS file name, it's index is it's ID
    static const QStringList MAPS;

    //ACTIONS = [HALT_ACT; N_ACT; NE_ACT; E_ACT; SE_ACT; S_ACT; SW_ACT; W_ACT; NW_ACT];
    //! Actions of each direction + halt: 1 + 8
    static const QStringList ACTIONS;
    //! Short name of the actions (1+8)
    static const QStringList ACTIONS_SHORT;
    //! Win state description
    static const QStringList WINSTATE_LIST;

    //! signal to send map by network (instead of loading server side)
    static const quint16 MAP_PASSED_BY_NETWORK = 65534; //ag131213: 1 less than max, which is used for stop!

    // AG160727: changed to Qt 5.2, because this is the latest Qt 5 version passed with Ubuntu 14.04
    //! data stream connection version
    static const QDataStream::Version DATASTREAM_VERSION = QDataStream::Qt_5_2; 

    //! default server IP
    static const std::string DEFAULT_SERVER_IP;

    //AG150121: names of enum MessageSendTo
    static const QStringList MESSAGESENDTO_NAMES;

    //! names of the type of players
    static const QStringList PLAYER_TYPE_NAMES;

    /*!
     * \brief getActionDirection get the direction of the action in rad, 0 rad being north and going clockwise
     * \param action
     * \return
     */
    static double getActionDirection(char action);

#else

#include <string>

//AG131016: function to get action name if using Qt, otherwise only the action ID
//#define ACTION_NAME(a) a
//#define ACTION_COUT(a) a
#define ACTION_NAME(a) HSGlobalData::getActionName(a)
#define ACTION_COUT(a) ACTION_NAME(a)<<"("<<a<<")"
//AG131204: name of the winstates
#define WINSTATE_NAME(w) w
#define WINSTATE_COUT(w) w


//AG130415: was class but becuase of some linking problems Sergi found it was better to put namespace
namespace HSGlobalData
{

std::string getActionName(int actionID);


/*!
 * \brief getActionDirection get the direction of the action in rad, 0 rad being north and going clockwise
 * \param action
 * \return
 */
double getActionDirection(char action);

#endif
    //HSGlobalData();


    static const char GAME_HIDE_AND_SEEK = 0;
    static const char GAME_FIND_AND_FOLLOW = 1;
    //AG141124: 2 robots
    static const char GAME_FIND_AND_FOLLOW_2ROB = 2;
    //AG150427: n (>1) robots
    static const char GAME_FIND_AND_FOLLOW_MULTI_ROB = 3;

    //AG150427
    static const int MAX_NUM_PLAYERS = 100;

    //All action IDs (AG140407 note: do not change IDs since they are used to calculate the direction)
    static const char ACT_H = 0;
    static const char ACT_N = 1;
    static const char ACT_NE= 2;
    static const char ACT_E = 3;
    static const char ACT_SE= 4;
    static const char ACT_S = 5;
    static const char ACT_SW= 6;
    static const char ACT_W = 7;
    static const char ACT_NW= 8;
    //! last action ID
    static const char NUM_ACTIONS = (ACT_NW+1);

    //AG140407
    //All directions of the actions
    static const char ACT_DIR_H = 0; //no direction
    static const char ACT_DIR_N = 0;
    static const char ACT_DIR_NE= 45;
    static const char ACT_DIR_E = 90;
    static const char ACT_DIR_SE= 4;
    static const char ACT_DIR_S = 5;
    static const char ACT_DIR_SW= 6;
    static const char ACT_DIR_W = 7;
    static const char ACT_DIR_NW= 8;

    //AG120903: hider type, also in hsautohider.
    //AG120904: added human, to make it consistent
    static const int OPPONENT_TYPE_HUMAN = 0;
    static const int OPPONENT_TYPE_HIDER_RANDOM = 1;
    static const int OPPONENT_TYPE_HIDER_SMART = 2;
    static const int OPPONENT_TYPE_HIDER_ACTION_LIST = 3;
    static const int OPPONENT_TYPE_SEEKER = 4;
    static const int OPPONENT_TYPE_HIDER_VERYSMART = 5;
    static const int OPPONENT_TYPE_HIDER_ALLKNOWING = 6;
    static const int OPPONENT_TYPE_HIDER_VALLKNOWING = 7;
    static const int OPPONENT_TYPE_HIDER_VERYSMART2 = 8;
    static const int OPPONENT_TYPE_HIDER_V2ALLKNOWING = 9;
    static const int OPPONENT_TYPE_HIDER_RANDOM_WALKER = 10;
    static const int OPPONENT_TYPE_HIDER_FILE = 11;
    static const int OPPONENT_TYPE_HIDER_FIXED_RAND_POS = 12;

    //ag130404: code sent by client to server to indicate a new win dist
    static const int SERVER_WIN_DIST_CODE = 9999;

    //AG121102: smallest number to be seen as 0
    static constexpr double ZERO_EPS = 1e-10;

    //AG130611: max double
    //AG131017: highest and lowest numbers
    static constexpr double INFTY_NEG_DBL = -1e100; // = 1e100; //DBL_MAX = 1.79769e+308;
    static constexpr double INFTY_POS_DBL =  1e100;
    static constexpr float INFTY_NEG_FLT  = -1e30;
    static constexpr float INFTY_POS_FLT  =  1e30;

    //! maximum actions, multiplication factor,
    //! max_act = factor * (rows+cols)
    static constexpr float MAX_ACT_MULT_FACTOR = 2.0;


    //AG131211: game status
    static const int GAME_STATE_NOT_STARTED = -1;
    static const int GAME_STATE_RUNNING = 0;
    static const int GAME_STATE_SEEKER_WON = 1;
    static const int GAME_STATE_HIDER_WON = 2;
    static const int GAME_STATE_TIE = 3;

    //AG150511: depricated by Player enum
    //AG131211: player types
    /*static const char PLAYER_TYPE_HIDER = 0;
    static const char PLAYER_TYPE_SEEKER = 1;*/

    //default server loaction
    static const int DEFAULT_SERVER_PORT = 1120;

    //initial position type
    static const char INIT_POS_TYPE_RANDOM = 0;
    static const char INIT_POS_TYPE_BASE = 1;
    static const char INIT_POS_TYPE_DIST = 2;
    static const char INIT_POS_TYPE_FIXED = 3;

    //filter score type
    static const char FILTER_SCORE_OLD = 0;
    static const char FILTER_SCORE_NEW1 = 1;
    static const char FILTER_SCORE_USE_ID = 2;
    static const char FILTER_SCORE_NEW2_WEIGHTED = 3;
    static const char FILTER_SCORE_NEW2_PREF_TAG = 4;
    static const char FILTER_SCORE_USE_TAG_ONLY = 5;

    //AG140426: automated walkers types
    static const int AUTOWALKER_NONE = 0;
    static const int AUTOWALKER_RANDOM = 1;
    static const int AUTOWALKER_SFM = 2; //Social Force Model
    static const int AUTOWALKER_RANDOM_GOAL = 3;
    static const int AUTOWALKER_FILE = 4;
    static const int AUTOWALKER_FIXED_POS = 5;

    //AG150120: players; AG150430: only 1 seeker type, but can be multiple, id is used to identify them
    enum Player {P_None=-1, P_Hider, P_Seeker}; //1, P_Seeker2};

};

#endif // HSGLOBALDATA_H
