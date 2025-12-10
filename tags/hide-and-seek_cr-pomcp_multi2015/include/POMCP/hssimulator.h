#ifndef HSSIMULATOR_H
#define HSSIMULATOR_H

#include "POMCP/simulator.h"

#include <random>


#include "HSGame/gmap.h"
#include "HSGame/gplayer.h"

#include "POMCP/hsstate.h"
#include "POMCP/hsobservation.h"

class SeekerHSParams;

#ifndef SMARTSEEKER_H
class SmartSeeker;
#endif


namespace pomcp {

/*!
 * \brief The HSSimulator class the hide&seek seeker simulator.
 * Note: the new State objects that are returned should be deleted by the caller of the function.
 * Note2: this class can use continuous states, but it does not add noise
 */
class HSSimulator : public Simulator
{
public:    

    HSSimulator(GMap* map, SeekerHSParams* params);

    virtual ~HSSimulator();

    virtual unsigned int getNumObservations();

    virtual unsigned int getNumActions();

    //virtual State* genInitState(const State* obs=0, const State* obs2=0, double obs1p=-1);
    virtual State* genInitState(const Observation *obs=NULL);

    //virtual std::vector<State*> genAllInitStates(const State* obs=0, int n=0, const State* obs2=0, double obs1p=-1);
    //! Complexity: O_getInivisibPoints -> O(n)
    virtual std::vector<State*> genAllInitStates(const Observation *obs=NULL, int n=0);

    virtual void getActions(const State* state, History* history, std::vector<int>& actions, bool smart=true);

    virtual void setInitialNodeValue(const State* state, History* history, BaseNode* node, int action=-1);

    virtual double getDiscount();

    /*virtual State* step(const State *state, int action, State*& obsOut, double &reward, bool genOutObs=true, const State* obsIn=NULL,
                        const State *obs2In=NULL, double obs1p=-1);*/
    //! Takes 1 step, and uses observation obs if passed.
    //! Also uses some randomness based on p_fp, and prob. of random act.
    //! Complexity: O(n) (max)
    virtual State* step(const State *state, int action, State*& obsOut, double &reward, bool genOutObs=true,
                        const Observation* obs=NULL);

    virtual bool isFinal(const State *state);

    /*!
     * \brief getGameStatus returns whether game still running / winner / tie
     * \param state
     * \return
     */
    virtual char getGameStatus(const HSState* state);

    /*!
     * \brief getPossibleNextHiderPos get next possible poses of the hider given a previous pos (and possibl observations)
     * \param prevPos the previous position
     * \param posVec [out] the vector of posible positions
     * \param obs [in] if set, it takes into account the observations
     * \return
     */
    virtual void getPossibleNextHiderPos(const Pos& prevPos, std::vector<Pos>& posVec, const HSObservation* obs = NULL);
    //virtual void getPossibleNextHiderPos(const Pos& prevPos, std::vector<Pos>& posVec, const HSState* obs = NULL, const HSState* obs2 = NULL);

    /*!
     * \brief getImmediateReward get immediate reward
     * \param state
     * \return
     */
    virtual double getImmediateReward(const State* state, const State* nextState=NULL);

    /*!
     * \brief setMaxRewardValue max reward value, default: rows*cols
     * \param maxRew
     */
    virtual void setMaxRewardValue(unsigned int maxRew);

    virtual bool checkNextStateObs(const State *state, const State *nextState, const State* obs);

    virtual bool checkNextStateObs(const State* state, const State* nextState, const Observation* obs) const;

    //AG150212: not used, disabled
    //virtual bool checkNextStateAction(State *state, State *nextState, int action);

    //virtual bool isStateConsistentWithObs(const State* state, const State* obs, const State* obs2=NULL);
    virtual bool isStateConsistentWithObs(const State* state, const Observation* obs) const;

    /*!
     * \brief isHiderStateConsistentWithObsState checks if the hiderPos variable of the state is consistent with an observed state. The
     * state is consistent if it is either the same as the observation obs
     * \param hsState the state
     * \param hsObsState the observed state
     * \return is the state consistent with the observed state
     */
    //virtual bool isHiderStateConsistentWithObsState(const HSState* hsState, const HSState* hsObsState);

    virtual int getActionForRollout(const State* state, History* history);

    //! unit tests
    //virtual void testAllFunctions();

    virtual void resetParams();

protected:
    /*inline*/ bool hiderWin(const HSState* hsstate);

    /*inline*/ bool seekerWin(const HSState* hsstate);

    // /*inline*/ bool tie(HSState* hsstate);

    Pos findClosestPos(const std::vector<Pos>& posVec, const Pos& toPos);


    //the map
    GMap* _map;

    SeekerHSParams* _params;

    //! number of observations
    unsigned int _numObs;

    //! player
    Player _player; //_seekerPlayer, _hiderPlayer;

    //! pos
    //Pos _seekerPos, _hiderPos;

    //the following are cached values

    //! max value for reward
    unsigned int _maxRewValue;

    // ! hidden observation
    //AG130831: not used anymore, now using HSState
    //int _hiddenObs;

    //! base
    Pos _basePos;

    //! random device
    std::random_device _randomDevice;
    //! random generator
    std::mt19937 _randomGenerator; //(_randomDevice());
    //! probability distribution, 0 to 1
    std::uniform_real_distribution<> _uniformProbDistr;
    //! probability distribution, 0 to 2pi
    std::uniform_real_distribution<> _uniformDirDistr;

    //AG140108
    //! smart seeker for the roll-out policy
    SmartSeeker* _smartSeeker;

    //AG150616
    //! smart seeker hider player
    PlayerInfo* _smartSeekerHiderPlayer;
};

}

#endif // HSSIMULATOR_H
