#ifndef BELIEF_H
#define BELIEF_H

#include "HSGame/gmap.h"

#include <vector>
//#include <map>
#include <string>

#include "POMCP/state.h"

namespace pomcp {

/*!
 * \brief The Belief class contains a number of samples of the belief.
 * When a belief contains more states, this is stored multiple times.
 * Note: 1) for the discrete case it could be made more efficient, but most of the time continuous states will
 * be used. 2) the trade-off is between using more memory (1 vector) or using a more complex retrieval and less memory (map)
 */
class Belief
{
public:

    Belief();

    ~Belief();

    /*!
     * \brief size number of items in the belief (the sum of the weights)
     * \return
     */
    std::size_t size() const;

    /*!
     * \brief operator [] return the item i
     * \param i
     * \return
     */
    State* operator[](std::size_t i);

    /*!
     * \brief add add state
     * \param state
     * \param deleteStatesAfterUsage deletes the states in the vector after usage
     *
     */
    void add(State* state, bool deleteStatesAfterUsage=false);

    /*!
     * \brief addAll add a list of states to the belief
     * \param states
     * \param deleteStatesAfterUsage deletes the states in the vector after usage
     */
    void addAll(std::vector<State*>& states, bool deleteStatesAfterUsage=false);

    /*!
     * \brief remove remove belief at index i
     * \param i
     */
    void remove(std::size_t i);


    /*!
     * \brief remove state from belief
     * \param state
     */
    void remove(State* state);

    /*!
     * \brief copy copy belief
     * \return
     */
    //Belief* copy();

    /*!
     * \brief copyFrom copy beliefs from passed belief
     * \param belief
     */
    //void copyFrom(Belief* belief);

    /*!
     * \brief isEmpty returns if belief is empty or not
     * \return
     */
    bool isEmpty();

    /*!
     * \brief getRandomSample get a random sample
     * \return
     */
    State* getRandomSample();

    /*!
     * \brief toString to string
     * \param maxItems show max items (-1 is all)
     * \return
     */
    std::string toString(int maxItems=-1);

    /*!
     * \brief getBeliefAvgAtPos get the belief at the passed position, this is an average based on the relative amount of belief points in that grid cell.
     * \param row
     * \param col
     * \return
     */
    double getBeliefAvgAtPos(int row, int col, int rowCount, int colCount);


protected:
    /*! NOTE: made for HSState only!!
     * \brief getMapHist get histogram of beliefs per map cell
     * \return
     */
    void updateMapHist(int rowCount, int colCount);

    //! vector with samples
    std::vector<State*> _sampleVector;

    //! the probability matrix
    double** _beliefProbMatrix;

    //! rowcount of beliefProbMatrix
    unsigned int _beliefProbMatrixRows;
    unsigned int _beliefProbMatrixCols;

    //! used to know if the values of the belief are up to date, otherwise they have to be re-generated
    bool _beliefProbMatrixUptodate;


};

}

#endif // BELIEF_H
