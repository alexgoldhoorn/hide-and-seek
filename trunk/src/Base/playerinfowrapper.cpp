#include "Base/playerinfowrapper.h"

#include <cmath>

#include "exceptions.h"

#include <iostream>
using namespace std;

PlayerInfoWrapper::PlayerInfoWrapper(PlayerInfo *playerInfo, SeekerHSParams *params) {
    _params = params;
    this->playerInfo = playerInfo;
    _beliefSum = -1;

    switch(_params->multiSeekerProcessOrder) {
        case SeekerHSParams::MSHB_PROC_ORDER_ID:
        break;
        case SeekerHSParams::MSHB_PROC_ORDER_BELIEF:
        recalc();
        break;
        default:
        throw CException(_HERE_, "unknown multi Seekers process order type");
        break;
    }
}

void PlayerInfoWrapper::recalc() {
    _beliefSum = 0;
    for(const BPos& bpos : playerInfo->multiHBPosVec) {
        _beliefSum += bpos.b;
    }
}

void PlayerInfoWrapper::recalcIfReq() {
    if (_params->multiSeekerProcessOrder==SeekerHSParams::MSHB_PROC_ORDER_BELIEF)
        recalc();
}

double PlayerInfoWrapper::getBeliefSum() {
    return _beliefSum;
}

bool PlayerInfoWrapper::operator <(const PlayerInfoWrapper& other) const {
    switch(_params->multiSeekerProcessOrder) {
        case SeekerHSParams::MSHB_PROC_ORDER_ID:
            return playerInfo->id < other.playerInfo->id;
            break;
        case SeekerHSParams::MSHB_PROC_ORDER_BELIEF:
            if (_beliefSum == other._beliefSum)
                return playerInfo->id < other.playerInfo->id;
            else
                return _beliefSum < other._beliefSum;
            break;
        default:
            throw CException(_HERE_, "unknown multi Seekers process order type");
            return false;
            break;
    }
}

bool PlayerInfoWrapper::operator >(const PlayerInfoWrapper& other) const {
    switch(_params->multiSeekerProcessOrder) {
        case SeekerHSParams::MSHB_PROC_ORDER_ID:
            return playerInfo->id > other.playerInfo->id;
            break;
        case SeekerHSParams::MSHB_PROC_ORDER_BELIEF:
            if (_beliefSum == other._beliefSum)
                return playerInfo->id > other.playerInfo->id;
            else
                return _beliefSum > other._beliefSum;
            break;
        default:
            throw CException(_HERE_, "unknown multi Seekers process order type");
            return false;
            break;
    }
}

bool PlayerInfoWrapper::operator ==(const PlayerInfoWrapper& other) const {
    switch(_params->multiSeekerProcessOrder) {
        case SeekerHSParams::MSHB_PROC_ORDER_ID:
            return playerInfo->id == other.playerInfo->id;
            break;
        case SeekerHSParams::MSHB_PROC_ORDER_BELIEF:
            return abs(_beliefSum - other._beliefSum) <= EPS;
            break;
        default:
            throw CException(_HERE_, "unknown multi Seekers process order type");
            return false;
            break;
    }
}

bool PlayerInfoWrapper::operator !=(const PlayerInfoWrapper& other) const {
    switch(_params->multiSeekerProcessOrder) {
        case SeekerHSParams::MSHB_PROC_ORDER_ID:
            return playerInfo->id != other.playerInfo->id;
            break;
        case SeekerHSParams::MSHB_PROC_ORDER_BELIEF:
            /*cout<<" not operator: belief: "<<abs(_beliefSum - other._beliefSum)<<", eps="<<EPS
               << "; !=: "<<(abs(_beliefSum - other._beliefSum) > EPS)<<endl*/;
            return (abs(_beliefSum - other._beliefSum) > EPS);
            break;
        default:
            throw CException(_HERE_, "unknown multi Seekers process order type");
            return false;
            break;
    }
}
