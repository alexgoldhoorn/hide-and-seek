#ifndef PARTICLEFILTERPLAYER_H
#define PARTICLEFILTERPLAYER_H

#include "Base/autoplayer.h"
#include "Filter/particlefilter.h"
#include "Filter/hsparticlefilterchecker.h"

//AG160211: re-use this code since, although might be slightly more inefficient, but only for init
#include "POMCP/hssimulatorcont.h"

/*!
 * \brief The ParticleFilter class follows the hider when (s)he is visible using a Particle Filter.
 */
class ParticleFilterPlayer : public AutoPlayer
{
public:
    /*!
     * \brief ParticleFilter constructor
     * \param params
     */
    ParticleFilterPlayer(SeekerHSParams* params);

    virtual ~ParticleFilterPlayer();

    virtual bool isSeeker() const;

    virtual std::string getName() const;

    virtual bool useGetAction();

    virtual bool tracksBelief() const;

    virtual double getBelief(int r, int c);

    virtual bool handles2Obs() const;


protected:
    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);

    virtual bool initBeliefRun();

    /*!
     * \brief genAllInitStates generates n initial states based on a robot pos, assuming that the person is not visible.
     * This method was copied from HSSimulator::genAllInitStates, TODO: maybe later join them, however Pos it does not (yet)
     * take into account the heading. This method does NOT take into account multiple positions of different seekers.
     * AG160211: updated to use HSSimulator, and to take into account multiple seekers.
     *
     * \param robot pos
     * \param n
     * \return the eigen vector
     */
    virtual Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic> genAllInitStates();

    /*!
     * \brief updateMapHist update the map (normalized) histogram (i.e. probability).
     * \param rowCount
     * \param colCount
     */
    void updateMapHist(int rowCount, int colCount);

    //! the kalman filter
    ParticleFilter _pf;

    //! the particle filter checker
    HSParticleFilterChecker* _pfChecker;

    //! the probability matrix
    double** _beliefProbMatrix;

    //! used to know if the values of the belief are up to date, otherwise they have to be re-generated
    bool _beliefProbMatrixUptodate;

    //AG160211
    //! use this simulator to generate initial points
    pomcp::HSSimulatorCont* _hsSimulator;

//private:

};
#endif // PARTICLEFILTERPLAYER_H
