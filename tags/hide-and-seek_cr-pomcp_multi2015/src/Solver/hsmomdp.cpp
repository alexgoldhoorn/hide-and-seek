#include "Solver/hsmomdp.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdio>

#include "ParserSelector.h"
#include "GlobalResource.h"
#include "SARSOP.h" //for stats

#include "Utils/generic.h"
#include "Solver/hssolver.h"



HSMOMDP::HSMOMDP(SeekerHSParams* params) : AutoPlayer(params) {
    _hslog = NULL;
    _applLogStream = NULL;
}

HSMOMDP::~HSMOMDP() {
    cout << "~HSMOMDP"<<endl;
    //AG120426: delete vars
    if (_hslog!=NULL) delete _hslog;
    if (_applLogStream!=NULL) {
        if (_applLogStream->is_open()) {
            _applLogStream->close();
        }
        delete _applLogStream;
    }
}

void HSMOMDP::initHSLog(const char* logFile) {
    if (logFile!=NULL) {
        _hslog = new HSLog(logFile);
        //_timeLogStream = _hslog->getOFStream();//new ofstream(timeLog);
        _hslog->printLineNoTime("Time,sec since 1970,What,col1,col2,col3,col4,col5,col6,col7,col8,col9,col10,col11,col12");//header
        _hslog->print("Load MOMDP,time(s)");
        _hslog->printLine("Init Belief,time(s),iteration,s-row,s-col,s-index,h-row,h-col,h-index,h-visible");
        _hslog->printLine("Update Belief,time(s),iteration,s-index,h-index,h-observation");
        _hslog->printLine("Generate Top MOMDP,State gen(s),calc X top trans prob (s),calc Y top trans prob (s),calc top obs probs (s),calc top Rew (s),calc top b0 (s),total gen top MOMDP (s),iteration,#X,#Y,#A,#O");
        _hslog->printLine("Solve,time(s),iteration,Elapsed time (s),#trials,#backups,low bound,upper bound,precision,#alpha,#beliefs");
        _hslog->printLine("Best Action,time(s),iteration,action,Max Reward,bottom action,bottom max reward,rank top in bottom,rank bottom in top");
    }
}


bool HSMOMDP::initBase(const char* pomdpFile, const char* logOutFile, const char *timeLog) {
    DEBUG_HS(cout << "*** HSMOMDP.init ***"<<endl;);    

    //AG130506: check if pomdp file exists!!
    if (!fileExists(pomdpFile)) {
        cout << "ERROR: the pomdp file '"<<pomdpFile<<"' does not exist!"<<endl;
        exit(-1);
    }

    initHSLog(timeLog);

    //int timerI = _timer.startTimer();

    //prepare params for appl param parser
    //(note: more efficient to pass required params directly)
    int argc_appl = 2;

    if (logOutFile != NULL) argc_appl += 2;

    char** argv_appl = new char*[argc_appl];
    argv_appl[0] = (char*)"pomdpsol";
    argv_appl[1] = charConstToCharArr(pomdpFile);

    if (logOutFile != NULL) {
        //open log file
        argv_appl[2] = (char*)"--output-file";
        argv_appl[3] = charConstToCharArr(logOutFile);
    }

    //get parser for the params
    _solverParams = &GlobalResource::getInstance()->solverParams;
    bool parseCorrect = SolverParams::parseCommandLineOption(argc_appl, argv_appl, *_solverParams);

    delete[] argv_appl;
    if(!parseCorrect)
    {
        cout << "Wrong parameters for APPL parameter parser." << endl;
        return false;
    }

    //ag note: note set
    bool enableFiling = (_solverParams->outputFile.length() > 0);
    DEBUG_HS(if (enableFiling) cout << "Output to: "<<_solverParams->outputFile<<endl; else cout << "No output to file"<<endl;);


    int timerI = _timer.startTimer();

    //load the MOMDP model
    DEBUG_HS_INIT(cout << "\nLoading the (bottom) MOMDP model ..." << endl << "  ";);    
    _momdp = ParserSelector::loadProblem(_solverParams->problemName, *_solverParams);
    DEBUG_HS_INIT(cout << "MOMDP Model loaded"<<endl;);

    _hslog->print("Load MOMDP");
    _hslog->printLine(_timer.stopTimer(timerI));

    int numActions = _momdp->getNumActions();
    if (numActions!=5 && numActions!=9) {
        cout << "Error: found "<<numActions <<" actions, expected 5 or 9"<<endl;
        exit(EXIT_FAILURE);
    }

    DEBUG_HS(showMOMDP(_momdp,false););


    //AG note: should be time (?) -> check
    srand(_solverParams->seed);//Seed for random number.  Xan


    if (enableFiling) {
        //open log file
        _applLogStream = new ofstream(_solverParams->outputFile.c_str());
        DEBUG_HS_INIT(cout << "APPL output log to: "<< _solverParams->outputFile <<endl;);
        *_applLogStream<<"start init"<<endl;
    } else {
        _applLogStream = NULL;
    }


    //TODO: set some params of solver (?)
    //AG121016: already passed through solverParams
    //_solverParams->timeoutSeconds = solveTimeOut;
    DEBUG_HS_INIT(if (_solverParams->timeoutSeconds >0) cout << "Solver time out time is "<<_solverParams->timeoutSeconds << " s"<<endl; else cout << "No solver time out limit."<<endl;);


    //setup bottom engine
    _solver.setup(_momdp,_solverParams);

    //set unused:
    _action = _x_state = _observ = -1;


    return true;
}


bool HSMOMDP::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible) {
    assert(_momdp!=NULL);
    assert(gmap!=NULL);

    _map = gmap;

    DEBUG_HS(cout << "*** HSMOMDP.initBelief ***"<<endl;);

    _hslog->print("Init Belief");

    int timerI = _timer.startTimer();

    //cout << "SEEKERPOS=" << seekerInitPos.toString()<<" HIDERPOS="<<hiderInitPos.toString()<<" VIS="<<hiderVisible<<endl;

    //assume that _gmap has all the info about positions
    //_gmap = gmap;

    //initStateIndexPos(); //ag130304: use gmap.getIndexFromCoord

    //get number of (variable) states (ie of either hider, seeker)

    int numStates = _map->numFreeCells();
    DEBUG_HS_INIT(cout << "Number of states: " << numStates << endl;);

    //generate initial belief
    //AG120425: init beliefY now directly in shared pointer
    SharedPointer<BeliefWithState> startVec (new BeliefWithState());
    SharedPointer<SparseVector> bvec = startVec->bvec;
    bvec->resize(numStates);

    // own initial (seeker) pos
    //Pos seekerInitPos = _gmap->getInitials();


    _map->printMap(seekerInitPos); //ag130301: other syntax printmap
    DEBUG_HS_INIT(cout << " seeker init pos: "<<seekerInitPos.toString()<<endl;);

    int seekerInitPosI = _map->getIndexFromCoord(seekerInitPos); //ag130304: was getPosIndex

    DEBUG_HS_INIT(cout <<"the initial position of the seeker is "<<seekerInitPos.toString()<<" index num: "<<seekerInitPosI<<endl;);

    //AG130320: data comes from outside
    //now check the hider pos
    //Pos hiderInitPos = _gmap->getInitialh();
    //int hiderInitPosI = _gmap->getIndexFromCoord(hiderInitPos);
    //bool hiderVis = _gmap->isVisible(seekerInitPos,hiderInitPos);
    int hiderInitPosI = -1;

    if (hiderVisible) {
        //int i = 0;
        //set probability only to hider init pos, since it is visible

        /*for(int r=0; r < _gmap->rowCount();r++) {
            for(int c=0; c < _gmap->colCount(); c++) {

                if (_gmap->getItem(r,c)!=GMap::GMAP_OBSTACLE) {
                    //only non-obstacle locations are states!!
                    if (r==hiderInitPos.x && c==hiderInitPos.y) {
                        bvec->push_back(i,1.0);
                    } else {
                        bvec->push_back(i,0);
                    }
                    i++;
                }

            }
        }*/
        //AG130301: disabled since bvec is a SparseVector with default value 0, only add 1 !!
        //int i = _gmap->getIndexFromCoord(hiderInitPos.row,hiderInitPos.col);
        int hiderInitPosI = _map->getIndexFromCoord(hiderInitPos);
        bvec->push_back(hiderInitPosI,1.0);

        DEBUG_HS_INIT(cout<<"hider is visible with pos "<<hiderInitPos.toString()<<" index num: "<<hiderInitPosI<<endl;);
    } else { //hider not visible
        DEBUG_HS_INIT(cout << "hider not visible, init with prob distribution over non-visible cells"<<endl;);
        //distribute prob over invisilbe cells
        //AG130301: rewritten get invisible cells code
        vector<Pos> invisPosVector = _map->getInvisiblePoints(seekerInitPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);
        /*char** visib = _gmap->getVisibletab();
        int numInvisibleCells = _gmap->invisibleCells();*/
        //belief for each invisible cell
        double invisProb = 1.0 / invisPosVector.size();

        //ag130301: only set probs for >0, since Sparse vector!!
        for (vector<Pos>::iterator it = invisPosVector.begin() ; it != invisPosVector.end(); ++it) {
            int i = _map->getIndexFromCoord(*it);
            bvec->push_back(i,invisProb);
        }


        //ag130301: rewritten
        /*
        int i = 0;
        for(int r=0; r < _gmap->rowCount();r++) {
            for(int c=0; c < _gmap->colCount(); c++) {

                if (!_gmap->isObstacle(r,c)) { // getItem(r,c)!=GMap::GMAP_OBSTACLE
                    //only non-obstacle locations are states!!

                    if (visib[r][c]==0) { //invisible
                        bvec->push_back(i,invisProb);
                    } else {
                        bvec->push_back(i,0);
                    }
                    i++;
                }

            }
        }*/

    }

    //initial seeker position (belief over X)
    startVec->sval = seekerInitPosI;

    DEBUG_HS1(cout << "seeker init pos: " << seekerInitPosI <<" ; beliefY.sval: " << startVec->sval <<endl;);

    //set current belief
    _currBelief = startVec;

    DEBUG_HS(cout << "init belief in appl:"<<endl;);

    //init enginge
    _solver.init(*startVec, /*hiderInitPosI,*/ _applLogStream);

    //index of 'unobserved' (ie hidden) observation index is the last in the array of observations
    _unobservedObs = _map->numFreeCells();

    //init action indicating the start (used later on to check if 1st to do a UpdateBelief)
    _action = -1;

    //iter 0
    _timeIter = 0;

    //show belief
    DEBUG_PB(HSSolver::printBelief(_currBelief,"initial belief",hiderInitPosI,_map););

    _hslog->print(_timer.stopTimer(timerI));
    _hslog->print(_timeIter);
    _hslog->print(seekerInitPos);
    _hslog->print(seekerInitPosI);
    _hslog->print(hiderInitPos);
    _hslog->print(hiderInitPosI);
    _hslog->printLine(hiderVisible);

    return true;
}

bool HSMOMDP::updateBelief(int observ, int x_state) {
    assert(_action>=0);

    DEBUG_HS(cout << "*** HSMOMDP.updateBelief ***"<<endl;);

    _hslog->print("Update Belief");
    int timerI = _timer.startTimer();

    //update belief
    _currBelief = _solver.updateBelief(observ, x_state, _action);

    //store states
    _x_state = x_state;
    _observ = observ;

    //show belief
    DEBUG_PB(HSSolver::printBelief(_currBelief,"update belief",observ,_map););

    _hslog->print(_timer.stopTimer(timerI));
    _hslog->print(_timeIter);
    _hslog->print(x_state);    
    _hslog->print(-1 /*y_state*/);

    _hslog->printLine(observ);

    return true;
}


bool HSMOMDP::updateBelief(Pos seekerPos, Pos hiderPos, bool visible) {
    int xI = _map->getIndexFromCoord(seekerPos);

    int obsI = _unobservedObs;
    if (visible) {
        obsI = _map->getIndexFromCoord(hiderPos);
    }

    return updateBelief(obsI,xI);
}


int HSMOMDP::getNextAction(Pos seekerPos, Pos hiderPos, bool visible, int actionDone) {
    int xI = _map->getIndexFromCoord(seekerPos);

    int obsI = _unobservedObs;
    if (visible) {
        obsI = _map->getIndexFromCoord(hiderPos);
    }

    return getNextAction(obsI,xI);
}



//todo: add 'static' ??
void HSMOMDP::showMOMDP(SharedPointer<MOMDP> momdp, bool showTransition) {
    cout << "<<---- MOMDP info ---->>"<<endl;

/*
    cout << "state vars: "<< momdp->getStateList().size()<<endl;
    cout << "obsList: "<< momdp->getObservationList().size()<<endl;
    cout << "actList: "<< momdp->getActionList().size()<<endl;
    cout << "rewardList: "<< momdp->getRewardList().size()<<endl;
    cout << "XTrans==null: "<<(momdp->XTrans ==NULL)<<endl;
    cout << "YTrans==null: "<<(momdp->YTrans==NULL)<<endl;
    cout << "XStates==null: "<<(momdp->XStates==NULL)<<endl;
    cout << "YStates==null: "<<(momdp->YStates==NULL)<<endl;
    cout << "obsProb==null: "<<(momdp->obsProb==NULL)<<endl;
    cout << "rewards==null: "<<(momdp->rewards==NULL)<<endl;
*/


    int numA = momdp->getNumActions();
    int numX = momdp->XStates->size();
    int numY = momdp->YStates->size();
    int numO = momdp->observations->size();

    cout << "#A: "<<numA<<"; #X: "<<numX<<"; #Y: "<<numY<<"; #O: "<<numO<<endl;

    cout << "discount : " << momdp->discount << endl;
    cout << "initialBeliefY : " << endl;
    momdp->initialBeliefY->write(cout) << endl;
    cout << "initialBeliefStval : " << endl;
    cout << "initialBeliefStval stval: " <<  momdp->initialBeliefStval->sval << endl;
    momdp->initialBeliefStval->bvec->write(cout) << endl;
    cout << "initialBeliefX : " << endl;
    momdp->initialBeliefX->write(cout) << endl;
    cout << "Num X States : " << momdp->XStates->size() << endl;
    cout << "Num Y States : " << momdp->YStates->size() << endl;
    cout << "Num Action : " << momdp->actions->size() << endl;
    cout << "Num Observations : " << momdp->observations->size() << endl;
    cout << "X Trans : " << momdp->XTrans->ToString() << endl;
    cout << "Y Trans : " << momdp->YTrans->ToString() << endl;
    cout << "Obs Prob : " << momdp->obsProb->ToString() << endl;
    cout << "Rewards : " << momdp->rewards->ToString() << endl;

    //cout << "# actions: "<< momdp->getNumActions() <<endl;
    cout << "MOMDP to string: " <<endl << momdp->ToString()<<endl;

    if (showTransition) {
        cout << "XTrans:"<<endl;
        for (int a=0;a<numA;a++) {
            for(int x=0;x<numX;x++) {
                cout << " - a:"<<a<<", x:"<<x;::

                SharedPointer<SparseMatrix> mat = momdp->XTrans->getMatrix(a,x);
                int s1 = mat->size1();
                int s2 = mat->size2();
                cout <<", s1:"<<s1<<", s2:"<<s2<<endl;

                for (int i=0;i<s1;i++) {
                    for (int j=0;j<s2;j++) {
                        double p = (*mat)(i,j);
                        if (p>EPSILON) {
                            cout <<"    ("<<i<<","<<j<<"): "<<flush;
                            cout <<p<<endl;
                        }
                    }
                }

            }
        }


        cout << "YTrans:"<<endl;
        for (int a=0;a<numA;a++) {
            for(int x=0;x<numX;x++) {
                for(int y=0;y<numY;y++) {
                    cout << " - a:"<<a<<",x:"<<x<<", y:"<<y;

                    SharedPointer<SparseMatrix> mat = momdp->YTrans->getMatrix(a,x,y);
                    int s1 = mat->size1();
                    int s2 = mat->size2();
                    cout <<", s1:"<<s1<<", s2:"<<s2<<endl;

                    for (int i=0;i<s1;i++) {
                        for (int j=0;j<s2;j++) {
                            double p = (*mat)(i,j);
                            if (p>EPSILON) {
                                cout <<"    ("<<i<<","<<j<<"): "<<flush;
                                cout <<p<<endl;
                            }
                        }
                    }

                }
            }
        }

        cout << "ObsProb:"<<endl;
        for (int a=0;a<numA;a++) {
            for(int x=0;x<numX;x++) {
                cout << " - a:"<<a<<", x:"<<x;

                SharedPointer<SparseMatrix> mat = momdp->obsProb->getMatrix(a,x);
                int s1 = mat->size1();
                int s2 = mat->size2();
                cout <<", s1:"<<s1<<", s2:"<<s2<<endl;

                for (int i=0;i<s1;i++) {
                    for (int j=0;j<s2;j++) {
                        double p = (*mat)(i,j);
                        if (p>EPSILON) {
                            cout <<"    ("<<i<<","<<j<<"): "<<flush;
                            cout <<p<<endl;
                        }
                    }
                }

            }
        }

        cout << "Reward:"<<endl;
        //for (int a=0;a<numA;a++) {
        for(int x=0;x<numX;x++) {
            cout << " - x:"<<x;

            SharedPointer<SparseMatrix> mat = momdp->rewards->getMatrix(x);
            int s1 = mat->size1();
            int s2 = mat->size2();
            cout <<", s1:"<<s1<<", s2:"<<s2<<endl;

            for (int i=0;i<s1;i++) {
                for (int j=0;j<s2;j++) {
                    double p = (*mat)(i,j);
                    if (p>EPSILON) {
                        cout <<"    ("<<i<<","<<j<<"): "<<flush;
                        cout <<p<<endl;
                    }
                }
            }

        }
    }
/*
    if(_solverParams->useLookahead)
    {
        cout << "  action selection :  one-step look ahead" << endl;
    }
    else
    {
        cout << "  action selection :  NOT one-step look ahead" << endl;
    }
*/
    cout << "<<---- end of MOMDP info ---->>"<<endl;
}


double HSMOMDP::getBelief(int y_state) {
    return (*_currBelief->bvec)(y_state);
}


double HSMOMDP::getBelief(int r, int c) {
    return getBelief(_map->getIndexFromCoord(r,c));
}


/*void HSMOMDP::initStateIndexPos() {
    _stateToPosList.clear();
    //create list
    for(int r=0; r < _gmap->rowCount();r++) {
        for(int c=0; c < _gmap->colCount(); c++) {

            if (_gmap->getItem(r,c)!=GMap::GMAP_OBSTACLE) {
                Pos p(r,c);
                _stateToPosList.push_back(p);
            }
        }
    }
}

Pos HSMOMDP::stateToPos(int state) {
    return _stateToPosList[state];
}

*/

bool HSMOMDP::tracksBelief() const {
    return true;
}

bool HSMOMDP::isSeeker() const {
    return true;
}
