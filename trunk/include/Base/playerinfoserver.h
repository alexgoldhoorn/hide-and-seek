#ifndef PLAYERINFOSERVER
#define PLAYERINFOSERVER

#include "Base/playerinfo.h"

#include "Base/seekerhsparams.h"

#include <QDateTime>
#include <QTcpSocket>

//! Struct for PlayerInfo info on server.
struct PlayerInfoServer : public PlayerInfo {
    PlayerInfoServer();

    //PlayerInfoServer(const PlayerInfoServer* info);

    virtual ~PlayerInfoServer();

    virtual void clear();

    //virtual void setUserName(QString uname);

    bool flag; //1-if the PlayerInfo is ready to write, 0-if not
    //in order for the PlayerInfo to be able to write he should have read smt, either the opponentś pos or an invalid msg.

    //AG150507: not used anymore
    //bool set; //1 if the PlayerInfo is defined... corresponds to a socket,0 elsewise
    //int win; //0-game on, 1-win, -1 -loose, 2-tie

    //! timestamp indicating the exact time that the user took the last action.
    //! AG160602: this can be send highest belief or calc actions!
    QDateTime timestamp;

    //AG140506:
    //! param received from the specific player
    SeekerHSParams params;

    //! socket
    QTcpSocket* socket;

    //! this is the hider obs. trust. prob (as in PlayerInfo), but this is the fixed value
    //! used when no dynamic trust prob. are available
    //double hiderObsTrustProbFixed;

    //! ID of the user in the DB for this game
    int gameUserDBID;

    //AG151116
    //! indicates if the player can see ALL (i.e. through walls)
    bool seesAll;

};

#endif // PLAYERINFOSERVER

