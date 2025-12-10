#include "POMCP/hsstate.h"

#include <sstream>
#include "exceptions.h"

using namespace pomcp;
using namespace std;

HSState::
HSState() : State() {
}

HSState::HSState(Pos seekerPos, Pos hiderPos) : State() {
    this->seekerPos = seekerPos;
    this->hiderPos = hiderPos;

    /*if (makeContinuous) {
        if (this->seekerPos.isSet() && !this->seekerPos.hasDouble()) this->seekerPos.add(0.5,0.5);
        if (this->hiderPos.isSet() && !this->hiderPos.hasDouble()) this->hiderPos.add(0.5,0.5);
    }*/
}

HSState::HSState(int seekerRow, int seekerCol, int hiderRow, int hiderCol) : State() {
    seekerPos.set(seekerRow,seekerCol);
    hiderPos.set(hiderRow,hiderCol);
}

HSState::~HSState(){
}



State* HSState::copy() const {
    HSState* state = new HSState(seekerPos,hiderPos);
    state->prob = prob;
    /*state->hiderPos = hiderPos;
    state->seekerPos = seekerPos;*/
    return state;
}


string HSState::toString() const {
    stringstream sstr;
    sstr << "<S:"<<seekerPos.toString()<<";H:"<<hiderPos.toString()<<";p="<<prob<<">";
    return sstr.str();
}


bool HSState::hiderVisible() const {
    return hiderPos.isSet();
}


bool HSState::operator <(const State& other) const {
    if (other.getStateType()==State::HSState_Type) {
        const HSState& hsstate = static_cast<const HSState&>(other);

        return (seekerPos<hsstate.seekerPos || (seekerPos==hsstate.seekerPos && hiderPos<hsstate.hiderPos));
    } else {
        throw CException(_HERE_,"HSState::operator <: requires a HSState as parameter");
    }

}

bool HSState::operator >(const State& other) const {
    if (other.getStateType()==State::HSState_Type) {
        const HSState& hsstate = static_cast<const HSState&>(other);

        return (seekerPos>hsstate.seekerPos || (seekerPos==hsstate.seekerPos && hiderPos>hsstate.hiderPos));
    } else {
        throw CException(_HERE_,"HSState::operator >: requires a HSState as parameter");
    }
}

bool HSState::operator ==(const State& other) const {
    if (other.getStateType()==State::HSState_Type) {
        const HSState& hsstate = static_cast<const HSState&>(other);

        return (hsstate.seekerPos.equals(seekerPos) && hsstate.hiderPos.equals(hiderPos));
    } else {
        return false;
    }
}

bool HSState::operator !=(const State& other) const {
    if (other.getStateType()==State::HSState_Type) {
        const HSState& hsstate = static_cast<const HSState&>(other);

        return (!hsstate.seekerPos.equals(seekerPos) || !hsstate.hiderPos.equals(hiderPos));
    } else {
        return true;
    }
}

State::StateType HSState::getStateType() const {
    return State::HSState_Type;
}

string HSState::getHash() const {
    //TODO SHOULD return a hash number that represents exactly one State!!!
    stringstream ss;
    ss << 'S' << seekerPos.getHash() << "-H" << hiderPos.getHash();
    return ss.str();
}

void HSState::convPosToCont() {
    if (this->seekerPos.isSet() && !this->seekerPos.hasDouble()) this->seekerPos.add(0.5,0.5);
    if (this->hiderPos.isSet() && !this->hiderPos.hasDouble()) this->hiderPos.add(0.5,0.5);
}

void HSState::convPosToInt() {
    seekerPos.convertValuesToInt();
    hiderPos.convertValuesToInt();
}

void HSState::convertToObservation() {
    convPosToInt();
}

bool HSState::readFromPlayer(const PlayerInfo &playerInfo) {
    if (playerInfo.posRead) {
        seekerPos = playerInfo.currentPos;
        hiderPos = playerInfo.hiderObsPosWNoise;
        prob = playerInfo.useObsProb; //AG150804, instead of: hiderObsTrustProb;

        return true;
    } else {
        //player info was not up to date or not read
        seekerPos.clear();
        hiderPos.clear();
        prob = 0;

        return false;
    }
}

const HSState* HSState::castFromState(const State *state) {
    if (state==NULL) {
        return NULL;
    } else {
        return static_cast<const HSState*>(state);
    }
}


HSState* HSState::castFromState(State *state) {
    if (state==NULL) {
        return NULL;
    } else {
        return static_cast<HSState*>(state);
    }
}

