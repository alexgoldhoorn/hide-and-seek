#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>

#include "POMCP/state.h"
#include "POMCP/history.h"
#include "POMCP/node.h"
#include "POMCP/observation.h"


namespace pomcp {

//AG150122: adapted to 2 seekers
/*!
 * \brief The Simulator class simulates the POMDP
 */
class Simulator
{
public:
    virtual ~Simulator();

    /*!
     * \brief getNumObservations get number of observations
     * \return
     */
    virtual unsigned int getNumObservations()=0;

    /*!
     * \brief getNumActions get the number of actions
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
     * \brief genInitState generate a random init state based on the passed observations, the probability of choosing an observation
     * is the State.prob variable.
     * \param obs  [optional] the observation
     * \return new random init state
     */
    virtual State* genInitState(const Observation* obs=NULL)=0;

    /*!
     * \brief genAllInitStates generate all random init states as described in genInitState. If the states are the same only 1 will be
     * returned, otherwise n as maximum.
     * \param obs  [optional] the observation
     * \param n [optional] (max) number of init states, if 0 the 'exact' amount will be used (i.e. depending on the number of (in)visible cells)
     * \return
     */
    virtual std::vector<State*> genAllInitStates(const Observation* obs=NULL, int n=0)=0;

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

    //AG150611
    /*!
     * \brief Step returns the next state when doing an action, also returns the observation and (direct) reward.
     * If observations (own thisSeekerObs, or more, otherObsVec) are passed then
     * is passed (obsIn) then this will be used to generate a consistent step.
     * \param state current state
     * \param action action done
     * \param obs [out] observation
     * \param reward [out] direct reward
     * \param genOutObs [in,optional] generate an output observation, if no i
     * \param obs [in,optional] observation done after having done the action (or NULL if not used to consistent check the next resulting state)
     * \return  new state or NULL if no state could be found that is consistent with given observations (if not NULL)
     */
    virtual State* step(const State *state, int action, State*& obsOut, double &reward, bool genOutObs=true,
                        const Observation* obs=NULL)=0;


    /*!
     * \brief getImmediateReward get the immediate reward for going from state -> nextState (if both set),
     * otherwise from nextState if set, otherwise from state.
     * \param state
     * \param nextState
     * \return
     */
    virtual double getImmediateReward(const State* state, const State* nextState=NULL)=0;

    /*!
     * \brief isFinal is a final state
     * \return
     */
    virtual bool isFinal(const State* state)=0;

    //AG150615: multi robot upd
    /*!
     * \brief checkNextStateObs check if the next state is consistent with the state and the observation
     * \param state
     * \param nextState
     * \param obs
     * \return
     */
    virtual bool checkNextStateObs(const State* state, const State* nextState, const Observation* obs) =0;

    virtual bool checkNextStateObs(const State* state, const State* nextState, const State* obs)=0;

    //AG150615: multi robot upd
    /*!
     * \brief isStateConsistentWithObsc hecks if the state is consistent with the obs
     * \param state
     * \param obs
     * \return
     */
    virtual bool isStateConsistentWithObs(const State* state, const Observation* obs) =0;

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
