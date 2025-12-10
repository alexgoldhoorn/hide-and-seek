#include "Filter/kalmanfilter.h"

#include <cmath>
#include <cassert>
#include <iostream>

//TODO: CHECK http://stackoverflow.com/questions/25999407/initialize-a-constant-eigen-matrix-in-a-header-file

Eigen::Matrix4d KalmanFilter::DEFAULT_F() {
    return (Eigen::Matrix4d() <<
            1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1).finished();

}
Eigen::Matrix2_4d KalmanFilter::DEFAULT_H() {
    return (Eigen::Matrix2_4d() <<
            1, 0, 1, 0,
            0, 1, 0, 1).finished();
}
Eigen::Matrix4d KalmanFilter::DEFAULT_Q() {
    return Eigen::Matrix4d::Identity();
}
Eigen::Matrix2d KalmanFilter::DEFAULT_R() {
    return Eigen::Matrix2d::Constant(0.0001);
}
Eigen::Vector4d KalmanFilter::DEFAULT_x() {
    return Eigen::Vector4d::Zero();
}
Eigen::Matrix4d KalmanFilter::DEFAULT_P() {
    return Eigen::Matrix4d::Identity()*1000.0;
}


KalmanFilter::KalmanFilter(const Eigen::Matrix4d& F, const Eigen::Matrix2_4d& H, const Eigen::Matrix4d& Q, const Eigen::Matrix2d& R) {
    this->F = F;
    this->H = H;
    this->Q = Q;
    this->R = R;
}

void KalmanFilter::initState(const Eigen::Vector4d &x, const Eigen::Matrix4d& P) {
    this->x = x;
    this->P = P;
}

void KalmanFilter::update(const Eigen::Vector2d& obs) {
    //innovation or measurment residual
    Eigen::Vector2d y = obs - this->H * this->x;
    //innovation (or residual) covariance
    Eigen::Matrix2d S = this->H * this->P * this->H.transpose() + this->R;
    //calculate inverse
    Eigen::Matrix2d Si = S.inverse(); 
    //optimal Kalman gain
    Eigen::Matrix4_2d K = this->P * this->H.transpose() * Si;
    //updated (a posteriori) state estimate
    this->x = this->x + K * y;
    //updated (a posteriori) estimate covariance
    this->P = (Eigen::Matrix4d::Identity() - K * this->H) * this->P;
}

void KalmanFilter::predict() {
    //predict (a priori) state estimate
    this->x = this->F * this->x; //TODO: add motion?
    //predict (a priori) estimate covariance
    this->P = this->F * this->P * this->F.transpose() + this->Q;
}

const Eigen::Vector4d& KalmanFilter::getState() const {
    return this->x;
}

const Eigen::Matrix4d& KalmanFilter::getStateCov() const {
    return this->P;
}

const Eigen::Matrix4d& KalmanFilter::getMotionNoise() const {
    return this->Q;
}

const Eigen::Matrix2d& KalmanFilter::getObsNoise() const {
    return this->R;
}

void KalmanFilter::setState(const Eigen::Vector4d& x) {
    this->x = x;
}

void KalmanFilter::setStateCov(const Eigen::Matrix4d& P) {
    this->P = P;
}

void KalmanFilter::setMotionNoise(const Eigen::Matrix4d& Q) {
    this->Q = Q;
}

void KalmanFilter::setObsNoise(const Eigen::Matrix2d& R) {
    this->R = R;
}


