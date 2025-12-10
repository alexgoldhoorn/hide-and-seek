#include "POMCP/simulator.h"

using namespace pomcp;

void Simulator::resetTimer(unsigned int t) {
    _time = t;
}

void Simulator::increaseTimer() {
    _time++;
}

