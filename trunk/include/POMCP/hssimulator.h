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

    virtual State* genInitState(const Observation *obs=NULL);

    //! Complexity: O_getInivisibPoints -> O(n)
    virtual std::vector<State*> genAllInitStates(const Observation *obs=NULL, int n=0);

    virtual void getActions(const State* state, History* history, std::vector<int>& actions, bool smart=true);

    virtual void setInitialNodeValue(const State* state, History* history, BaseNode* node, int action=-1);

    virtual double getDiscount();

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

    virtual bool checkNextStateObs(const State* state, const State* nextState, const Observation* obs) ;

    virtual bool isStateConsistentWithObs(const State* state, const Observation* obs) ;

    virtual int getActionForRollout(const State* state, History* history);

    virtual void resetParams();

protected:
    bool hiderWin(const HSState* hsstate);

    bool seekerWin(const HSState* hsstate);

    /*!
     * \brief findClosestPos find position in posVec closest to toPos
     * \param posVec
     * \param toPos
     * \return
     */
    Pos findClosestPos(const std::vector<Pos>& posVec, const Pos& toPos);

    /*!
     * \brief genRandomState generate a random pos that is not visible
     * \param obs
     * \param invisPosVec list of not visible  poses (not taking into account distance)
     * \param seekerPosVec list of seeker poses (form obs)
     * \return
     */
    virtual HSState* genRandomHSState(const HSObservation* hsObs, const std::vector<Pos>& invisPosVec,
                                    const std::vector<Pos>& seekerPosVec); //AG160215: cannot be const due to use of random generator
    //! the map
    GMap* _map;

    //! params
    SeekerHSParams* _params;

    //! number of observations
    unsigned int _numObs;

    //! player
    Player _player;

    //the following are cached values

    //! max value for reward
    unsigned int _maxRewValue;

    //! base
    Pos _basePos;

    //! random device
    std::random_device _randomDevice;
    //! random generator
    std::mt19937 _randomGenerator;
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
