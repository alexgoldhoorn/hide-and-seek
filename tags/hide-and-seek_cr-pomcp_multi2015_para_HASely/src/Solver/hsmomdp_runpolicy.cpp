
#include "Solver/hsmomdp_runpolicy.h"

#include "Utils/generic.h"

#include "exceptions.h"

RunPolicyHSMOMDP::RunPolicyHSMOMDP(SeekerHSParams* params) : HSMOMDP(params) {
    DEBUG_HS_INIT(cout << "RunPolicyHSMOMDP generated"<<endl;);
}

RunPolicyHSMOMDP::~RunPolicyHSMOMDP() {
}


bool RunPolicyHSMOMDP::init(const char* pomdpFile, const char* logOutFile, const char* policyFile, const char *timeLog) {
    DEBUG_HS(cout << "*** RunPolicyHSMOMDP.init ***"<<endl;);

    //AG130506: check if policy file exists!!
    if (!fileExists(policyFile)) {
        cout << "ERROR: the policy file '"<<policyFile<<"' does not exist!"<<endl;
        exit(-1);
    }

    bool ok = HSMOMDP::initBase(pomdpFile, logOutFile, timeLog);

    if (ok) {
        ok = _solver.readPolicy(policyFile);
    }

    return ok;
}



int RunPolicyHSMOMDP::getNextAction(int observ, int x_state) {
    DEBUG_HS(cout << "***-- RunPolicyHSMOMDP.getNextAction --***"<<endl;);

    //1. Update belief
    if (_action!=-1) { //i.e. not the first step
        updateBelief(observ,x_state);
    }

    //2. Get action
    return getBestAction();
}


int RunPolicyHSMOMDP::getBestAction() {
    DEBUG_HS(cout << "*** RunPolicyHSMOMDP.getBestAction ***"<<endl;);

    /*_hslog->print("Solve");
    int timerI = _timer.startTimer();

    //solve MOMDP (and get the policy)
    SARSOPSolveState* solveStats = _solver.solve(_timeIter);

    _hslog->print(_timer.stopTimer(timerI));
    _hslog->print(_timeIter);

    //  Time   |#Trial |#Backup |LBound    |UBound    |Precision  |#Alphas |#Beliefs
    _hslog->print(solveStats->elapsedTime);
    _hslog->print(solveStats->numTrials);
    _hslog->print(solveStats->numBackups);
    _hslog->print(solveStats->lowBound);
    _hslog->print(solveStats->upBound);
    _hslog->print(solveStats->precision);
    _hslog->print(solveStats->numAlpha);
    _hslog->printLine(solveStats->numBeliefs);
*/

    _hslog->print("Best Action");
    int timerI = _timer.startTimer();

    //find the best action to do for our state
    double reward;//ag120904: reward
    _action = _solver.getBestAction(reward);

    _hslog->print(_timer.stopTimer(timerI));
    _hslog->print(_timeIter);
    _hslog->print(_action);
    _hslog->printLine(reward);//ag120904: reward

    _timeIter++;

    return _action;
}


vector<int> RunPolicyHSMOMDP::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone, int n) {
    throw CException(_HERE_, "RunPolicyHSMOMDP::getNextMultipleActions: not yet implemented");
}


std::string RunPolicyHSMOMDP::getName() const {
    return "HSMOMDP_run_policy";
}

