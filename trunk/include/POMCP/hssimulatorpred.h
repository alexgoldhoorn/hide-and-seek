#ifndef HSSIMULATORPRED_H
#define HSSIMULATORPRED_H

#include "POMCP/hssimulatorcont.h"
#include "PeoplePrediction/personpathpredconsumer.h"
#include "Utils/normalbivardist.h"


namespace pomcp {

/*!
 * \brief The HSSimulatorPred class Simulator that uses the predicted steps of the person.
 *
 * TODO: add (possible) use of heading
 */
class HSSimulatorPred : public HSSimulatorCont, public PersonPathPredConsumer
{
public:
//AG141106: moved to seekerhsparams.h such that it the header is not necessary for
//    static const double CONT_NEXT_HIDER_HALT_PROB      = 0.1;
//    static const double CONT_USE_HIDER_PRED_STEP_PROB  = 0.4;


    //TODO: need to known in which 'step' the person is

    HSSimulatorPred(GMap *map, SeekerHSParams* params);

    ~HSSimulatorPred();

    //virtual State* genRandomState(State* prevState=0, History* history=0);

    //virtual State* genInitState(State* obs=0);

    //virtual std::vector<State*> genAllInitStates(State* obs=0, int n=0);

    //virtual void getActions(State* state, History* history, std::vector<int>& actions, bool smart=true);

    //virtual void setInitialNodeValue(State* state, History* history, BaseNode* node, int action=-1);


    /*virtual State* step(const State *state, int action, State*& obsOut, double &reward,  bool genOutObs=true, const State* obsIn=NULL,
                        const State *obs2In=NULL, double obs1p=-1);*/
    virtual State* step(const State *state, int action, State*& obsOut, double &reward, bool genOutObs=true,
                        const Observation* obs=NULL);

    //virtual State* stepAfterObs(State* state, int action, int obs, double& reward);

    //virtual bool isFinal(State *state);

    /*!
     * \brief getGameStatus returns whether game still running / winner / tie
     * \param state
     * \return
     */
    //virtual char getGameStatus(HSState* state);




    /*!
     * \brief setSeekerHiderPos set seeker and hider pos
     * \param seekerPos
     * \param hiderPos
     */ //AG131104: depricated by State* obs parameter of getInitState / getAllInitStates
    //virtual void setSeekerHiderPos(Pos seekerPos, Pos hiderPos, bool opponentVisible);

    /*!
     * \brief getPossibleNextPos get next possible pos's given a previous pos
     * \param prevPos
     * \return
     */
    //virtual void getPossibleNextPos(Pos& prevPos, std::vector<Pos>& posVec, HSState* obs = NULL);

    /*!
     * \brief getImmediateReward get immediate reward
     * \param state
     * \return
     */
    //virtual double getImmediateReward(State* state, State* nextState=NULL);

    //virtual bool checkNextStateObs(State *state, State *nextState, State* obs);

    //virtual bool checkNextStateAction(State *state, State *nextState, int action);

    //AG140305: use of original simulator ... LATER TODO: handle inconsistent observations
    //virtual bool isStateConsistentWithObs(State* state, State* obs);

    //! unit tests
    //virtual void testAllFunctions();

    virtual void resetParams();


    /*!
     * \brief getRandomPersonPoint get next random person point
     * \param r_in
     * \param c_in
     * \param r_out
     * \param c_out
     */
    virtual void getRandomPersonPoint(const double& r_in, const double &c_in, double& r_out, double &c_out);


    virtual void resetTimer(unsigned int t=0);

    //virtual void testAllFunctions();

protected:
    //virtual bool hiderWin(HSState* hsstate);

    /*!
     * \brief addNoiseToPos add noise to the seeker and hider position. If the hider position is not set, it won't be changed.
     * \param state
     * \param gausDistSeek
     * \param gausDistHider
     * \param useInt
     */
    /*virtual void addNoiseToPos(HSState* state, std::normal_distribution<double>& _gausDistSeek, std::normal_distribution<double>& _gausDistHider,
                               bool useInt=false);*/

    /*!
     * \brief addBivarNormalNoise adds bivariate normal noise to the meanPos. It checks if the new position is a legal position
     *  (i.e. inside the map and not an obstacle).
     * \param dist
     * \param meanPos
     * \param outPos
     */
    virtual void addBivarNormalNoise(NormalBivariateDist& dist, const Pos& meanPos, Pos& outPos, std::string whereDebug);


    //bool _addNoise;

    //create distributions
    //std::normal_distribution<double> _gaus;

    //! covarince for prediction step
    //Eigen::MatrixXd _covarPredStep;
    //! covariance for normal step
    /*Eigen::MatrixXd _covarStep;
    //! mean used for the location
    Eigen::VectorXd _mean;*/

    //! distribution of next hider pos
    NormalBivariateDist _nextHiderStepDistr;
    //! distribution of next seeker pos
    NormalBivariateDist _nextSeekerStepDistr;
    //! distribution of hider pos obs
    NormalBivariateDist _hiderObsDistr;
    //! distribution of seeker pos obs
    NormalBivariateDist _seekerObsDistr;

    uint _ngetRandP, _ngetRandPPred;
};

}


#endif // HSSIMULATORPRED_H
