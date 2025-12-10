#include "Filter/particlefilterchecker.h"
#include <cmath>
#include <iostream>

#include "Base/hsconfig.h"

using namespace std;

ParticleFilterChecker::~ParticleFilterChecker() {
}

double ParticleFilterChecker::calcWeight(const Eigen::Vector3d &particle, const Pos &obsPos, const Pos &seekerPos, const pomcp::Observation& obs) {
    static constexpr double sigmab2 = 0.9*0.9; //2*10*10; //*0.9*0.9;//TODO this has to be configurable

    Eigen::Vector2d obsEV = obsPos.toEigenVector<2>();

    DEBUG_FILTER(cout << "WeightCalc: particle="<<particle<<", obs="<<obsEV<<flush;);

    double err2 = pow(particle(0)-obsEV(0),2)+pow(particle(1)-obsEV(1),2);

    double w = exp(-err2 / sigmab2);

    DEBUG_FILTER(cout << ",err2="<<err2<<", w="<<w<<endl;);

    return w;
}

bool ParticleFilterChecker::isValid(const Eigen::Vector3d &particle) {
    return true;
}
