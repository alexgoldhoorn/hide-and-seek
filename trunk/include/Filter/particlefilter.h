/****************************************/
/* (C) 2013 Alex Goldhoorn              */
/****************************************/

/* TODO
   - Make setters and getters for all matrices (?)
   */

#ifndef PARTICLE_FILTER_H
#define PARTICLE_FILTER_H

#include <random>
#include <vector>

#include <eigen3/Eigen/Dense>

//#include "Filter/particle.h"
#include "Filter/particlefilterchecker.h"
#include "Base/seekerhsparams.h"
#include "POMCP/hsobservation.h"

//#define NUM_ROWS 3

/*! Class that executes Particle Filter.
 * OLD: This version is using the linear Particle Filter, and uses a state of size 4: [x,y,v_x,v_y]^T
 * where v is the velocity.
 *
 * This version uses a linear Particle Filter wiht 3 state variabels: [x,y,theta]^T
 * And a random speed: N(1,stepNoiseStd)
 * and heading change: N(0,headNoiseStd)
 *
 * Initially the parameters are passed.
 *
 * From Thrun et al. Probabilistic Robotics, Table 4.3,
 * (and Straub2010)
 *
 * Wikipedia:
 * https://en.wikipedia.org/wiki/Particle_filter
 */
class ParticleFilter {
    public:
        //! number of rows of vector
        static const unsigned int NUM_ROWS=3;//made define, because didn't work for template return type

        ParticleFilter(SeekerHSParams* params, ParticleFilterChecker* pfWeighter = nullptr);

        /**
         * @brief initialize the state based on one observation, it
         * generates random points close to the observation.
         *
         * @param x the observation
         */
        void initState1(const Eigen::Vector3d& x);

        /**
         * @brief set the initial state to the passed state,
         * assumed to have numParticles particles
         *
         * @param X
         */
        void initState(Eigen::Matrix<double, NUM_ROWS, Eigen::Dynamic> X);

        /**
         * @brief predict step
         */
        void predict();

        /**
         * @brief update Update function which updates based on the observation.
         * Can allow updates if no observation given, but then the solverType should be SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES.
         * In the later case it, and if there is no observation, the weight is calculated based on the visibility of the robot.
         * If the obs is not given, and the particle is visible, the weight will be 0 or low.
         * @param obs
         * @param seekerPos
         * @param degeneracyRet [out]
         * @param weightMeanRet [out]
         * @param weightStdRet [out]
         */
        void update(const Eigen::Vector2d& obs,
                    const Pos& seekerPos,
                    double* degeneracyRet = nullptr,
                    double* weightMeanRet = nullptr,
                    double* weightStdRet = nullptr);


        //AG160215: use hsobservation
        void updateMulti(const pomcp::HSObservation& obs,
                    double* degeneracyRet = nullptr,
                    double* weightMeanRet = nullptr,
                    double* weightStdRet = nullptr);

        /**
         * @brief return all particles.
         *
         * @return all particles
         */
        const Eigen::Matrix<double, NUM_ROWS, Eigen::Dynamic>* getParticles() const;

        /**
         * @brief get the mean particle value
         *
         * @return returns mean particle
         */
        Eigen::Vector3d getMeanParticle() const;

        /**
         * @brief calculate the covariance of the particles
         *
         * @return the covariance
         */
        Eigen::Matrix3d getCovParticle() const;

        /**
         * @brief get the weights of the particles
         *
         * @return weights
         */
        Eigen::Matrix<double, Eigen::Dynamic, 1> getWeights() const;

        /**
         * @brief set the particle filter weighter class
         *
         * @param pfWeighter
         */
        void setParticleFilterChecker(ParticleFilterChecker* pfChecker);

        /*//! set step noise std.dev.
        void setStepNoiseStd(double stepNoiseStd);

        //! set heading noise std.dev.
        void setHeadNoiseStd(double headNoiseStd);

        //! set step size
        void setStepSize(double stepSize);

        //! set number of particles
        void setNumParticles(unsigned int numParticles);*/


    protected:
        //! normal random value
        double randomNormValue(double mean, double std);
        //! uniform random value
        double randomUnValue(double max, double min=0);


    private:
        /*//! stdev of step noise
        double _stepNoiseStd;
        //! stdev of heading noise
        double _headNoiseStd;
        //! number of particles
        unsigned int _numParticles;
        //! step size
        double _stepSize;*/

        //AG160121: using seekerhsparams instead

        //! params
        SeekerHSParams* _params;


        //! matrix of particles
        Eigen::Matrix<double, NUM_ROWS, Eigen::Dynamic> X;

        //! weights
        Eigen::Matrix<double, Eigen::Dynamic, 1> _weight;

        //! the checker of particles, and weighter
        ParticleFilterChecker* _pfChecker;

        // to generate random numbers
        std::random_device _randomDevice;
        std::mt19937 _randomGenerator;
        std::normal_distribution<double> _normDist;
        std::uniform_real_distribution<> _uniformProbDistr;

};


double inline ParticleFilter::randomNormValue(double mean, double std) {
    return _normDist(_randomGenerator)*std + mean;
}

double inline ParticleFilter::randomUnValue(double max, double min) {
    return _uniformProbDistr(_randomGenerator)*(max-min)+min;
}
#endif
