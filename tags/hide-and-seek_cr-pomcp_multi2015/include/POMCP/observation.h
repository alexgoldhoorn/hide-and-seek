#ifndef OBSERVATION_H
#define OBSERVATION_H

#include "POMCP/state.h"

namespace pomcp {

/*!
 * \brief Observation class Abstract class for the observations.
 */
struct Observation : public State
{
public:
    Observation();

    virtual ~Observation();

    /*!
     * \brief getUpdateObservationState get the observation State which has to be used for the update process
     * Note: this is necessary because MCTS needs one observation, while this Observation class could contain more states.
     * \return
     */
    virtual const State* getUpdateObservationState() const=0;

    //AG150325
    //! is this state inconsistent (used by simulator/mcts)
    bool isInconsistent;


    //AG150611
    //! probability of trusting in case of observation, or belief
    double prob;
};

}


#endif // OBSERVATION_H

