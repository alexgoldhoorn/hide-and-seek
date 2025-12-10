#ifndef KALMANFILTERPLAYER_H
#define KALMANFILTERPLAYER_H

#include "Base/autoplayer.h"
#include "Filter/kalmanfilter.h"

/*!
 * \brief The KalmanFilter class follows the hider when (s)he is visible using a Kalman Filter.
 * NOTE: we assume that the person is
 */
class KalmanFilterPlayer : public AutoPlayer
{
public:
    /*!
     * \brief KalmanFilter constructor
     * \param params
     */
    KalmanFilterPlayer(SeekerHSParams* params);

    virtual ~KalmanFilterPlayer();

    virtual bool isSeeker() const;

    virtual std::string getName() const;

    virtual bool useGetAction();

protected:
    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);

    virtual bool initBeliefRun();

    //virtual double getNextDirection(bool &haltAction);

    //! the kalman filter
    KalmanFilter _kf;


//private:

};
#endif // KALMANFILTER_H
