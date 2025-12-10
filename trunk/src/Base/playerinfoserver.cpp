#include "Base/playerinfoserver.h"

PlayerInfoServer::PlayerInfoServer() : PlayerInfo() {
}

PlayerInfoServer::~PlayerInfoServer() {
}

void PlayerInfoServer::clear() {
    PlayerInfo::clear();

    flag = true;
    seesAll = false;
}

