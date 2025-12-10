#include "hsstate.h"

#include <sstream>

using namespace pomcp;
using namespace std;

HSState::HSState() : State() {
}

HSState::HSState(Pos seekerPos, Pos hiderPos) : State() {
    this->seekerPos.set(seekerPos);
    this->hiderPos.set(hiderPos);
}

HSState::HSState(int seekerRow, int seekerCol, int hiderRow, int hiderCol) : State() {
    seekerPos.row = seekerRow;
    seekerPos.col = seekerCol;
    hiderPos.row = hiderRow;
    hiderPos.col = hiderCol;
}



State* HSState::copy() {
    HSState* state = new HSState();
    state->hiderPos = hiderPos;
    state->seekerPos = seekerPos;
    return state;
}


string HSState::toString() {
    stringstream sstr;
    sstr << "<S:"<<seekerPos.toString()<<";H:"<<hiderPos.toString()<<">";
    return sstr.str();
}


bool HSState::hiderVisible() {
    return hiderPos.isSet();
}
