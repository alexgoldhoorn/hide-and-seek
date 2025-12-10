#include "AutoHider/randomfixedhider.h"

#include <iostream>

#include "Base/hsglobaldata.h"

#include "exceptions.h"

using namespace std;

RandomFixedHider::RandomFixedHider(SeekerHSParams* params, size_t n) : RandomHider(params,n) {
}

RandomFixedHider::~RandomFixedHider() {
}


/*int RandomFixedHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    return HSGlobalData::ACT_DIR_H;
}*/

/*Pos RandomFixedHider::getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    return hiderPos;
}*/

Pos RandomFixedHider::getNextPosRun(int actionDone, int *newAction) {
    playerInfo.nextPos = playerInfo.currentPos;
    if (newAction!=NULL)
        *newAction = HSGlobalData::ACT_DIR_H;

    return playerInfo.nextPos;
}

int RandomFixedHider::getHiderType() const {
    return HSGlobalData::OPPONENT_TYPE_HIDER_FIXED_RAND_POS;
}

std::string RandomFixedHider::getName() const {
    return "RandomFixedHider";
}


std::vector<IDPos> RandomFixedHider::getAllNextPos(Pos seekerPos, Pos hiderPos) {
    //no update, since we don't move
    return _autoWalkerVec;
}
