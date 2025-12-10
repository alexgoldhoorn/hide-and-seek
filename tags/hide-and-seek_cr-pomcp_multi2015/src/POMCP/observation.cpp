#include "POMCP/observation.h"

using namespace pomcp;

Observation::Observation() : State(), isInconsistent(false), prob(-1) {
}

Observation::~Observation(){
}
