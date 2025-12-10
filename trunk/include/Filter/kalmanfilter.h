/****************************************/
/* (C) 2015 Alex Goldhoorn              */
/****************************************/

/* TODO 
   - Make setters and getters for all matrices (?)
   */

#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <eigen3/Eigen/Dense>

//! add Matrix2_4d (2x4) matrix
namespace Eigen {
    typedef Matrix<double, 2, 4> Matrix2_4d;
    typedef Matrix<double, 4, 2> Matrix4_2d;
}


/*! Class that executes Kalman Filter.
 * This version is using the linear Kalman Filter, and uses a state of size 4: [x,y,v_x,v_y]^T 
 * where v is the velocity.
 *
 * Initially the parameters are passed.
 *
 * Copied from Wikipedia:
 * https://en.wikipedia.org/wiki/Kalman_filter
 */ 
class KalmanFilter {
    public:
        static Eigen::Matrix4d DEFAULT_F();
        static Eigen::Matrix2_4d DEFAULT_H();
        static Eigen::Matrix4d DEFAULT_Q();
        static Eigen::Matrix2d DEFAULT_R();
        static Eigen::Vector4d DEFAULT_x();
        static Eigen::Matrix4d DEFAULT_P();

        KalmanFilter(const Eigen::Matrix4d& F   = DEFAULT_F(), 
                     const Eigen::Matrix2_4d& H = DEFAULT_H(), 
                     const Eigen::Matrix4d& Q   = DEFAULT_Q(), 
                     const Eigen::Matrix2d& R   = DEFAULT_R()
        );

        /**
         * @brief Initilize state and covariance.
         *
         * @param x the state
         * @param P covariance
         */
        void initState(const Eigen::Vector4d& x, // = DEFAULT_x(), 
                       const Eigen::Matrix4d& P = DEFAULT_P()
        );


        /**
         * @brief predict step
         */
        void predict();

        /**
         * @brief update based on observation
         *
         * @param obs
         */
        void update(const Eigen::Vector2d& obs);

        const Eigen::Vector4d& getState() const;

        const Eigen::Matrix4d& getStateCov() const;

        const Eigen::Matrix4d& getMotionNoise() const;

        const Eigen::Matrix2d& getObsNoise() const;

        void setState(const Eigen::Vector4d& x);

        void setStateCov(const Eigen::Matrix4d& P);

        void setMotionNoise(const Eigen::Matrix4d& Q);

        void setObsNoise(const Eigen::Matrix2d& R);

    protected:
        //! motion noise
        Eigen::Matrix4d Q;
        //! observation noise
        Eigen::Matrix2d R;
        //! current state 
        Eigen::Vector4d x;
        //! current covariance
        Eigen::Matrix4d P;
        //! motion vector
        Eigen::Vector4d B;
        //! next state function
        Eigen::Matrix4d F;
        //! motion function
        Eigen::Matrix2_4d H;
};

#endif
