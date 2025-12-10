#ifndef BELIEF_H
#define BELIEF_H

#include <vector>
#include <string>

#include "state.h"

namespace pomcp {

/*!
 * \brief The Belief class contains a number of samples of the belief
 */
class Belief
{
public:
    Belief();

    ~Belief();

    /*!
     * \brief size number of items in the belief
     * \return
     */
    std::size_t size();

    /*!
     * \brief operator [] return the item i
     * \param i
     * \return
     */
    State* operator[](int i);

    /*!
     * \brief add add state
     * \param state
     * \param score [default=1.0] for state
     */
    void add(State* state, double score=1.0);

    /*!
     * \brief copy copy belief
     * \return
     */
    Belief* copy();

    /*!
     * \brief copyFrom copy beliefs from passed belief
     * \param belief
     */
    void copyFrom(Belief* belief);

    /*!
     * \brief isEmpty returns if belief is empty or not
     * \return
     */
    bool isEmpty();

    /*!
     * \brief getRandomSample get a random sample
     * \return
     */
    State* getRandomSample(bool useScore=true);

    /*!
     * \brief toString to string
     * \param maxItems show max items (-1 is all)
     * \return
     */
    std::string toString(int maxItems=-1);


protected:
    //! returns index using scores as weights
    size_t weightedRandomIndex();

    //! samples
    std::vector<State*> _sampleVector;

    //AG130731: added score (/weight)
    //! the vector that represents a score (kind of probability) for each sample
    std::vector<double> _scoreVector;

    //sum of 'scores'
    double _scoreSum;
};

}

#endif // BELIEF_H
