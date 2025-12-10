#include "AutoHider/randomfixedhider.h"

#include <iostream>
#include <cassert>

#include "Base/hsglobaldata.h"
#include "Utils/generic.h"

#include "exceptions.h"

using namespace std;

RandomFixedHider::RandomFixedHider(SeekerHSParams* params, size_t n) : RandomHider(params,n) {
}

RandomFixedHider::~RandomFixedHider() {
}

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
