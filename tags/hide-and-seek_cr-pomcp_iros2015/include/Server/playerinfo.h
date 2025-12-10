#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include "seekerhs.h"
#include "hsglobaldata.h"
#include "HSGame/pos.h"

#include <vector>
#include <QDateTime>


//TODO: use Player instead of PlayerInfo
//! Struct for PlayerInfo info on server.
struct PlayerInfo {
    PlayerInfo();

    //AG131210: clear all values (to -1)
    void clear();

    void setUserName(QString uname);

    //! current position
    Pos currentPos;

    //AG121112:
    //! previous pos
    Pos previousPos;

    //!number of actions taken
    int numberActions;

    //int action[MAXACTIONS]; //actions
    //AG131210: change to last action
    int lastAction;

    bool flag; //1-if the PlayerInfo is ready to write, 0-if not
    //in order for the PlayerInfo to be able to write he should have read smt, either the opponentś pos or an invalid msg.

    bool set; //1 if the PlayerInfo is defined... corresponds to a socket,0 elsewise
    //int win; //0-game on, 1-win, -1 -loose, 2-tie
    QString username; //players name
    QDateTime timestamp;    //a timestamp indicating the exact time that the user took the last action.

    //AG140531: meta info
    QString metaInfo;
    //comments user
    QString comments;

    //AG140506:
    //! param received from the specific player
    SeekerHSParams params;

    //! player type (seeker/hider)
    HSGlobalData::Player playerType;

    //AG1500202
    //! indicates if the goal poses are received already (for multiple seekers)
    bool multiHasGoalPoses;

    //! list of goal for the seekers (for multiple seekers)
    //! first value refers to goal of this player
    std::vector<Pos> multiGoalPosesVec;

    //! list of belief values for the seekers (for multiple seekers)
    //! same order as multiGoalPosesVec
    std::vector<double> multiGoalBeliefVec;

    //! chosen pos by seeker
    Pos multiChosenPos;

    //! is a seeker, otherwise hider
    bool isSeeker();


    //!get user and player type name (for debug): [name(type)]
    std::string toString();
};


#endif // PLAYERINFO_H
