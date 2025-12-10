#include "gamenoserver.h"

GameNoServer::GameNoServer(SeekerHS* seekerHS) {
    _seekerHS = seekerHS;
}

bool GameNoServer::startGame() {
    return false;
}
void GameNoServer::moveToThreadVariables(QThread* thread) {

}
