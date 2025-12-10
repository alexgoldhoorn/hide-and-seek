#ifndef HSSOLVER_H
#define HSSOLVER_H

/*

Author:         Alex Goldhoorn
Created:        3/4/2012
Last change:    idem
  */


#include "Base/hsconfig.h"
#include "Utils/hslog.h"
#include "HSGame/gmap.h"

#include <vector>
#include <string>
#include "MOMDP.h"

#include "SARSOP.h"
#include "AlphaVectorPolicy.h"


//using namespace mompd;

/*! The base of the Hide&Seek MOMDP solver class. It uses the APPL library.
  */
class HSSolver
{
public:
    //! Print belief
    static void printBelief(SharedPointer<BeliefWithState>  b, string name, int yState=-1, GMap* gmap=NULL);

    //! Create solver
    HSSolver();

    ~HSSolver();

    //! Set the MOMDP and APPL solver parameters
    void setup(SharedPointer<MOMDP> momdp, SolverParams* solverParams);

    /* iters: number of repetitions of a simulation
       startVec: start belief for y, with startVec.sval being the start x state value
       realYState: real state of Y (to check it)
       startBeliefX: if startVec.sval==-1 then this is the start belief over the X-states
       ofstream: output stream for log/debug
       AG120304: removed startBeliefX since not used now and caused problems when NULL
      */
    //! Init: belief and real state
    //AG120426: use shared pointer
#ifdef HSSOLVER_ACTSTATE_ON
    int init(const BeliefWithState& startVec, int realYState, ofstream* logStream);
#else
    int init(const BeliefWithState& startVec, ofstream* logStream);
#endif

    //AG120904: set belief vector
    void setBelief(const BeliefWithState& belVec);

    //! Solve
    SARSOPSolveState* solve(int generalTimeIter);

    //! Display
    void display(belief_vector& b, ostream& s);

    //ag120303: update belief
    // after done action: observ, x_state
    //should be done after first action done
    //! Update the belief
#ifdef HSSOLVER_ACTSTATE_ON
    SharedPointer<BeliefWithState> updateBelief(int observ, int y_state, int x_state, int action);
#else
    SharedPointer<BeliefWithState> updateBelief(int observ, int x_state, int action);
#endif

    //ag120304: get the best action based on current belief
    //      note: reward not used
    //AG121010: added vector max val per action (out)
    //      note: only passed when best action lookahead (-bl) is not used
    //! Get best action for current belief and policy.
    int getBestAction(double& reward,vector<double>* maxValPerAction = NULL); //double& reward, double& expReward);

    //! Set logging system
    void setLog(HSLog* hslog);

    //! Set policy
    void setPolicy(SharedPointer<AlphaVectorPolicy> policy);

    //! Read policy from file
    bool readPolicy(const char* policyFile);

    //! Get current belief state
    SharedPointer<BeliefWithState> getCurrentBeliefState();

    //! Get MOMDP
    SharedPointer<MOMDP> getMOMDP();

private:

    //! MOMDP
    SharedPointer<MOMDP> _momdp;
    //! policy
    SharedPointer<AlphaVectorPolicy> _policy;
    //! Parameters of the SARSOP solver
    SolverParams * _solverParams;

    //params for model from 'runFor'
    //filing for the APPL
    bool _enableFiling;

    //! gamma used in the learning
    double _gamma;

    //! log stream (TODO: is this usefull)
    ofstream* _logStream;

    //! time iterator
    int _timeIter;

    //! Current action
    int _currAction;

    //! Current reward
    double currReward;

#ifdef HSSOLVER_ACTSTATE_ON  //AG130320: disabled
    //! actual system state
    SharedPointer<BeliefWithState> _actStateCompl;
    //! actual new system state
    SharedPointer<BeliefWithState> _actNewStateCompl;
    //! Current actual Y state
    int _currUnobsState;
    // ! Current actual X state
    //int _currObsState;
#endif

    //! Current belief state
    SharedPointer<BeliefWithState> _currBelSt;
    //! Next belief state
    SharedPointer<BeliefWithState> _nextBelSt;

    /*! Current belief state X.
        This is a vector, which in theory might be used by init X belief.
        But in the case of H&S X is always fully known.
      */
    DenseVector currBelX;

    //! Size of belief (ie # Y states)
    int _belSize;

    //! log
    HSLog *_hslog;

};

#endif // HSSOLVER_H
