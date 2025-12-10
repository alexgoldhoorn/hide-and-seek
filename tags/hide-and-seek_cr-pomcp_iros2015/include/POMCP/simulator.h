#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>

#include "POMCP/state.h"
#include "POMCP/history.h"
#include "POMCP/node.h"


namespace pomcp {

//AG150122: adapted to 2 seekers
/*!
 * \brief The Simulator class simulates the POMDP
 */
class Simulator
{
public:
    //Simulator() {}

    virtual ~Simulator() {}

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
     */ //AG140112: Not used, therefore disabled
    //virtual State* genRandomState(State* prevState=0, History* history=0)=0;

    /*!
     * \brief genInitState generate a random init state. If the observation obs is set (with a position), and obs2 is null,
     * then obs is returned. If obs and obs2 are boths then the probabilities are used, choosing obs with a probability
     * of obs1p, otherwise obs2 is returned. If obs is not set, then the hider is not visible to the seeker, thus a random
     * position will be returned which is not visible to the seeker (according to ray-tracing).
     * \param obs observation, if not set completely random state used
     * \param obs2 observation of seeker 2 (if available)
     * \param obs1p prob. of observation 1
     * \return
     */
    virtual State* genInitState(const State* obs=0, const State* obs2=0, double obs1p=-1)=0;

    //AG131031: get all init states
    /*!
     * \brief genAllInitStates generate all random init states as described in genInitState. If the states are the same only 1 will be
     * returned, otherwise n as maximum.
     * \param obs observation, if not set completely random state used
     * \param n (max) number of init states, if 0 the 'exact' amount will be used (i.e. depending on the number of (in)visible cells)
     * \param obs2 observation of seeker 2 (if available)
     * \param obs1p prob. of observation 1
     * \return
     */
    virtual std::vector<State*> genAllInitStates(const State* obs=0, int n=0, const State* obs2=0, double obs1p=-1)=0;

    /*!
     * \brief getActions get the possible actions in the given state, if smart=true then
     * only 'smart' actions will be returned.
     * \param state
     * \param history
     * \param actions (out)
     * \param smart (optional,in) tells if the actions are 'smart'
     */
    virtual void getActions(const State* state, History* history, std::vector<int>& actions, bool smart=true)=0;

    /*!
     * \brief setInitialNodeValue get the initial value and count for the node if the action is
     *set it calculates the value depending on the action done. value and count are set in the node
     * \param state
     * \param history
     * \param node  node to change count and value for
     * \param action (default: -1, not set)
     */
    virtual void setInitialNodeValue(const State* state, History* history, BaseNode* node, int action=-1)=0;

    /*!
     * \brief getDiscount the discount
     * \return
     */
    virtual double getDiscount()=0;

    //AG150122
    /*!
     * \brief Step returns the next state when doing an action, also returns the observation and (direct) reward. if an observation
     * is passed (obsIn) then this will be used to generate a consistent step.
     * \param state current state
     * \param action action done
     * \param obs [out] observation
     * \param reward [out] direct reward
     * \param genOutObs [in] generate an output observation, if no i
     * \param obsIn [in] observation done after having done the action (or NULL if not used to consistent check the next resulting state)     
     * \param obs2In [in] observation of other seeker
     * \param obs1p [in] probability of choosing obs1
     * \return  new state or NULL if no state could be found that is consistent with a given obsIn (if not NULL)
     */
    virtual State* step(const State *state, int action, State*& obsOut, double &reward, bool genOutObs=true, const State* obsIn=NULL,
                        const State *obs2In=NULL, double obs1p=-1)=0;

    /*!
     * \brief isFinal is a final state
     * \return
     */
    virtual bool isFinal(const State* state)=0;

    /*!
     * \brief checkNextStateObs check if the next state is consistent with the state and the observation
     * \param state
     * \param nextState
     * \param obs
     * \return
     */
    virtual bool checkNextStateObs(const State* state, const State* nextState, const State* obs)=0;

    /*!
     * \brief checkNextStateAction check if the next state is consistent with the state and the observation
     * \param state
     * \param nextState
     * \param action
     * \return
     */
    //AG150212: not used, disabled
    //virtual bool checkNextStateAction(State* state, State* nextState, int action)=0;

    /*!
     * \brief isStateConsistentWithObs checks if the state is consistent with the obs
     * \param state
     * \param obs
     * \param obs2 the observation as seen from another seeker
     * \return
     */
    virtual bool isStateConsistentWithObs(const State* state, const State* obs, const State* obs2=NULL)=0;

    //AG140108:
    /*!
     * \brief getActionForRollout returns an action for the roll-out, it can follow different types of policies (e.g. random or smart).
     * \param state
     * \param history
     */
    virtual int getActionForRollout(const State* state, History* history)=0;

    /*!
     * \brief resetParams called by MCTS before a learning phase is started of several simulations, in this
     *  function the parameters from (SeekerHSParams) can be
     */
    virtual void resetParams()=0;

    /*!
     * \brief rsetTimer (re)set timer
     * \param t
     */
    virtual void resetTimer(unsigned int t=0);

    /*!
     * \brief increaseTimer increase timer by 1 time step
     */
    virtual void increaseTimer();

protected:
    //! timer
    unsigned long _time;

};


}


#endif // SIMULATOR_H
