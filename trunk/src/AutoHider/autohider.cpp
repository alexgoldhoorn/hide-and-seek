#include "AutoHider/autohider.h"
#include "exceptions.h"

AutoHider::AutoHider(SeekerHSParams* params) : AutoPlayer(params)
{
}

AutoHider::~AutoHider() {
}

double AutoHider::getBelief(int r, int c) {
    return 0;
}

bool AutoHider::tracksBelief() const {
    return false;
}

bool AutoHider::isSeeker() const {
    return false;
}
