#ifndef HSCONFIG_H
#define HSCONFIG_H


#define HS_VERSION 1

//AG131204: to know if it is compiled on 64 or 32 bits machine
#if __x86_64__ || __ppc64__ || _WIN64
    #define _MACHINEBITS 64
#else
    #define _MACHINEBITS 32
#endif



//debug lines

//per (big) step + map outputs
#ifdef DEBUG_HS_ON
    #define DEBUG_HS(v) v
#else
    #define DEBUG_HS(v)
#endif

//init only
#ifdef DEBUG_HS_INIT_ON
    #define DEBUG_HS_INIT(v) v
#else
    #define DEBUG_HS_INIT(v)
#endif

//low level debug
#ifdef DEBUG_HS1_ON
    #define DEBUG_HS1(v) v
#else
    #define DEBUG_HS1(v)
#endif


//detailed map debugging
#ifdef DEBUG_MAP_ON
    #define DEBUG_MAP(v) v
#else
    #define DEBUG_MAP(v)
#endif

//detailed segment debugging
#ifdef DEBUG_SEGMENT_ON
    #define DEBUG_SEGMENT(v) v
#else
    #define DEBUG_SEGMENT(v)
#endif

//show transitions
#ifdef DEBUG_SHOW_TRANS_ON
    #define DEBUG_SHOW_TRANS (true)
#else
    #define DEBUG_SHOW_TRANS (false)
#endif

//show autohider
#ifdef DEBUG_AUTOHIDER_ON
    #define DEBUG_AUTOHIDER(v) v
#else
    #define DEBUG_AUTOHIDER(v)
#endif

//delete debug
#ifdef DEBUG_DELETE_ON
    #define DEBUG_DELETE(v) v
#else
    #define DEBUG_DELETE(v)
#endif

//player
#ifdef DEBUG_PLAYER_ON
    #define DEBUG_PLAYER(v) v
#else
    #define DEBUG_PLAYER(v)
#endif


//seeker hs debug
#ifdef DEBUG_SHS_ON
    #define DEBUG_SHS(v) v
#else
    #define DEBUG_SHS(v)
#endif

//print belief
#ifdef DEBUG_PB_ON
    #define DEBUG_PB(v) v
#else
    #define DEBUG_PB(v)
#endif

//print info taking action
#ifdef DEBUG_AUTOHIDER_ON
    #define DEBUG_AUTOHIDER(v) v
#else
    #define DEBUG_AUTOHIDER(v)
#endif

//pomcp
#ifdef DEBUG_POMCP_ON
    #define DEBUG_POMCP(v) v
#else
    #define DEBUG_POMCP(v)
#endif

#ifdef DEBUG_POMCP_SIM_ON
    #define DEBUG_POMCP_SIM(v) v
#else
    #define DEBUG_POMCP_SIM(v)
#endif

#ifdef DEBUG_CONNECTOR_ON
    #define DEBUG_CONNECTOR(v) v
#else
    #define DEBUG_CONNECTOR(v)
#endif


#ifdef DEBUG_POMCP_DETAIL_ON
    #define DEBUG_POMCP_DETAIL(v) v
    #define DEBUG_NUM_BELIEF_TOSTRING -1
#else
    #define DEBUG_POMCP_DETAIL(v)
    #define DEBUG_NUM_BELIEF_TOSTRING 5
#endif


#ifdef DEBUG_SERVER_ON
    #define DEBUG_SERVER(v) v
#else
    #define DEBUG_SERVER(v)
#endif

#ifdef DEBUG_SERVER_VERB_ON
    #define DEBUG_SERVER_VERB(v) v
#else
    #define DEBUG_SERVER_VERB(v)
#endif


#ifdef DEBUG_CLIENT_ON
    #define DEBUG_CLIENT(v) v
#else
    #define DEBUG_CLIENT(v)
#endif

#ifdef DEBUG_CLIENT_VERB_ON
    #define DEBUG_CLIENT_VERB(v) v
#else
    #define DEBUG_CLIENT_VERB(v)
#endif

#ifdef DEBUG_CHECK_INF_LOOP_ON
    #define DEBUG_CHECK_INF_LOOP(v) v
#else
    #define DEBUG_CHECK_INF_LOOP(v)
#endif

#ifdef DEBUG_PEOPLEPRED_ON
    #define DEBUG_PEOPLEPRED(v) v
#else
    #define DEBUG_PEOPLEPRED(v)
#endif

#ifdef DEBUG_PATHPLAN_ON
    #define DEBUG_PATHPLAN(v) v
#else
    #define DEBUG_PATHPLAN(v)
#endif

#endif // HSCONFIG_H
