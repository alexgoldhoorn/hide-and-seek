#ifndef HSSIMULATOR_H
#define HSSIMULATOR_H

#include "simulator.h"

#include "HSGame/gmap.h"
#include "HSGame/gplayer.h"

#include "hsstate.h"

class SeekerHSParams;


namespace pomcp {

/*!
 * \brief The HSSimulator class the hide&seek seeker simulator.
 * Note: the new State objects that are returned should be deleted by the caller of the function.
 */
class HSSimulator : public Simulator
{
public:
    static const char STATE_RUNNING = 0;
    static const char STATE_WIN_SEEKER = 1;
    static const char STATE_WIN_HIDER = 2;
    static const char STATE_TIE = 3;

    HSSimulator(GMap* map, SeekerHSParams* params);

    virtual unsigned int getNumObservations();

    virtual unsigned int getNumActions();

    virtual State* genRandomState(State* prevState=0, History* history=0);

    virtual State* genInitState();

    virtual void getActions(State* state, History* history, std::vector<int>& actions, bool smart=true);

    virtual void setInitialNodeValue(State* state, History* history, BaseNode* node, int action=-1);

    virtual double getDiscount() {
        return 0.95;
    }

    virtual State* step(State *state, int action, int &obsOut, double &reward, int obsIn=-1);

    //virtual State* stepAfterObs(State* state, int action, int obs, double& reward);

    virtual bool isFinal(State *state);

    /*!
     * \brief getGameStatus returns whether game still running / winner / tie
     * \param state
     * \return
     */
    virtual char getGameStatus(HSState* state);


    /*!
     * \brief rsetTimer (re)set timer
     * \param t
     */
    virtual void resetTimer(unsigned int t=0);

    /*!
     * \brief increaseTimer increase timer by 1 time step
     */
    virtual void increaseTimer();

    /*!
     * \brief setSeekerHiderPos set seeker and hider pos
     * \param seekerPos
     * \param hiderPos
     */
    virtual void setSeekerHiderPos(Pos seekerPos, Pos hiderPos, bool opponentVisible);

    /*!
     * \brief getPossibleNextPos get next possible pos's given a previous pos
     * \param prevPos
     * \return
     */
    virtual void getPossibleNextPos(Pos& prevPos, std::vector<Pos>& posVec, int obs = -1);

    /*!
     * \brief getImmediateReward get immediate reward
     * \param state
     * \return
     */
    virtual double getImmediateReward(State* state, State* nextState=NULL);

    /*!
     * \brief setMaxRewardValue max reward value, default: rows*cols
     * \param maxRew
     */
    virtual void setMaxRewardValue(unsigned int maxRew);

    /*!
     * \brief getObservation get the observation ID for the positions of seeker and hider
     * if the hider is not visible opponentVissible=false
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \return
     */
    virtual int getObservation(Pos seekerPos, Pos hiderPos, bool opponentVisible);



    //! unit tests
    void testAllFunctions();

protected:
    bool hiderWin(HSState* hsstate);

    bool seekerWin(HSState* hsstate);

    inline bool tie(HSState* hsstate);


    //the map
    GMap* _map;

    SeekerHSParams* _params;

    //! number of observations
    unsigned int _numObs;

    //! timer
    unsigned int _timer;

    //! player
    Player _player; //_seekerPlayer, _hiderPlayer;

    //! pos
    Pos _seekerPos, _hiderPos;

    //the following are cached values

    //! max value for reward
    unsigned int _maxRewValue;

    //! hidden observation
    int _hiddenObs;

    //! base
    Pos _basePos;

};

}

#endif // HSSIMULATOR_H
