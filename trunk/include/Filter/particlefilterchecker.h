#ifndef PARTICLE_FILTER_CHECKER_H
#define PARTICLE_FILTER_CHECKER_H

#include <eigen3/Eigen/Dense>
#include "HSGame/pos.h"
#include "POMCP/observation.h"

/**
 * @brief Default Particle Filter checker, contains a weight calculation function, and check of particle validity.
 */
class ParticleFilterChecker {
    public:
        virtual ~ParticleFilterChecker();

        /**
         * @brief the default weight function
         * Note the particle has as third row the heading, the observation does not.
         *
         * @param particle particle to compare
         * @param obsPos the observation
         * @param sekerPos the seeker position
         * @param obs the observation containing info of all players
         *
         *
         * @return
         */
        virtual double calcWeight(const Eigen::Vector3d& particle, const Pos& obsPos, const Pos& seekerPos, const pomcp::Observation& obs);


        /**
         * @brief the default validity check
         *
         * @param particle
         *
         * @return
         */
        virtual bool isValid(const Eigen::Vector3d& particle);

};
#endif
