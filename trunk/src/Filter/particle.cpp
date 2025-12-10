#include "Filter/particle.h"
#include <cmath>
#include "exceptions.h"

using namespace std;

Particle::Particle() : x(0), y(0), h(0) {
    throw CException(_HERE_, "this class is not to be used");
}

Particle::Particle(double x, double y, double h) {
    this->x = x;
    this->y = y;
    this->h = h;
}

bool Particle::advance(double speed, double headingNoise) {
    //speed
    return false;
}

Particle Particle::readSensor() {
    return Particle();
}


