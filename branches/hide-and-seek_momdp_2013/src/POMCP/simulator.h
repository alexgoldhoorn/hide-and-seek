#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>

#include "state.h"
#include "history.h"
#include "node.h"


namespace pomcp {

/*!
 * \brief The Simulator class simulates the POMDP
 */
class Simulator
{
public:
    Simulator();

    /*!
     * \brief getNumObservations get number of observations
     * \return
     */
    virtual unsigned int getNumObservations()=0;

    /*!
     * \brief getNumActions
     * \return
     */
    virtual unsigned int getNumActions()=0;

    /*!
     * \brief genRandomState returns a random (real) state
     * i.e. it is a possible real state based on the given parameters (previous state and history).
     * If no previous state is given a completely random state will be returned.
     * \param prevState (default null)
     * \param history (default null)
     * \return new state
     */
    virtual State* genRandomState(State* prevState=0, History* history=0)=0;

    /*!
     * \brief genInitState
     * \return
     */
    virtual State* genInitState()=0;

    /*!
     * \brief getActions get the possible actions in the given state, if smart=true then
     * only 'smart' actions will be returned.
     * \param state
     * \param history
     * \param actions (out)
     * \param smart (optional,in) tells if the actions are 'smart'
     */
    virtual void getActions(State* state, History* history, std::vector<int>& actions, bool smart=true)=0;

    /*!
     * \brief setInitialNodeValue get the initial value and count for the node if the action is
     *set it calculates the value depending on the action done. value and count are set in the node
     * \param state
     * \param history
     * \param node  node to change count and value for
     * \param action (default: -1, not set)
     */
    virtual void setInitialNodeValue(State* state, History* history, BaseNode* node, int action=-1)=0;

    /*!
     * \brief getDiscount the discount
     * \return
     */
    virtual double getDiscount()=0;

    /*!
     * \brief Step returns the next state when doing an action, also returns the observation and (direct) reward. if an observation
     * is passed (obsIn) then this will be used to generate a consistent step.
     * \param state current state
     * \param action action done
     * \param obs [out] observation
     * \param reward [out] direct reward
     * \param obsIn [in] observation done (or -1 if not used)
     * \return  new state
     */
    virtual State* step(State *state, int action, int &obsOut, double &reward, int obsIn=-1)=0;



    /*!
     * \brief isFinal is a final state
     * \return
     */
    virtual bool isFinal(State* state)=0;
};


}


#endif // SIMULATOR_H
