#ifndef STATE_H
#define STATE_H

#include <string>

namespace pomcp {

/*!
 * \brief The State class Abstract state class
 */
class State
{
public:
    State();

    /*!
     * \brief copy copy state
     * \return
     */
    virtual State* copy()=0;

    /*!
     * \brief toString string description
     * \return
     */
    virtual std::string toString()=0;
};

}

#endif // STATE_H
