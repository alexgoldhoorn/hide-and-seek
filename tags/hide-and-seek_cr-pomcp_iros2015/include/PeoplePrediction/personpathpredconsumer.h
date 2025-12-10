#ifndef PERSONPATHPREDCONSUMER_H
#define PERSONPATHPREDCONSUMER_H

#include <vector>

#include "Utils/normalbivardist.h"

/*!
 * \brief The PersonPathPredConsumer class This abstract class should be inherited from by classes that consume the
 * path prediction from G. Ferrer (/iri-lab/labrobotica/restricted/algorithms/people_prediction/branches/force_planning).
 */
class PersonPathPredConsumer {
public:
    PersonPathPredConsumer();
    virtual ~PersonPathPredConsumer();

    /*!
     * \brief setPredPath Set the prediction path: mean (x,y) and covariance. Both vectors contain vectors of double, the index indicates the
     * time stamp. The first item is the next time step. delta t is defined in people_prediction algorithm.
     * \param pointMeanVec vector of vector of doubles, each subvector contains (u=mean): u_x, u_y
     * \param pointCovVec  vector of vector of doubles, each subvector contains (s=sigma): s_xx, s_xy, s_yx, s_yy  (first top row, then bottom row)
     */
    virtual void setPredPath(const std::vector<std::vector<double>>& pointMeanVec, const std::vector<std::vector<double>>& pointCovVec);


    /*!
     * \brief getPathPoint get the path point predicted, at time t+1
     * \param t index, or time-1 (i.e. next time step is 0, 2 timesteps in future is t=1)
     * \param mean [out] the center of the position
     * \param covar [out] the covariance of the position
     */
    //virtual void getPathPoint(unsigned int t, Eigen::VectorXd& mean, Eigen::MatrixXd& covar);

    /*!
     * \brief getRandPathPoint get a random point (normal, using mean, covar)
     * \param t index, or time (1 is next)
     * \param r new row
     * \param c new col
     */
    virtual void getRandPredPoint(long t, double &r, double &c);

    /*!
     * \brief predictionAvailable is there a prediction available for the time step t, location r,c
     *  location r,c represents the 'current' time step t-1 (t=0 is next, t=1, next next, etc.)
     * \param t the time step, t=1 represents the next step
     * \param r
     * \param c
     * \return
     */
    virtual bool predictionAvailable(long t, const double &r, const double &c);

    /*!
     * \brief resetTime reset time to t
     * \param t new time (default 0)
     */
    //void resetTime(unsigned int t=0);

    /*!
     * \brief increaseTime increase time with 1
     */
    //void increaseTime();

protected:


    //! vector of future steps (index 0 is the next time step, i.e. t+1 where t is now)
    std::vector<std::vector<double>> _pointMeanVec;
    //! vector of cov. of future steps (index 0 is the next time step, i.e. t+1 where t is now)
    std::vector<std::vector<double>> _pointCovVec;

    //! normal multivar. distr. to sample points
    NormalBivariateDist _normalBivarDist;

    //! time step, t=0 is now
    //unsigned int _t;

    //! in last step used prediction
    bool _lastStepUsedPred;

};

#endif // PERSONPATHPREDCONSUMER_H
