#include "Server/playerinfo.h"

PlayerInfo::PlayerInfo() {
    clear();
}

void PlayerInfo::clear() {
    previousPos.clear();
    currentPos.clear();
    lastAction = -1;
    flag = true;
    set = false;
    numberActions = 0;
    playerType = HSGlobalData::P_None;
    multiHasGoalPoses = false;
}

void PlayerInfo::setUserName(QString uname) {
    username = uname;
    params.userName = uname.toStdString();
}

bool PlayerInfo::isSeeker() {
    return playerType==HSGlobalData::P_Seeker1 || playerType==HSGlobalData::P_Seeker2;
}

std::string PlayerInfo::toString() {
    QString s = "["+username+"(";
    if ((int)playerType<0)
        s += "?";
    else
        s += HSGlobalData::PLAYER_TYPE_NAMES[playerType];
    s += ")";
    return s.toStdString();
}
