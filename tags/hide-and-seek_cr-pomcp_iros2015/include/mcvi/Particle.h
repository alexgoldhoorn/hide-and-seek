#ifndef __PARTICLE_H
#define __PARTICLE_H

#include <vector>
#include "MState.h"

struct Particle
{
    MState state;
    double weight;
    long pathLength;

    Particle(MState st, long pL, double weight): state(st),
                                                weight(weight),
                                                pathLength(pL) {}
    Particle() {}
};

struct ParticleStore
{
    // Total sum of all immediate reward (from calling model.sample())
    // of all particle
    double currSum;
    std::vector<Particle> particles;
};

#endif
