#ifndef MOMDP_H
#define MOMDP_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>

#include "Observations.h"
#include "Actions.h"
#include "States.h"

#include "Rewards.h"

#include "ObservationProbabilities.h"
#include "StateTransitionX.h"
#include "StateTransitionXY.h"
#include "StateTransitionXXpY.h"
#include "StateTransitionY.h"

#include "Belief.h"
#include "BeliefTransition.h"

#include "POMDP.h"
#include "MObject.h"
#include "MathLib.h"
#include "FacmodelStructs.h"
#include "POMDPLayer.h"
#include "State.h"
#include "ObsAct.h"
#include "Cache.h"

/* AG120228:

> > p(x'|x,y,a): XTrans->getMatrix(a,x) -> SparseMatrix[y,x']
> > p(y'|x,y,a,x'): YTrans->getMatrix(a,x,x') -> SparseMatrix[y,y']
> > p(o|x',y',a): obsProb->getMatrix(a,x') -> SparseMatrix[y', o]
> > R(x,y,a): rewards->getMatrix(x)-> SparseMatrix[y,a]

  */


using namespace std;
using namespace momdp;
namespace momdp {
class FactoredPomdp;

class MOMDP : public MObject {
private:
    vector<State> stateList;
    vector<ObsAct> observationList;
    vector<ObsAct> actionList;
    vector<ObsAct> rewardList;


    virtual void deleteMatrixVector(vector<SharedPointer<SparseMatrix> > *m);

public:
    SharedPointer<SparseVector> initialBeliefY;
    vector<SharedPointer<SparseVector> > initialBeliefYByX;

    Cache<SharedPointer<SparseMatrix> > cache;

    MOMDP(void); // default constructor


    static SharedPointer<MOMDP> convertMOMDPFromPOMDP(POMDP* pomdpProblem);
    static SharedPointer<MOMDP> convertMOMDPFromPOMDPX(FactoredPomdp* factoredPomdp, bool assumeUnknownFlag,unsigned int probType);
    //ag120228: generate MOMDP based on its variables
    static SharedPointer<MOMDP> generateMOMDP(/*States*	XStates,
                              States*	YStates,
                              Actions* actions,
                              Observations* observations,*/ //generate these, based on next, note that only for 'text'/debug (?)
        vector<State> stateList,
        vector<ObsAct> observationList,
        vector<ObsAct> actionList,
        vector<ObsAct> rewardList,
        StateTransitionX* XTrans,
        StateTransitionY* YTrans,
        ObservationProbabilities* obsProb,
        Rewards* rewards,
        SharedPointer<BeliefWithState>  initialBeliefStval,
        SharedPointer<DenseVector> initialBeliefX, //note: this is when there is not a full belief of X at init
        SharedPointer<SparseVector> initialBeliefY,
        double discount,
        bool assumeUnknownFlag,unsigned int probType,
        SharedPointer<MOMDP> momdp);//AG121025: added momdp pointer

    //ag120302: generate new momdp, based on old
    static SharedPointer<MOMDP> generateMOMDP(SharedPointer<MOMDP> momdp);

    virtual ~MOMDP(void);

    virtual string ToString();
    // functions
    BeliefTransition* beliefTransition;

    // data

    REAL_VALUE discount;

    SharedPointer<BeliefWithState>  initialBeliefStval;
    SharedPointer<DenseVector> initialBeliefX;

    States*	XStates;
    States*	YStates;
    Actions* actions;
    Observations* observations;

    StateTransitionX* XTrans;
    StateTransitionY* YTrans;
    ObservationProbabilities* obsProb;
    Rewards* rewards;

    // TODO: remove the following pomdpXXX variables
    SharedPointer<SparseMatrix> pomdpR;
    vector<SharedPointer<SparseMatrix> > *pomdpT, *pomdpTtr, *pomdpO;

    vector<vector<int> > isPOMDPTerminalState;

    // If some parts do not support intraslice yet, they can check this and complain
    bool hasIntraslice;

    inline SharedPointer<SparseVector> getInitialBeliefY(int obsState) {
        if( initialBeliefY != NULL ) {
            return initialBeliefY;
        } else {
            return initialBeliefYByX[obsState];
        }
    }

    virtual REAL_VALUE getDiscount () {
        return discount;
    }

    virtual bool hasPOMDPMatrices();
    virtual void deletePOMDPMatrices();

    virtual obsState_prob_vector& getObsStateProbVector(obsState_prob_vector& result, SharedPointer<belief_vector>& belY, DenseVector& belX, int a);
    virtual obsState_prob_vector& getObsStateProbVector(obsState_prob_vector& result, BeliefWithState& b, int a);
    // Xn: next X
    virtual SparseVector& getJointUnobsStateProbVector(SparseVector& result, SharedPointer<BeliefWithState> b, int a, int Xn);

    virtual int getNumActions();
    virtual int getBeliefSize();

    virtual bool getIsTerminalState(BeliefWithState &b);

    virtual void getObsProbVectorFast(obs_prob_vector& result, int a, int Xn, SparseVector& tmp1);
    virtual void getObsProbVector(obs_prob_vector& result, const BeliefWithState& b, int a, int Xn);
    virtual void getObsProbVector(obs_prob_vector& result, SharedPointer<belief_vector>& belY, obsState_prob_vector& belX, int a, int Xn);

    //
    virtual map<string, string> getActionsSymbols(int actionNum);
    virtual map<string, string> getFactoredObservedStatesSymbols(int stateNum) ;
    virtual map<string, string> getFactoredUnobservedStatesSymbols(int stateNum) ;
    virtual map<string, string> getObservationsSymbols(int observationNum) ;

    //ag120302: get functions for default vars
    vector<State> getStateList() {
        return stateList;
    }
    vector<ObsAct> getObservationList() {
        return observationList;
    }
    vector<ObsAct> getActionList() {
        return actionList;
    }
    vector<ObsAct> getRewardList() {
        return rewardList;
    }
};
}
#endif
