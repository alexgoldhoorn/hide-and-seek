#ifndef HSSIMULATORCONT_H
#define HSSIMULATORCONT_H

#include "POMCP/hssimulator.h"

#include <random>

namespace pomcp {

/*!
 * \brief The HSSimulatorCont class Simulator that adds noise to position.
 */
class HSSimulatorCont : public HSSimulator
{
public:

    HSSimulatorCont(GMap *map, SeekerHSParams* params);

    ~HSSimulatorCont();

    virtual State* genInitState(const State* obs=0, const State* obs2=0, double obs1p=-1);

    virtual std::vector<State*> genAllInitStates(const State* obs=0, int n=0, const State* obs2=0, double obs1p=-1);

    virtual State* step(const State *state, int action, State*& obsOut, double &reward,  bool genOutObs=true,
                        const State* obsIn=NULL, const State *obs2In=NULL, double obs1p=-1);

    virtual bool checkNextStateObs(const State *state, const State *nextState, const State* obs);

    //AG150212: not used, disabled
    //virtual bool checkNextStateAction(State *state, State *nextState, int action);

    //AG140305: use of original simulator ... LATER TODO: handle inconsistent observations
    virtual bool isStateConsistentWithObs(const State* state, const State* obs, const State* obs2=NULL);

    virtual void resetParams();

protected:
    virtual bool hiderWin(const HSState* hsstate);

    /*!
     * \brief addNoiseToPos add noise to the seeker and hider position. If the hider position is not set, it won't be changed.
     * \param state
     * \param gausDistSeek
     * \param gausDistHider
     * \param useInt
     */
    virtual void addNoiseToPos(HSState* state, std::normal_distribution<double>& _gausDistSeek, std::normal_distribution<double>& _gausDistHider,
                               bool useInt=false);


    bool _addNoise;

    //create distributions
    std::normal_distribution<double> _gausDistSeek; //(0, _params->contNextSeekerStateStdDev);
    std::normal_distribution<double> _gausDistHider; //(0, _params->contNextHiderStateStdDev);
    std::normal_distribution<double> _gausDistObsSeek; //(0, _params->contSeekerObsStdDev);
    std::normal_distribution<double> _gausDistObsHider; //(0, _params->contHiderObsStdDev);
    //random pos in map
    std::uniform_int_distribution<> _uniformDistrRows;
    std::uniform_int_distribution<> _uniformDistrCols;

};

}

#endif // HSSIMULATORCONT_H
