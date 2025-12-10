#include "POMCP/hsstate.h"

#include <sstream>


using namespace pomcp;
using namespace std;

HSState::HSState() : State() {
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
    /*state->hiderPos = hiderPos;
    state->seekerPos = seekerPos;*/
    return state;
}


string HSState::toString() const {
    stringstream sstr;
    sstr << "<S:"<<seekerPos.toString()<<";H:"<<hiderPos.toString()<<">";
    return sstr.str();
}


bool HSState::hiderVisible() const {
    return hiderPos.isSet();
}


bool HSState::operator <(const State& other) const {
    const HSState& hsstate = static_cast<const HSState&>(other);

    return (seekerPos<hsstate.seekerPos || (seekerPos==hsstate.seekerPos && hiderPos<hsstate.hiderPos));
}

bool HSState::operator >(const State& other) const {
    const HSState& hsstate = static_cast<const HSState&>(other);

    return (seekerPos>hsstate.seekerPos || (seekerPos==hsstate.seekerPos && hiderPos>hsstate.hiderPos));
}

bool HSState::operator ==(const State& other) const {
    const HSState& hsstate = static_cast<const HSState&>(other);

    return (hsstate.seekerPos.equals(seekerPos) && hsstate.hiderPos.equals(hiderPos));
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
