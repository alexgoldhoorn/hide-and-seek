#ifndef HSGLOBALDATA_H
#define HSGLOBALDATA_H

//#include <cfloat>

#ifdef USE_QT

#include <QStringList>

class HSGlobalData {
public:
    //! list of MAPS file name, it's index is it's ID
    static const QStringList MAPS;

    //ACTIONS = [HALT_ACT; N_ACT; NE_ACT; E_ACT; SE_ACT; S_ACT; SW_ACT; W_ACT; NW_ACT];
    //! Actions of each direction + halt: 1 + 8
    static const QStringList ACTIONS;
    //! Short name of the actions (1+8)
    static const QStringList ACTIONS_SHORT;

    //! signal to send map by network (instead of loading server side)
    static const quint16 MAP_PASSED_BY_NETWORK = 65535;


#else

//AG130415: was class but becuase of some linking problems Sergi found it was better to put namespace
namespace HSGlobalData
{
#endif
    //HSGlobalData();

    //All action IDs
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

    //ag130404: code sent by client to server to indicate a new win dist
    static const int SERVER_WIN_DIST_CODE = 9999;

    //AG121102: smallest number to be seen as 0
    static const double ZERO_EPS = 1e-10;

    //AG130611: max double
    static const double INFTY = 1e100; //DBL_MAX = 1.79769e+308;

};

#endif // HSGLOBALDATA_H
