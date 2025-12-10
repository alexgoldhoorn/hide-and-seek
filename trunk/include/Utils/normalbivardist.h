#ifndef NORMALBIVARDIST_H
#define NORMALBIVARDIST_H

#include <vector>
#include <random>
#include <eigen3/Eigen/Dense>
#include "HSGame/pos.h"

/*!
 * \brief The NormalBivariateDist class Normal Bivariate Distribution class, can be used to sample.
 * Based on example from: http://stackoverflow.com/questions/6142576/sample-from-multivariate-normal-gaussian-distribution-in-c
 */
class NormalBivariateDist {
public:
    NormalBivariateDist();
    virtual ~NormalBivariateDist();

    /*!
     * \brief setMeanCovar set the mean and covariance for the random variables
     * \param meanVec
     * \param covVec
     */
    void setMeanCovar(const std::vector<double>& meanVec, const std::vector<double>& covVec);

    /*!
     * \brief setMean set mean of distr.
     * \param meanVec
     */
    void setMean(const std::vector<double>& meanVec);

    /*!
     * \brief setMean set mean
     * \param x
     * \param y
     */
    void setMean(double x, double y);

    /*!
     * \brief setMean set mean with pos
     * \param pos
     */
    void setMean(const Pos& pos);

    /*!
     * \brief setCovar set cov. of distr.
     * \param covVec
     */
    void setCovar(const std::vector<double>& covVec);

    /*!
     * \brief getRandPoint get a random point
     * \param x [out] random x/col point
     * \param y [out] random y/row point
     */
    void getRandPoint(double &x, double &y);

    /*!
     * \brief getRandPoint
     * \param p
     */
    Pos getRandPoint();

    /*!
     * \brief getRandPoint get a random point with given mean and covariance
     * \param meanVec
     * \param covVec
     * \param x [out] random x point
     * \param y [out] random y point
     */
    void getRandPoint(const std::vector<double>& meanVec, const std::vector<double>& covVec, double &x, double &y);


protected:
    //! random device
    std::random_device _randomDevice;
    //! random generator
    std::mt19937 _randomGenerator;

    //! to create random values
    std::normal_distribution<double> _normalDist;

    //! mean used for the location
    Eigen::VectorXd _mean;

    //! covarince for prediction step
    Eigen::MatrixXd _covar;

    //! covarince for prediction step
    Eigen::MatrixXd _normTransform;

    //! sample matrix
    Eigen::MatrixXd _sampMat;
};

#endif // NORMALBIVARDIST_H
