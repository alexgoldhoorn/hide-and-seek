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

    virtual State* genInitState(const Observation *obs=NULL);

    virtual std::vector<State*> genAllInitStates(const Observation *obs=NULL, int n=0);

    virtual State* step(const State *state, int action, State*& obsOut, double &reward, bool genOutObs=true,
                        const Observation* obs=NULL);

    virtual bool checkNextStateObs(const State *state, const State *nextState, const State* obs);
    virtual bool checkNextStateObs(const State* state, const State* nextState, const Observation* obs) const;

    //AG140305: use of original simulator ... LATER TODO: handle inconsistent observations
    virtual bool isStateConsistentWithObs(const State* state, const Observation* obs) ;

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
    std::normal_distribution<double> _gausDistSeek;
    std::normal_distribution<double> _gausDistHider;
    std::normal_distribution<double> _gausDistObsSeek;
    std::normal_distribution<double> _gausDistObsHider;
    //random pos in map
    std::uniform_int_distribution<> _uniformDistrRows;
    std::uniform_int_distribution<> _uniformDistrCols;

};

}

#endif // HSSIMULATORCONT_H
