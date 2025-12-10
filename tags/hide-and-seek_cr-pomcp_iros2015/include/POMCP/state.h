#ifndef STATE_H
#define STATE_H

#include <string>

namespace pomcp {

/*!
 * \brief The State class Abstract state class
 */
struct State
{
public:
    State();

    virtual ~State();

    /*!
     * \brief copy copy state
     * \return
     */
    virtual State* copy() const=0;

    /*!
     * \brief toString string description
     * \return
     */
    virtual std::string toString() const=0;

    /*!
     * \brief convertToObservation converts the state into an observation,
     * for an observed state the values could be continuous, the observations should be limited (e.g. integer)
     */
    virtual void convertToObservation()=0;


    virtual bool operator <(const State& other) const=0;

    virtual bool operator >(const State& other) const=0;

    virtual bool operator ==(const State& other) const=0;

    virtual std::string getHash() const=0; //TODO SHOULD return a hash number that represents exactly one State!!!

    //AG150325
    //! is this state inconsistent (used by simulator/mcts)
    bool isInconsistent;
};

}

#endif // STATE_H
