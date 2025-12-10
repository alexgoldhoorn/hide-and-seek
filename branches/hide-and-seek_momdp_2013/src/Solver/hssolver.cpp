#include "hssolver.h"

#include <iomanip>

#include "GlobalResource.h"

#include "MOMDP.h"
#include "BackupAlphaPlaneMOMDP.h"
#include "BackupBeliefValuePairMOMDP.h"



using namespace std;
using namespace momdp;


HSSolver::HSSolver() {
}

HSSolver::~HSSolver() {
}



void HSSolver::setup(SharedPointer<MOMDP> problem, SolverParams * solverParams)
{
    //this->policy = policy;
    this->_momdp = problem;
    this->_solverParams = solverParams;
}

//// ----- AG implemented methods ------/////

//AG120904: set belief vector
void HSSolver::setBelief(const BeliefWithState& belVec) {
    _currBelSt->sval = belVec.sval;	// if initial x value is not known, set sval as -1
    copy(*_currBelSt->bvec, *belVec.bvec);
}


//AG note: removed start belief of X since not used by us
#ifdef HSSOLVER_ACTSTATE_ON
int HSSolver::init(const BeliefWithState& startVec, int realYState /*, const SparseVector* startBeliefX*/, ofstream* streamOut)
#else
int HSSolver::init(const BeliefWithState& startVec, ofstream* streamOut)
#endif
{
    //log stream
    this->_logStream = streamOut;
    _enableFiling = false;
    if(streamOut == NULL) {
        _enableFiling = false;
    } else {
        _enableFiling = true;
    }

#ifdef HSSOLVER_ACTSTATE_ON
    // actual system state (refering to full knowledge), we can use it to test (?)
    _actStateCompl = new BeliefWithState();
    _actNewStateCompl = new BeliefWithState();
#endif

    // policy follower state
    // belief with state
    //SharedPointer<BeliefWithState> nextBelSt;
    _currBelSt = new BeliefWithState();// for policy follower based on known x value

    // set sval to -1 if x value is not known

    // belief over x. May not be used depending on model type and commandline flags, but declared here anyways.
    //DenseVector currBelX; // belief over x


    // get starting actStateCompl, the complete state (X and Y values)
    if (startVec.sval == -1) // check if the initial starting state for X is fixed
    {   // random starting state for X
        //AG111214: [put random distrib about X... but not for our case] --> SHOULD NOT OCCUR!!!
       //// actStateCompl->sval = chooseFromDistribution(*startBeliefX, ((double)rand()/RAND_MAX));
        //copy(currBelX, startBeliefX); //AG120104: disabled because different types AND assumed not to happen
        cout << "ERROR@OnlineLayeredMOMDP.init: startvec.sval=-1 -> SHOULD put a random distrib over X" <<endl;
        exit(EXIT_FAILURE);
    }
#ifdef HSSOLVER_ACTSTATE_ON
    else {   // initial starting state for X is fixed
        _actStateCompl->sval = startVec.sval;
    }
#endif
    //int currUnobsState = chooseFromDistribution(*startVec.bvec, ((double)rand()/RAND_MAX));
    // YOUR CODE HERE
    // now choose a starting unobserved state for the actual system


    //>AG111130

    //already by  them, check: this is random we need to calc the initial belief
    //TODO: unobserved=partially observable?  doesn't come from file?.. comes from current field??
#ifdef HSSOLVER_ACTSTATE_ON
    _currUnobsState = realYState; //chooseFromDistribution(*startVec.bvec, ((double)rand()/RAND_MAX));
#endif

    _belSize = startVec.bvec->filled(); //startvec is given start belief vector
    //ag note: filled gives # items in vector, size: 'capacity' (i assume: size>=filled)
    //<AG111130

#ifdef HSSOLVER_ACTSTATE_ON
    _actStateCompl->bvec->resize(_belSize);
    _actStateCompl->bvec->push_back(_currUnobsState, 1.0);
#endif

    //the current belief global var, for later use
    _currBelSt->sval = startVec.sval;	// if initial x value is not known, set sval as -1
    copy(*_currBelSt->bvec, *startVec.bvec);

    //ag120301: copy to pomdp as init belief
    _momdp->initialBeliefStval->sval = startVec.sval;
    copy(*_momdp->initialBeliefStval->bvec, *startVec.bvec);


    //TODO initbeliefY (?)

    //&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    //cout<<"INITIAL BELIEF"<<endl;
    //currBelSt->bvec->write(cout)<<endl;

    // we now have actStateCompl (starting stateUnobs&stateObs) for "actual state" system
    // "policy follower" system has currBelSt (starting beliefUnobs&stateObs) OR currBelSt (starting beliefUnobs&-1) and currBelX (starting beliefObs) if initial x is a belief and not a state

    _gamma = _momdp->getDiscount();

    _timeIter = 0;


    if (_enableFiling) {
        *streamOut << "init model ok" <<endl;
    }


    //cout << "init belief startvec: " <<startVec.bvec->ToString()<< endl;

    DEBUG_HS(printBelief(_currBelSt,"Initial belief"););

    return 0;
}



#ifdef HSSOLVER_ACTSTATE_ON
SharedPointer<BeliefWithState> HSSolver::updateBelief(int observ, int y_state, int x_state, int action)
#else
SharedPointer<BeliefWithState> HSSolver::updateBelief(int observ, int x_state, int action)
#endif
{
#ifdef HSSOLVER_ACTSTATE_ON
    assert(action>=0 && observ>=0 && x_state>=0 && y_state>=0);
    assert(action<_momdp->getNumActions() && observ<_momdp->observations->size() && y_state<_momdp->YStates->size() && x_state<_momdp->XStates->size());

    DEBUG_HS(cout<<"updateBelief @ iter " << _timeIter << ": observ: "<<observ<<", y_state: "<<y_state<<", x_state: "<<x_state<<", action: "<<action<<endl;);
#else
    assert(action>=0 && observ>=0 && x_state>=0);
    assert(action<_momdp->getNumActions() && observ<_momdp->observations->size() && x_state<_momdp->XStates->size());

    DEBUG_HS(cout<<"updateBelief @ iter " << _timeIter << ": observ: "<<observ<<", x_state: "<<x_state<<", action: "<<action<<endl;);
#endif

#ifdef HSSOLVER_ACTSTATE_ON
    // the actual next state for the observed variable
    _actNewStateCompl->sval = x_state;
    // the actual next state for the unobserved variable [AG111205: added, got from SimulationEngine.cpp]
    int newUnobsState = y_state; //chooseFromDistribution(actualActionUpdUnobs, ((double)rand()/RAND_MAX));

    //actual y state (for comparison (?))
    _actNewStateCompl->bvec->resize(_belSize);
    _actNewStateCompl->bvec->push_back(newUnobsState, 1.0);
#endif

    //set action
    _currAction = action;

    //ag:we know obs
    //belief_vector obsPoss;
    int currObservation = observ;


    /*if(enableFiling)
    {
        //initial states and belief, before any action
        if (timeIter == 0)
        {
            //actual X state, X might be a distribution at first time step
            map<string, string> obsState = problem->getFactoredObservedStatesSymbols(actStateCompl->sval);
            if (obsState.size()>0) {
                streamOut->width(4);*streamOut<<left<<"X"<<":";
                printTuple(obsState, streamOut);
            }

            //actual Y state
            streamOut->width(4);*streamOut<<left<<"Y"<<":";
            map<string, string> unobsState = problem->getFactoredUnobservedStatesSymbols(currUnobsState);
            printTuple(unobsState, streamOut);

            // if initial belief X is a distribution at first time step
            if (currBelSt->sval == -1) {
                SparseVector currBelXSparse;
                copy(currBelXSparse, currBelX);
                int mostProbX  = currBelXSparse.argmax(); 	//get the most probable Y state
                streamOut->width(4);*streamOut<<left<<"ML X"<<":";
                map<string, string> mostProbXState = problem->getFactoredObservedStatesSymbols(mostProbX);
                printTuple(mostProbXState, streamOut);
            }

            //initial belief Y state
            int mostProbY  = currBelSt->bvec->argmax(); 	//get the most probable Y state
            //double prob = currBelSt->bvec->operator()(mostProbY);	//get its probability
            streamOut->width(4);*streamOut<<left<<"ML Y"<<":";
            map<string, string> mostProbYState = problem->getFactoredUnobservedStatesSymbols(mostProbY);
            printTuple(mostProbYState, streamOut);
        }

        streamOut->width(4);*streamOut<<left<<"A"<<":";
        map<string, string> actState = problem->getActionsSymbols(currAction);
        printTuple(actState, streamOut);

        streamOut->width(4);*streamOut<<left<<"R"<<":";
        *streamOut << currReward<<endl;
    }*/

    // now that we have the action, state of observed variable, and observation,
    // we can update the belief of unobserved variable

    //AG: NOTE for bottom MOMDP the timeIter is NOT updated, therefore not active
    if (_currBelSt->sval==-1) { //therefore assume X-state is fully known also at start!
        cout << "ERROR @ OnlineLayeredMOMDP::updateBelief: currBelSt.sval=-1 -> ie use of init X belief -> NOT SUPORTED BY AG CODE"<<endl;
        exit(EXIT_FAILURE);
    }
    //  (should: implement to support x-belief at init)
    /*
    if (timeIter == 1) {  // check to see if the initial X is a distribution or a known state
        //ag120113: timeItter ==0 ->1 since we do it in different order..
        if (currBelSt->sval == -1) {// special case for first time step where X is a distribution
            nextBelSt = problem->beliefTransition->nextBelief(currBelSt->bvec, currBelX, currAction, currObservation, actNewStateCompl->sval);
        } else {
            nextBelSt = problem->beliefTransition->nextBelief(currBelSt, currAction, currObservation, actNewStateCompl->sval); //Most likely case
            nextBelSt->bvec->write(cout)<<endl;
        }
    } else {*/
    // UPDATE BELIEF
   // printBelief(currBelSt,"current belief before update");
    //cout<<"Updating belief"<<endl;

        _nextBelSt = _momdp->beliefTransition->nextBelief(_currBelSt, _currAction, currObservation, x_state);
        //cout <<"write vec:"<<endl;
        //nextBelSt->bvec->write(cout)<<endl;
    //}


   /* if(enableFiling)
    {

        //actual X state after action
        map<string, string> obsState = problem->getFactoredObservedStatesSymbols(actNewStateCompl->sval);
        if(obsState.size()>0){
            streamOut->width(4);*streamOut<<left<<"X"<<":";
            printTuple(obsState, streamOut);
        }

        //actual Y state after action
        streamOut->width(4);*streamOut<<left<<"Y"<<":";
        map<string, string> unobsState = problem->getFactoredUnobservedStatesSymbols(newUnobsState);
        printTuple(unobsState, streamOut);

        //observation after action
        streamOut->width(4);*streamOut<<left<<"O"<<":";
        map<string, string> obs = problem->getObservationsSymbols(currObservation);
        printTuple(obs, streamOut);

        //get most probable Y state from belief after applying action A
        int mostProbY  = currBelSt->bvec->argmax(); 	//get the most probable Y state
        //double prob = nextBelSt->bvec->operator()(mostProbY);	//get its probability
        streamOut->width(4);*streamOut<<left<<"ML Y"<<":";
        map<string, string> mostProbYState = problem->getFactoredUnobservedStatesSymbols(mostProbY);
        printTuple(mostProbYState, streamOut);

    }*/


    //AG120306: disabled actState udpate, ONLY used for reward caclulations, not done now
        /*
    printBelief(actStateCompl,"actual state prev");

    //update actual states
    currUnobsState = newUnobsState; //Y state, hidden
    actStateCompl->sval = actNewStateCompl->sval;
    copy(*actStateCompl->bvec, *actNewStateCompl->bvec);

    printBelief(actStateCompl,"actual state new");
    */


    DEBUG_HS(printBelief(_currBelSt,"Prev belief (curr)"););
    /*currBelSt->bvec->write(cout)<<endl;*/

    //update belief states
    copy(*_currBelSt->bvec, *_nextBelSt->bvec);
    _currBelSt->sval = _nextBelSt->sval;

    DEBUG_HS(printBelief(_currBelSt,"New belief (curr)"););
    /*currBelSt->bvec->write(cout)<<endl;
    printBelief(nextBelSt,"New belief (next)");
    nextBelSt->bvec->write(cout)<<endl;*/


    return _currBelSt;
}


//AG120904: added reward as return
int HSSolver::getBestAction(double& reward, vector<double>* maxValPerAction) { //, double& expReward) {
    assert(_policy!=NULL);
    // get action according to policy and current belief and state



    /* Mail Zhan Wei 10/9/12 (Re):

getBestAction gives the action with the best value based on the
alpha-vector policies alone (argmax b \cdot \alpha). The lookahead
refers to applying a 1-ply online tree search for the current belief
with values alpha-vector policies at the leaves. Its result is
strictly better than without look-ahead. There is a command line
option to toggle the lookahead. It existed in the first place due to
different benchmarking requirements.

      */

    //-----------------------------
/*    if (_timeIter == 0) {
        DEBUG_HS(cout << "first action"<<endl;);

        if(_solverParams->useLookahead) {
            if (_currBelSt->sval == -1) {// special case for first time step where X is a distribution
                _currAction = _policy->getBestActionLookAhead(_currBelSt->bvec, currBelX);
            } else {
                _currAction = _policy->getBestActionLookAhead(*_currBelSt, reward); //Most likely this case!!
            }
        } else {//Not using lookahead
            if (_currBelSt->sval == -1) { // special case for first time step where X is a distribution
                _currAction = _policy->getBestAction(_currBelSt->bvec, currBelX);
            } else {
                _currAction = _policy->getBestAction(*_currBelSt, reward);
            }
        }

        //cout << "action "<<currAction<<endl;

    } else {//Subsequent timeIters */
        if (_solverParams->useLookahead) {
            _currAction = _policy->getBestActionLookAhead(*_currBelSt, reward);
        } else {
            bool showValPerAct = (maxValPerAction!=NULL);
            _currAction = _policy->getBestAction(*_currBelSt, reward, showValPerAct, maxValPerAction); //AG121010: added max val per action vector
        }/*
    }*/

    if(_currAction < 0 )
    {
        cout << "ERROR @ OnlineLayeredMOMDP::getBestAction: You are using a MDP Policy, please make sure you are using a MDP policy together with one-step look ahead option turned on" << endl;
        return -1;
    }
    // this is the reward for the "actual state" system
    //cout <<"currrew"<<endl;

    //AG120304: skipping reward, since not directly used
    /*
    currReward = getReward(*actStateCompl, currAction);
    cout << "ok,rew="<<currReward<<endl;
    expReward += mult*currReward;
    mult *= gamma;
    reward += currReward;
    */
    // actualActionUpdUnobs and actualActionUpdObs are beliefs, based on prev state&state and transition matrices

    ////  belief_vector actualActionUpdUnobs(belSize), actualActionUpdObs(problem->XStates->size()) ;


//        }

//return firstAction;


    DEBUG_HS(cout << " getBestAction, action="<< _currAction <<" @ timeI="<<_timeIter<<endl;);


    _timeIter++;

    return _currAction;
}


void HSSolver::setPolicy(SharedPointer<AlphaVectorPolicy> policy) {
    _policy = policy;
}

bool HSSolver::readPolicy(const char *policyFile) {
    DEBUG_HS(cout << "Read policy " << policyFile <<endl;);
    //AG120514: create a new object
    //NOTE: should we delete previous or is this done by the SharedPointer?
    _policy = new AlphaVectorPolicy(_momdp);
    bool policyRead = _policy->readFromFile(policyFile);
    DEBUG_HS(cout << "Policy loaded: " << policyRead << endl;);
    return policyRead;
}




void HSSolver::setLog(HSLog* hslog) {
    _hslog=hslog;
}


SARSOPSolveState* HSSolver::solve(int generalTimeIter) //int main(int argc, char **argv)
{


    DEBUG_HS(cout << "solver"<<endl;);

    /*OutputParams op;
    if(GlobalResource::getInstance()->benchmarkMode)
    {
            if(GlobalResource::getInstance()->simNum == 0|| GlobalResource::getInstance()->simLen == 0)
            {
                    cout << "Benchmark Length and/or Number not set, please set them using option --simLen and --simNum" << endl;
                    exit(-1);
            }
    }*/

    //ag: is this required???
    GlobalResource::getInstance()->init();
    /*string baseName = GlobalResource::getInstance()->parseBaseNameWithoutPath(p->problemName);
    GlobalResource::getInstance()->setBaseName(baseName);*/

/*        //ag: register crtl-c handler:
#ifdef _MSC_VER
    registerCtrlHanler();
#else
    setSignalHandler(SIGINT, &sigIntHandler);
#endif */

   // printf("\nLoading the model ... \n  ");

    //Parser* parser = new Parser();

    /*GlobalResource::getInstance()->PBSolverPrePOMDPLoad();
    SharedPointer<MOMDP> problem (NULL);
    if(p->hardcodedProblem.length() ==0 )
    {
            problem = ParserSelector::loadProblem(p->problemName, *p);
    }
    else
    {
cout << "Unknown hard coded problem type : " << p->hardcodedProblem << endl;
exit(0);
    }

    double pomdpLoadTime = GlobalResource::getInstance()->PBSolverPostPOMDPLoad();
    printf("  loading time : %.2fs \n", pomdpLoadTime);*/
    //is this necessary??
    GlobalResource::getInstance()->problem = _momdp;

    //Getting a MDP solutions
    /*if(p->MDPSolution == true)
    {
            MDPSolution(problem, p);
            return 0;
    }

    if(p->QMDPSolution == true)
    {
            QMDPSolution(problem, p);
            return 0;
    }

    if(p->FIBSolution == true)
    {
            FIBSolution(problem, p);
            return 0;
    }

    if(GlobalResource::getInstance()->benchmarkMode)
    {
            srand(GlobalResource::getInstance()->randSeed);
            GlobalResource::getInstance()->expRewardRecord.resize(GlobalResource::getInstance()->simNum);
    }*/
    //decide which solver to create

    //PointBasedAlgorithm* solver;

    /*switch (p->strategy)
    {
    case S_SARSOP:
            {*/

    DEBUG_HS(cout << "Solver (APPL) parameters:" <<endl
            << " - targetPrecision: " << _solverParams->targetPrecision<<endl
            << " - mem limit:       " << _solverParams->memoryLimit<<endl
            << " - target trials:   " << _solverParams->targetTrials<<endl
            << " - Use look ahead:  " << _solverParams->useLookahead << endl;
            );

    DEBUG_HS(cout << "Init SARSOP ..."<<endl;);

                    SARSOP* sarsopSolver = NULL;
                    BackupAlphaPlaneMOMDP* lbBackup = new BackupAlphaPlaneMOMDP();
                    BackupBeliefValuePairMOMDP* ubBackup = new BackupBeliefValuePairMOMDP();

                    sarsopSolver = new SARSOP(_momdp, _solverParams);
                    //if (_hslog != NULL) sarsopSolver->setLogStream(_hslog->getOStream());//TODO!!! requires more than ostream, also a timing!!!! in the log
                    lbBackup->problem = _momdp;
                    sarsopSolver->lowerBoundBackup = lbBackup;

                    ((BackupAlphaPlaneMOMDP* )(sarsopSolver->lowerBoundBackup))->solver = sarsopSolver;

                    ubBackup->problem = _momdp;
                    sarsopSolver->upperBoundBackup = ubBackup;
                    //solver = sarsopSolver;

                    DEBUG_HS(cout << "SARSOP init done, now solving.."<<endl;);


                    //AG120426: delete unused vars

           /* }
            break;

            //case S_FSVI:
            //	solver = new FSVI(problem, p);
            //	break;

            //case S_GES:
            //	if(GlobalResource::getInstance()->migsPathFile != NULL)
            //	{
            //		if(GlobalResource::getInstance()->migsPathFileNum < 0 )
            //		{
            //			GlobalResource::getInstance()->migsPathFileNum = 10;
            //		}
            //		solver = new GES(problem, p, true);
            //	}
            //	else
            //	{
            //		solver = new GES(problem, p);
            //	}
            //	break;

    default:
            assert(0);// should never reach this point
    };*/


    //AG NOTE TODO: should load directly policy from solver
    //          now done save/load, because of complexity of storing/loading


    //file name policy
    stringstream ssFile;
    ssFile << "momdpOnlineTopPolicy"<< generalTimeIter<<".policy";
    _solverParams->policyFile = _solverParams->outPolicyFileName = ssFile.str();

    //AG121016: print solver params
    if (_solverParams->showParams) _solverParams->showAllParams();

    //solve the problem
    sarsopSolver->solve(_momdp);


    DEBUG_HS(cout << "solve done" << endl;);

    //now load policy
    _policy = new AlphaVectorPolicy(_momdp);
    DEBUG_HS(cout << "Loading policy " << _solverParams->outPolicyFileName <<endl;);
    bool policyRead = _policy->readFromFile(_solverParams->outPolicyFileName);
    DEBUG_HS(cout << "Policy loaded: " << policyRead << endl;);

    //TODO: here get the policy




    //AG120426: delete unused vars TODO!!!!!!!!!!!!!!!!

    /*SARSOP* sarsopSolver = NULL;
    BackupAlphaPlaneMOMDP* lbBackup = new BackupAlphaPlaneMOMDP();
    BackupBeliefValuePairMOMDP* ubBackup = new BackupBeliefValuePairMOMDP();

    sarsopSolver = new SARSOP(_momdp, _solverParams);
    //if (_hslog != NULL) sarsopSolver->setLogStream(_hslog->getOStream());//TODO!!! requires more than ostream, also a timing!!!! in the log
    lbBackup->problem = _momdp;
    sarsopSolver->lowerBoundBackup = lbBackup;

    ((BackupAlphaPlaneMOMDP* )(sarsopSolver->lowerBoundBackup))->solver = sarsopSolver;

    ubBackup->problem = _momdp;
    sarsopSolver->upperBoundBackup = ubBackup;
    //solver = sarsopSolver;

    DEBUG_HS(cout << "SARSOP init done, now solving.."<<endl;);
*/


    return &(sarsopSolver->lastSolverStats);

}


/* util functions */

void printTuple(map<string, string> tuple, ofstream* streamOut){
    *streamOut << "(";
    for(map<string, string>::iterator iter = tuple.begin() ; iter != tuple.end() ; )
    {
        *streamOut << iter->second;
        if(++iter!=tuple.end())
            *streamOut << ",";
    }
    *streamOut<<")" << endl;
}


void HSSolver::printBelief(SharedPointer<BeliefWithState> b, string name, int yState, GMap* gmap) {
    cout << "Belief " << name <<":"<<endl;
    cout << "   x-state: " << b->sval<<endl;
    cout << "   y-state: " << yState << "; distribution: " ;

    int szb = b->bvec->size();
    cout << "[size="<<szb<<"]: " <<endl;

    if (gmap == NULL) {
        // show belief without map
        for (int i=0;i<szb;i++){
            //ag120307: changed use of vector to () operator of SparseVector!!
            //          the sparsevector does NOT store all date directly (0s not)
            cout <<"  ["<<i<<"]:"<<(*b->bvec)(i);  //<<b->bvec->data[i].value ;
        }
        cout <<endl;
    } else {
        //show belief on top of map
        int i = 0;
        for (int r=0; r<gmap->rowCount(); r++) {
            for (int c=0; c<gmap->colCount(); c++) {
                if (gmap->isObstacle(r,c)) {
                    //print obst
                    cout << "[XXXXX]";
                } else {
                    //print others, check for hider/seeker/base
                    double p = (*b->bvec)(i);
                    char s1='[',s2=']';
                    if (i==b->sval) {
                        //seeker pos
                        s1=s2='S';
                    }
                    if (i==yState) {
                        s2='H';
                        if (s1!='S') s1='H';
                    }
                    if (gmap->isBase(r,c)) {
                        if (s1=='[' && s2==']' || s1==s2) {
                            s2 = 'B';
                            if (s1=='[') s1='B';
                        } else {
                            s1=s1-('A'-'a');
                            s2=s2-('A'-'a');
                        }
                    }
                    if (p>0) {
                        cout <<s1<<setprecision(3)<< setw(4)<<fixed<< p << s2;
                    } else {
                        cout <<s1<< "     " << s2;
                    }

                    i++;
                }
            }
            cout << endl;
        }
        //last line and undo print settings
        cout << endl << resetiosflags(ios_base::fixed) << setw(-1) <<setprecision(-1);
    }
}



void HSSolver::display(belief_vector& b, ostream& s)
{
    for(unsigned int i = 0; i < b.filled(); i++)
    {
        s << b.data[i].index << " -> " << b.data[i].value << endl;
    }
}
