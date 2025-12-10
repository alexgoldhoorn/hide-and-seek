#include "hsmomdp_layered.h"

#include <cstdlib>
#include <iomanip>
#include <cfloat>

#include "PreCMatrix.h"

#include "Utils/generic.h"

#include "exceptions.h"


LayeredHSMOMDP::LayeredHSMOMDP() : HSMOMDP() {
    _topSolver = NULL;
    _listBottomTopStates = _listBottomTopStatesX = NULL;

    DEBUG_HS_INIT(cout << "LayeredHSMOMDP generated"<<endl;);
    DEBUG_HS_INIT(cout << "Removing previous policy files: ";);
    //AG WARNING: 1) will also delete if other instance is running!! -> delete its policies!
    //            2) should be based on the name of pomdp + date-time
    //            3) not sure if it works
    std::remove("momdpOnlineTopPolicy*.policy");
    DEBUG_HS_INIT(cout << "ok"<<endl;);
}

LayeredHSMOMDP::~LayeredHSMOMDP() {
    DEBUG_DELETE(cout << "~LayeredHSMOMDP ... "<<flush;);
    //AG120426: delete
    if (_topSolver!=NULL) delete _topSolver;
    //ag120426: delete previous segments
    if (_listBottomTopStates!=NULL) delete _listBottomTopStates;
    if (_listBottomTopStatesX!=NULL) delete _listBottomTopStatesX;
    DEBUG_DELETE(cout <<"ok"<<endl;);
}


bool LayeredHSMOMDP::init(const char* pomdpFile, const char* logOutFile, const char *timeLog, bool useFIBUBInit,
                          char segmentValue, const char* policyFile, char topRewardAggregation, bool segmentSeekerStates,
                          bool segmentObservations, bool setFinalStateOnTop, bool setFinalTopRewards) {

    DEBUG_HS(cout << "*** LayeredHSMOMDP.init ***"<<endl;);

    //AG130506: check if policy file exists!!
    if (policyFile != NULL && !fileExists(policyFile)) {
        cout << "ERROR: the policy file '"<<policyFile<<"' does not exist!"<<endl;
        exit(-1);
    }

    bool ok = HSMOMDP::initBase(pomdpFile, logOutFile, timeLog);

    assert(segmentValue>=SEGMENT_VALUE_BELIEF_ONLY && segmentValue <= SEGMENT_VALUE_BELIEF_REWARD);
    _segmentValueType = segmentValue;

    //ag120420
    setUseFIBUBInit(useFIBUBInit);

    //AG121010: compare layered policy with offline's policy
    if (ok && policyFile != NULL) {
        ok = _solver.readPolicy(policyFile);
        _compareWithOffline = true;
    } else {
        _compareWithOffline = false;
    }

    //AG121108: top reward aggregation
    _topRewardAggregation = topRewardAggregation;

    //AG121213: also segmenter seeker postion top layer
    _segmentSeekerStates = segmentSeekerStates;
    //AG121219: segment observations
    _segmentObservations = segmentObservations;

    _setFinalStateOnTop = setFinalStateOnTop;
    _setFinalTopRewards = setFinalTopRewards;

    return ok;
}


//! get FIB Upper bound init (if no use (Q)MDP; see APPL)
void LayeredHSMOMDP::setUseFIBUBInit(bool useFIBUBInit) {
    _solverParams->FIB_UB_Init = useFIBUBInit;
    //_solverParams->MDPSolution = !useFIBUBInit;
    //_solverParams->FIBSolution = useFIBUBInit;
}

//ag120420
//! get FIB Upper bound init (if no use (Q)MDP; see APPL)
bool LayeredHSMOMDP::getUseFIBUBInit() {
    if (_solverParams == NULL) {
        return false;
    } else {
        return _solverParams->FIBSolution;
    }
}

//ag120423
//! set target precision and initial precision factor
void LayeredHSMOMDP::setTargetPrecision(double targetPrecision,double targetInitPrecFactor) {
    assert(targetPrecision>0);
    assert(targetInitPrecFactor>=0);
    _solverParams->targetPrecision = targetPrecision;
    _solverParams->cb_initialization_precision_factor = targetInitPrecFactor;
}

bool LayeredHSMOMDP::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible) {
    DEBUG_HS(cout << "*** LayeredHSMOMDP.initBelief ***"<<endl;);
    assert(_segmenter!=NULL);//TODO: exception

    bool ok = HSMOMDP::initBelief(gmap,seekerInitPos,hiderInitPos,hiderVisible);

    //AG120829: init position also in segmenter
    //Pos seekerInitPos = gmap->getInitials();

    _segmenter->setGMap(gmap);
    _segmenter->setPosition(seekerInitPos.row, seekerInitPos.col);

    //AG130101: segmenter x
    if (_segmentSeekerStates) {
        assert(_segmenterX!=NULL);
        _segmenterX->setGMap(gmap);
        _segmenterX->setPosition(seekerInitPos.row, seekerInitPos.col);
    }

    //get state of base
    _baseState = _gmap->getIndexFromCoord(_gmap->getBase());

    return ok;
}


bool LayeredHSMOMDP::updateBelief(int observ, int x_state) {
    DEBUG_HS(cout << "*** LayeredHSMOMDP.updateBelief ***"<<endl;);
    assert(_segmenter!=NULL);//TODO: exception

    bool ok = HSMOMDP::updateBelief(observ, x_state);

    //AG120829: init position also in segmenter
    Pos seekerPos = _gmap->getCoordFromIndex(x_state); //ag130304: was stateToPos(x_state);
    _segmenter->setPosition(seekerPos.row, seekerPos.col);

    //AG130101: segmenter x
    if (_segmentSeekerStates) {
        assert(_segmenterX!=NULL);
        _segmenterX->setPosition(seekerPos.row, seekerPos.col);
    }

    return ok;
}


bool LayeredHSMOMDP::generateTopMOMDP() {
    assert(_momdp.get()!=NULL); //TODO: check what to use for NULL check for boost shared pointer

    DEBUG_HS(cout << "*** LayeredHSMOMDP.generateTopMOMDP ***"<<endl;);
    _hslog->print("Generate Top MOMDP");

    DEBUG_HS(cout << "segment seeker: "<<_segmentSeekerStates<<" "<<(_segmenterX==NULL?"NULL":"not set")<<"; obs segmenter: "<<_segmentObservations<<endl;);

    cout << "#X:"<<_momdp->XStates->size()<<endl;
    cout << "#Y:"<<_momdp->YStates->size()<<endl;    
    int timerI = _timer.startTimer();

    //first generate new states
    DEBUG_HS1(cout << "get bottom states: "<<flush;);
    vector<State> stateList = _momdp->getStateList();
    DEBUG_HS1(cout << "ok"<<endl<<"check X,Y states: "<<flush;);
    //generate
    vector<State> stateListTop;
    State *stateX,*stateY;

    if (stateList.size()!=2) {
        cout << "Error @ LayeredHSMOMDP::generateTopMOMDP: StateVar list size: " << stateList.size() <<", expected: 2"<<endl;
        exit(EXIT_FAILURE);
    }

    //get original states
    State state = stateList[0];
    if (state.getObserved()) {
        stateX = &state;
        stateY = &(stateList[1]);
    } else {
        stateY = &state;
        stateX = &(stateList[1]);
    }

    //expected: 1 visible, 1 not visible state
    if (stateX==NULL || stateY==NULL) {
        cout << "Error @ LayeredHSMOMDP::generateTopMOMDP: state X or Y is null"<<endl;
        exit(EXIT_FAILURE);
    }
    if (stateX->getObserved()==stateY->getObserved()) {
        cout << "Error @ LayeredHSMOMDP::generateTopMOMDP: states are both observed="<< stateX->getObserved()<<endl;
        exit(EXIT_FAILURE);
    }

    DEBUG_HS1(cout << "ok"<<endl<<"get #A,X,Y,O: "<<flush;);

    //num
    unsigned int numActions = _momdp->getNumActions();
    unsigned int numStatesX = stateX->getValueEnum().size();
    unsigned int numStatesY = stateY->getValueEnum().size();
    unsigned int numObs = _momdp->observations->size(); //IS THIS OK??

    
    DEBUG_HS1(cout << "ok"<<endl<<"generate state X,Y: "<<flush;);


    //generate top Y state
    State stateYTop;
    stateYTop.setObserved(false);
    stateYTop.setVNamePrev("y0");
    stateYTop.setVNameCurr("y1");

    //AG121219: top X state
    State stateXTop;
    stateXTop.setObserved(true);
    stateXTop.setVNamePrev("x0");
    stateXTop.setVNameCurr("x1");



	DEBUG_HS1(cout << "gen top "<<flush;);

    //segment to generate regions
    vector<string> ssyvec = generateTopStates(numActions, numStatesY, _segmenter, _listBottomTopStates, _listTopBottomStates);

#ifdef DEBUG_HS1_ON
    cout <<"Y top:"<<flush;
    for(vector<string>::iterator vecit=ssyvec.begin();vecit!=ssyvec.end();vecit++) {
        cout << (*vecit)<<","<<flush;
    }
    cout << endl;
#endif

    //set state values of top state y
    stateYTop.setValueEnum(ssyvec);
    //num states
    unsigned int numStatesYTop = ssyvec.size();

    //AG121219: set top x states
    unsigned int numStatesXTop = 0;
    if (_segmentSeekerStates) {
        vector<string> ssxvec = generateTopStates(numActions, numStatesX, _segmenterX, _listBottomTopStatesX, _listTopBottomStatesX);

        numStatesXTop = ssxvec.size();
        stateXTop.setValueEnum(ssxvec);

#ifdef DEBUG_HS1_ON
        cout <<"X top:"<<flush;
        for(vector<string>::iterator vecit=ssxvec.begin();vecit!=ssxvec.end();vecit++) {
            cout << (*vecit)<<","<<flush;
        }
        cout << endl;
#endif

    } else {
        numStatesXTop = numStatesX;
        stateXTop.setValueEnum(stateX->getValueEnum());
    }

    //add states to list
    stateListTop.push_back(stateXTop); //AG121219, use top x state // *stateX);
    stateListTop.push_back(stateYTop);


    //AG121221: also segment observations


    unsigned int numObsTop = 0;
    vector<ObsAct> obsListTop;
    if (_segmentObservations) {
        numObsTop = numStatesYTop+1;
        ObsAct obs;
        obs.setVName("o");
        vector<string> obsvec(ssyvec);
        obsvec.push_back("u");
        obs.setValueEnum(obsvec);
        //130105: add obs (forgotten)
        obsListTop.push_back(obs);
    } else {
        numObsTop = numObs;
        obsListTop = _momdp->getObservationList();
    }

    //TODO: check obs size


    DEBUG_HS1(cout << "ok"<<endl<<""<<flush;);

    _hslog->print(_timer.stopTimer(timerI));


    DEBUG_HS(cout << "MOMDP:        #A: " << numActions << ", #Y: "<<numStatesY     <<", #X: "<<numStatesX      <<", #O: "<<numObs<<endl;);
    DEBUG_HS(cout << "TOP MOMDP:    #A: " << numActions << ", #Y: "<<numStatesYTop  <<", #X: "<<numStatesXTop   <<", #O: "<<numObsTop<<endl;);

    DEBUG_HS(if (numObs<=numStatesY) cout << "WARNING: NOT right number of observation"<<endl;);
    DEBUG_HS(if (numObsTop<=numStatesYTop) cout << "WARNING: NOT right number of TOP observation"<<endl;);

    DEBUG_HS(cout << "init matrices: "<<flush;);

    //matrices for top probabilities + reward
    vector<vector<SharedPointer<SparseMatrix> > > xmatrixTop;
    vector<vector<SharedPointer<SparseMatrix> > > xmatrixTopTr;
    vector<vector<SharedPointer<SparseMatrix> > > ymatrixTop;
    vector<vector<SharedPointer<SparseMatrix> > > ymatrixTopTr;
    vector<vector<SharedPointer<SparseMatrix> > > omatrixTop;
    vector<vector<SharedPointer<SparseMatrix> > > omatrixTopTr;
    vector<SharedPointer<SparseMatrix> > rmatrixTop;

    xmatrixTop.resize(numActions);
    xmatrixTopTr.resize(numActions);
    ymatrixTop.resize(numActions);
    ymatrixTopTr.resize(numActions);
    omatrixTop.resize(numActions);
    omatrixTopTr.resize(numActions);
    rmatrixTop.resize(numStatesX);
    //NOTE: filling of the SparseMatrix in the matrices is done: forall col: forall row
    //      and NOT forall row: forall col -> because of the way the SparseMatrix works


    //NOTE we need to recreate the XTransition function, since it returns a matrix [y,x']
    // but since we changed the y states, we ALSO have to change these probs!!!
    DEBUG_HS(cout << "ok"<<endl<<"Calc X trans top .."<<endl;);

    //_hslog->print(" * calculate X top trans prob");
    _timer.restartTimer(timerI);

    //get X trans of bottom
    StateTransitionX* xTrans = _momdp->XTrans;

    //TODO: X segmentation!!! error when using
      //      ./hsmomdp -sl -m pomdpx/simplerew_fs/map3_10x10.pomdpx -p pomdpx/simplerew_fs/map3_10x10.policy -or -u testS -gr 10 45 0 0 -grx 10 45 0 2 > test1.log

    //PROBABLY passing index of x/y due to out of bounds (using top index/..)


    int topFSCount = 0;//tmp


    //create top transition X
    FOR (a, numActions) {
        //cout << " a="<<a<<flush;//tmp
        xmatrixTop[a].resize(numStatesXTop);
        xmatrixTopTr[a].resize(numStatesXTop);

        FOR (x, numStatesXTop) {
            //cout<<" x="<<x<<flush;//tmp
            //bottom transitions (a,x)->[y,x']
            SharedPointer<SparseMatrix> txSm;
            if (_segmentSeekerStates == false) {
                txSm = xTrans->getMatrix(a,x);
            }

            //top transitions
            //AG121029: use PreSparseMatrix
            //AG121219: use numStateXTop
            PreSparseMatrix txTopSm(numStatesYTop,numStatesXTop);
            PreSparseMatrix txTopSmTr(numStatesXTop,numStatesYTop);

            //AG121219: top x states
            vector<int> xBs;
            if (_segmentSeekerStates) {
                xBs = _listTopBottomStatesX[x];
            }

            //AG130429: exchanged FOR(xn..) and FOR(y..)

            FOR(y, numStatesYTop) { //r->y
                vector<int> yBs = _listTopBottomStates[y];
            /*FOR(xn, numStatesXTop) { //c->xn
                //cout<<" x'="<<xn<<flush;//tmp

                //AG121219: top x' states
                vector<int> xnBs;
                if (_segmentSeekerStates) {
                    xnBs = _listTopBottomStatesX[xn];
                }*/


                //ag130503: set final state
                bool isFinalS = false;
                if (_setFinalStateOnTop) {
                    //note: state where seeker wins.. in theory base is clear since is only 1 state
                    bool sWins=false;
                    isFinalS = isFinalState(x,y,xBs,yBs,sWins);
                }

                //ag130429: is final state so stay in same state (and we want to set top final states)
                if (isFinalS) {
                    txTopSm.addEntries(y,x,1.0);
                    topFSCount++; //tmp

                } else {
                    //not final state, normal calculation

                    FOR(xn, numStatesXTop) { //c->xn
                        vector<int> xnBs;
                        if (_segmentSeekerStates) {
                            xnBs = _listTopBottomStatesX[xn];
                        }

                    /*FOR(y, numStatesYTop) { //r->y
                        //cout <<" y="<<y<<flush;//tmp
                        //current y states (list of bottom states) in top state r
                        vector<int> yBs = _listTopBottomStates[y];*/

                        double p =0;

                        if (_segmentSeekerStates) { //AG121219: seeker states also segmented
                            //sum all probabilities: p(x'|y,x,a) with: y in Gamma(y_T), x' in G(x'_T), x in G(x_T)
                            FOR(xc, xBs.size()) {
                                txSm = xTrans->getMatrix(a,xBs[xc]);
                                FOR(xnc, xnBs.size()) {
                                    FOR(yc, yBs.size()) {
                                        p += (*txSm)(yBs[yc], xnBs[xnc]);
                                    }
                                }
                            }

                            //get average
                            p /= yBs.size()*xBs.size();

                        } else {

                            //sum all probabilities: p(x'|y,x,a) with: y in Gamma(y_T),
                            FOR(yc, yBs.size()) {
                                p += (*txSm)(yBs[yc], xn);
                            }

                            //get average
                            p /= yBs.size();

                        }


                        //put in top matrix [AG121102] if not 0
                        if (p > HSGlobalData::ZERO_EPS) {
                            txTopSm.addEntries(y,xn,p); //txTopSm->push_back(r,c,p);
                        }

                        //cout <<"."<<flush;//tmp
                    }
                }
            }



            //AG121102: convert matrix to sparsematrix, to access it
            SharedPointer<SparseMatrix> txTopMat = txTopSm.convertSparseMatrix();

            //cout<<"-"<<flush;//tmp

            //AG121102: new for-loop because items should be added column, then row, therefore
            //          the tr(ansposed) matrix has to be filled in the opposite way
            FOR(y, numStatesYTop) {
                FOR(xn, numStatesXTop) {
                    double p = (*txTopMat)(y,xn);
                    //put in top matrix [AG121102] if not 0
                    if (p > HSGlobalData::ZERO_EPS) {
                        txTopSmTr.addEntries(xn,y,p);
                    }
                }
            }


            //cout<<"-"<<flush;//tmp


            //put in bigger matrix
            xmatrixTop[a][x] = txTopMat;
            xmatrixTopTr[a][x] = txTopSmTr.convertSparseMatrix();

            //cout<<"#"<<flush;//tmp
        }

    }


    DEBUG_HS(cout << "TOP FINAL STATES: "<<topFSCount/numActions<<endl;);


    //generate T_{X,new}
    StateTransitionX *xTransTop = new StateTransitionX(xmatrixTop,xmatrixTopTr);


    _hslog->print(_timer.stopTimer(timerI));

#if defined(DEBUG_HS_ON) && defined(DEBUG_SHOW_TRANS_ON)
    cout << "top XTrans-print:"<<endl;
    FOR(a,numActions) {
        FOR(x,numStatesX) {
            cout << " - a:"<<a<<",x:"<<x;

            SharedPointer<SparseMatrix> mat = xTransTop->getMatrix(a,x);
            int s1 = mat->size1();
            int s2 = mat->size2();
            cout <<", s1 (y):"<<s1<<", s2 (x'):"<<s2<<endl;

            for (int i=0;i<s1;i++) {
                for (int j=0;j<s2;j++) {
                    //get prob
                    double p = (*mat)(i,j);
                    if (p>EPSILON) {
                        cout <<"    ("<<i<<","<<j<<"): "<<flush;
                        cout <<p<<endl;
                    }
                }
            }
        }
    }
#endif


    DEBUG_HS(cout << "Calc Y trans top .."<<endl;);

    topFSCount = 0; //tmp

    //_hslog->print(" * calculate Y top trans probs");
    _timer.restartTimer(timerI);

    //get Y trans of bottom
    StateTransitionY* yTrans = _momdp->YTrans;

    //create top transition Y
    FOR (a, numActions) {
        //cout<<" a"<<a<<flush;//tmp
        ymatrixTop[a].resize(numStatesXTop);
        ymatrixTopTr[a].resize(numStatesXTop);

        FOR (x, numStatesXTop) {
            //cout<<" x"<<x<<flush;//tmp

            //bottom transitions (a,x)->[y,y']
            SharedPointer<SparseMatrix> tySm;
            //AG121219: segment seeker state
            if (_segmentSeekerStates == false)
                tySm = yTrans->getMatrix(a,x,0); //note: 0: xp-> not used

            //top transitions
            //AG121029: use PreSparseMatrix
            PreSparseMatrix tyTopSm(numStatesYTop,numStatesYTop);
            PreSparseMatrix tyTopSmTr(numStatesYTop,numStatesYTop);

            //cout<<"."<<flush;//tmp

            //AG121219: x segmentation
            vector<int> xBs;
            if (_segmentSeekerStates) {
                xBs = _listTopBottomStatesX[x];
            }

            //cout<<"."<<flush;//tmp

            //AG130429: exchanged FOR(y..) and FOR(yn..)

            FOR(y, numStatesYTop) { //r->-y
                //cout <<" y"<<y<<flush;//tmp

                //current y states (list of bottom states) in top state r
                vector<int> yBs = _listTopBottomStates[y];
            /*FOR(yn, numStatesYTop) { //c->yn
                //cout <<" y'"<<yn<<flush;//tmp

                //new y states (list of bottom states) in top state c
                vector<int> yBSn = _listTopBottomStates[yn];*/


                //ag130503: set final state
                bool isFinalS = false;
                if (_setFinalStateOnTop) {
                    //note: state where seeker wins.. in theory base is clear since is only 1 state
                    bool sWins=false;
                    isFinalS = isFinalState(x,y,xBs,yBs,sWins);
                }


                //ag130429: is final state so stay in same state (and final state to be set)
                if (isFinalS) {
                    tyTopSm.addEntries(y,y,1.0);
                    topFSCount++; //tmp

                } else {


                    FOR(yn, numStatesYTop) { //c->yn
                        //cout <<" y'"<<yn<<flush;//tmp

                        //new y states (list of bottom states) in top state c
                        vector<int> yBSn = _listTopBottomStates[yn];

                    /*FOR(y, numStatesYTop) { //r->-y
                        //cout <<" y"<<y<<flush;//tmp

                        //current y states (list of bottom states) in top state r
                        vector<int> yBs = _listTopBottomStates[y];*/


                        double p =0;

                        if (_segmentSeekerStates) {
                            //sum of all probabilities: p(y'|y,x,a) with: y in Gamma(y_T),y' in Gamma(y_T'), x in G(x_T)
                            FOR(xc, xBs.size()) {
                                tySm = yTrans->getMatrix(a,xBs[xc],0);
                                FOR(yc, yBs.size()) {
                                    FOR(ync, yBSn.size()) {
                                        p += (*tySm)(yBs[yc], yBSn[ync]);
                                    }
                                }
                            }

                            p /= yBs.size()*xBs.size();

                            //put in top matrix [AG121102] if not 0
                            if (p > HSGlobalData::ZERO_EPS) {
                                tyTopSm.addEntries(y,yn,p);
                            }

                        } else {
                            //sum all probabilities: p(y'|y,x,a) with: y in Gamma(y_T),y' in Gamma(y_T')
                            FOR(yc, yBs.size()) {
                                FOR(ync, yBSn.size()) {
                                    p += (*tySm)(yBs[yc], yBSn[ync]);
                                }
                            }

                            p /= yBs.size();

                            //put in top matrix [AG121102] if not 0
                            if (p > HSGlobalData::ZERO_EPS) {
                                tyTopSm.addEntries(y,yn,p);
                            }
                        }

                        //cout <<"."<<flush;//tmp
                    }
                }

            }



            //cout <<"-"<<flush;//tmp

            //AG121102: convert matrix to sparsematrix, to access it
            SharedPointer<SparseMatrix> tyTopMat = tyTopSm.convertSparseMatrix();

            //cout <<"-"<<flush;//tmp

            //AG121102: new for-loop because items should be added column, then row, therefore
            //          the tr(ansposed) matrix has to be filled in the opposite way
            FOR(yn, numStatesYTop) {
                FOR(y, numStatesYTop) {
                    double p = (*tyTopMat)(y,yn);
                    //put in top matrix [AG121102] if not 0
                    if (p > HSGlobalData::ZERO_EPS) {
                        tyTopSmTr.addEntries(yn,y,p);
                    }
                }
            }

            //cout <<"-"<<flush;//tmp

            ymatrixTop[a][x] = tyTopMat;
            ymatrixTopTr[a][x] = tyTopSmTr.convertSparseMatrix();

            //cout <<"!"<<flush;//tmp
        }
    }


    DEBUG_HS(cout << "TOP FINAL STATES: "<<topFSCount/numActions<<endl;);//tmp

    //cout<<"XX"<<endl;//tmp

    //generate T_{Y,new}
    StateTransitionXY* yTransTop = new StateTransitionXY(ymatrixTop,ymatrixTopTr);

    _hslog->print(_timer.stopTimer(timerI));

#if defined(DEBUG_HS_ON) && defined(DEBUG_SHOW_TRANS_ON)
    cout << "top YTrans-print:"<<endl;
    FOR(a,numActions) {
        FOR(x,numStatesX) {
            //FOR(y,numStatesYTop) {
                cout << " - a:"<<a<<",x:"<<x; //<<", y:"<<y;

                SharedPointer<SparseMatrix> mat = yTransTop->getMatrix(a,x,0); //xp not used    //y);
                int s1 = mat->size1();
                int s2 = mat->size2();
                cout <<", s1 (y):"<<s1<<", s2 (y'):"<<s2<<endl;

                for (int i=0;i<s1;i++) {
                    for (int j=0;j<s2;j++) {
                        //get prob
                        double p = (*mat)(i,j);
                        if (p>EPSILON) {
                            cout <<"    ("<<i<<","<<j<<"): "<<flush;
                            cout <<p<<endl;
                        }
                    }
                }

            //}y3.
        }
    }
#endif


    DEBUG_HS(cout <<"Calc O prob top .."<<endl;);


    ///TODO: set O_top
    /// calc obs of top

    //_hslog->print(" * calculate top observations probs");
    _timer.restartTimer(timerI);

    //get probab of o
    ObservationProbabilities* obsProb = _momdp->obsProb;

    //create top obs probs
    FOR (a, numActions) {
        omatrixTop[a].resize(numStatesX);
        omatrixTopTr[a].resize(numStatesX);

        FOR (xn, numStatesXTop) {
            //bottom observation prob (a,x)->[y',o]
            SharedPointer<SparseMatrix> oSm;
            if (_segmentSeekerStates==false)
                oSm = obsProb->getMatrix(a,xn); //note: 0: xp-> not used

            //top obs prob
            //AG121029: use PreSparseMatrix
            PreSparseMatrix oTopSm(numStatesYTop,numObsTop);
            PreSparseMatrix oTopSmTr(numObsTop,numStatesYTop);

            //AG121221: x segmentation
            vector<int> xBSn;
            if (_segmentSeekerStates) {
                xBSn = _listTopBottomStatesX[xn];
            }

            FOR(o, numObsTop) { //c->o

                //AG121221: o segmentation
                vector<int> oBs;
                if (_segmentObservations) {
                    if (o<numObsTop-1) {
                        oBs = _listTopBottomStates[o];
                    } else {
                        oBs.push_back(numObs-1); //the 'unknown' observation is the last item
                    }
                }

                FOR(yn, numStatesYTop) { //r->yn
                    //bottom states in top state r
                    vector<int> yBSn = _listTopBottomStates[yn];
                    double p =0;

                    if (_segmentSeekerStates==true && _segmentObservations==true) {
                        //sum all probabilities: p(o'|x',y',a) with: y' in Gamma(y_T'), x in G(x_T), o in G(o_T)
                        FOR(xnc, xBSn.size()) {
                            oSm = obsProb->getMatrix(a, xBSn[xnc]);
                            FOR(oc, oBs.size()) {
                                FOR(ync, yBSn.size()) {
                                    p += (*oSm)(yBSn[ync], oBs[oc]);
                                }
                            }
                        }

                        p /= yBSn.size()*xBSn.size();

                    } else if (_segmentSeekerStates==false && _segmentObservations==false) {
                        //sum all probabilities: p(o'|x',y',a) with: y' in Gamma(y_T')
                        FOR(ync, yBSn.size()) {
                            p += (*oSm)(yBSn[ync], o);
                        }

                        p /= yBSn.size();
                    } else {
                        cout << "ERROR: segment seeker states and segment observations should either be both true or false, no other option implemented yet!";
                        exit(-1);
                    }

                    //put in top matrix [AG121102] if not 0
                    if (p > HSGlobalData::ZERO_EPS) {
                        oTopSm.addEntries(yn,o,p);
                    }

                }
            }

            //AG121102: convert matrix to sparsematrix, to access it
            SharedPointer<SparseMatrix> oTopMat = oTopSm.convertSparseMatrix();

            //AG121102: new for-loop because items should be added column, then row, therefore
            //          the tr(ansposed) matrix has to be filled in the opposite way
            FOR(yn, numStatesYTop) {
                FOR(o, numObsTop) {
                    double p = (*oTopMat)(yn,o);
                    //put in top matrix [AG121102] if not 0
                    if (p > HSGlobalData::ZERO_EPS) {
                        oTopSmTr.addEntries(o,yn,p);
                    }
                }
            }

            omatrixTop[a][xn] = oTopMat;
            omatrixTopTr[a][xn] = oTopSmTr.convertSparseMatrix();
        }
    }

    //obs prob top
    ObservationProbabilities *obsProbTop = new ObservationProbabilities(omatrixTop, omatrixTopTr);

    _hslog->print(_timer.stopTimer(timerI));

#if defined(DEBUG_HS_ON) && defined(DEBUG_SHOW_TRANS_ON)
    cout << "Print ObsProb:"<<endl;
    FOR(a,numActions) {
        FOR(x,numStatesX) {
            cout << " - a:"<<a<<", x:"<<x;

            SharedPointer<SparseMatrix> mat = obsProbTop->getMatrix(a,x);
            int s1 = mat->size1();
            int s2 = mat->size2();
            cout <<", s1 (y'):"<<s1<<", s2 (o):"<<s2<<endl;

            for (int i=0;i<s1;i++) {
                for (int j=0;j<s2;j++) {
                    //get prob
                    double p = (*mat)(i,j);
                    if (p>EPSILON) {
                        cout <<"    ("<<i<<","<<j<<"): "<<flush;
                        cout <<p<<endl;
                    }
                }
            }

        }
    }
#endif


    DEBUG_HS(cout <<"Calc rew top .."<<endl;);

    //_hslog->print(" * calculate top rewards");
    _timer.restartTimer(timerI);

    //rewards
    Rewards* rewards = _momdp->rewards;

    //int baseI =_gmap->getIndexFromCoord(_gmap->getBase());

    //create top rewards
    FOR(x, numStatesXTop) {

        vector<int> xBs;
        if (_segmentSeekerStates) {
            xBs = _listTopBottomStatesX[x];
        }

        //bottom reward (x)->[y,a]
        SharedPointer<SparseMatrix> rSm = rewards->getMatrix(x);        
        //AG121029: use PreSparseMatrix
        PreSparseMatrix rTopSm(numStatesYTop,numActions);

        FOR(a, numActions) { //c->a
            FOR(y, numStatesYTop) { //r->y
                //bottom states in top state y
                vector<int> yBs = _listTopBottomStates[y];

                //AG130503: set final state top rewards
                if (_setFinalTopRewards) {
                    //ag130429: is final state so stay in same state
                    /*if (x==baseI) {
                        rTopSm.addEntries(y,a,-1.0);
                    } else {
                    */
                        bool seekerWins = false;
                        bool isFinalS = isFinalState(x,y,xBs,yBs,seekerWins);
                        if (isFinalS) {
                            rTopSm.addEntries( y, a, (seekerWins ? 1.0 : -1.0 ) );
                            cout << " ("<<x<<","<<y<<","<<(seekerWins?"S":"H")<<") "<<flush;
                            //cout <<"!"<<flush;
                        } //else cout <<"."<<flush;
                    //}
                } else { // avg rewards of bottom
                    //reward
                    double r = 0;
                    //init reward
                    switch (_topRewardAggregation) {
                        case TOP_REWARD_SUM:
                        case TOP_REWARD_AVG:
                            r = 0;
                            break;
                        case TOP_REWARD_MIN:
                            r = DBL_MAX;
                            break;
                        case TOP_REWARD_MAX:
                            r = -DBL_MAX;
                            break;
                        default:
                            cout << "ERROR: unknown top reward type: "<<_topRewardAggregation<<endl;
                            exit(-1);
                            break;
                    }

                    //sum all rewards: R(x,y,a) with: y in Gamma(y_T)
                    //if (y==4) cout << "BASE (y=4,x="<<x<<",a="<<a<<") # bottom y states: "<< yBs.size() << "; aggreg="<<(int)_topRewardAggregation<<endl;//TMP
                    FOR(yc, yBs.size()) {

                        double rx = (*rSm)(yBs[yc], a);
                        //if (y==4) cout <<"  y_b="<<yBs[yc]<<"r="<<r<<"; rx="<<rx<<endl;//TMP

                        switch (_topRewardAggregation) {
                            case TOP_REWARD_SUM:
                            case TOP_REWARD_AVG:
                                r += rx;
                                break;
                            case TOP_REWARD_MIN:
                                if (rx<r) r = rx;
                                break;
                            case TOP_REWARD_MAX:
                                if (rx>r) r = rx;
                                break;
                        }
                    }

                    if (_topRewardAggregation==TOP_REWARD_AVG) {
                        r /= yBs.size();
                    }

                    //put in top matrix [AG121102] if not 0 (note: rewards can be <0)
                    if (r < -HSGlobalData::ZERO_EPS || r > HSGlobalData::ZERO_EPS) {
                        rTopSm.addEntries(y,a,r);
                    }

                } // set state average or final
            }
        }

        rmatrixTop[x] = rTopSm.convertSparseMatrix();
    }

    //top rewards
    Rewards* rewardsTop = new Rewards(rmatrixTop);

    _hslog->print(_timer.stopTimer(timerI));

#if defined(DEBUG_HS_ON) && defined(DEBUG_SHOW_TRANS_ON)
    cout << "Top reward aggragation: "<<(int)_topRewardAggregation<<endl;
    cout << "Print Reward:"<<endl;
    //for (int a=0;a<numA;a++) {
    FOR(x,numStatesX) {
        cout << " - x:"<<x;

        SharedPointer<SparseMatrix> mat = rewardsTop->getMatrix(x);
        int s1 = mat->size1();
        int s2 = mat->size2();
        cout <<", s1 (y):"<<s1<<", s2 (a):"<<s2<<endl;

        for (int i=0;i<s1;i++) {
            for (int j=0;j<s2;j++) {
                //get prob
                double p = (*mat)(i,j);
                if (p>EPSILON || p<-EPSILON) { //AG121108: also show negative rewards
                    cout <<"    ("<<i<<","<<j<<"): "<<flush;
                    cout <<p<<endl;
                }
            }
        }

    }
#endif

    //set the initial belief of the top, based on the bottom belief
    DEBUG_HS(cout <<"Calc b0 top .."<<endl;);
   //NOTE: here I assume initialBeliefStval->bvec == initialBeliefY

    //_hslog->print(" * calculate top b0");
    _timer.restartTimer(timerI);

    SharedPointer<BeliefWithState> initialBeliefStvalTop(new BeliefWithState());
    //note: belief to be set each time

    //130109: change x seeker states
    if (_segmentSeekerStates) {
        cout << "b0 top - bottom to top"<<endl;
        cout << "listBottomTopStates==null:"<<(_listBottomTopStates==NULL)<< "listBottomTopStatesX==null:"<<(_listBottomTopStatesX==NULL)<<endl;
        cout <<"bottomTopSX.sz="<<_listBottomTopStatesX->size()<<" bottom x="<<_momdp->initialBeliefStval->sval<<endl;//tmp
        initialBeliefStvalTop->sval = (*_listBottomTopStatesX)[_momdp->initialBeliefStval->sval];
        cout <<"top x="<<initialBeliefStvalTop->sval<<endl;//tmp
    } else {
        cout <<"  "<<_momdp->initialBeliefStval->sval<<endl;//tmp
        initialBeliefStvalTop->sval = _momdp->initialBeliefStval->sval;
    }

    SharedPointer<SparseVector> bvecTop = initialBeliefStvalTop->bvec;
    bvecTop->resize(numStatesYTop);
    //bottom belief
    SharedPointer<SparseVector> bvec = _momdp->initialBeliefStval->bvec;

    DEBUG_HS(cout << "Print top b0: "<<endl;);

    //calculate the top b0, averaging over its bottom states beliefs
    FOR(yt,numStatesYTop) {
        //bottom states in top state r
        vector<int> yBs = _listTopBottomStates[yt];

        double p = 0;
        //sum beliefs: b(x,y) with y in Gamma(y_T)
        //note: it is a sum (not an avg) since the bottom states sum to 1.0
        FOR(yc, yBs.size()) {
            p += (*bvec)( yBs[yc]);
        }

        bvecTop->push_back(yt,p);

        //DEBUG_HS(cout << p << ",");
    }

    _hslog->print(_timer.stopTimer(timerI));

    DEBUG_HS(cout <<endl<< "Generating momdp .."<<endl;);

    //_hslog->print("Generate Top MOMDP");
    _timer.restartTimer(timerI);

    /*cout << stateListTop.size() << " StateVars: "<<endl;
    FOR(i,stateListTop.size()) {
        State s = stateListTop[i];
        cout << " - "<< s.getVNameCurr()<<": "<<s.getValueEnum().size()<<endl;
    }*/

    //TODO CREATE InitialBeliefX

    //130108: compress X belief to top
    //SharedPointer<DenseVector> initialBeliefXTop(new DenseVector(numStatesXTop));
    SharedPointer<DenseVector> initialBeliefXTop;

    if (_segmentSeekerStates) {
        initialBeliefXTop = new DenseVector(numStatesXTop);

        FOR(x,numStatesXTop) {
            //set to 0
            (*initialBeliefXTop)(x) = 0;
            //all bottom states in that top
            vector<int> bottXStates = _listTopBottomStatesX[x];
            FOR(i,bottXStates.size()) {
                (*initialBeliefXTop)(x) += (*_momdp->initialBeliefX)(bottXStates[i]);
            }
        }
    } else {
        initialBeliefXTop = _momdp->initialBeliefX;
    }

    //TODO CHECK THIS, if not works maybe other initial belief ...

    _momdpTop = MOMDP::generateMOMDP(stateListTop, //stateList,
                                     obsListTop, //_momdp->getObservationList(), //AG121221: top list of observations
                                     _momdp->getActionList(),
                                     _momdp->getRewardList(),
                                     xTransTop, //momdp->XTrans,
                                     yTransTop, //momdp->YTrans,
                                     obsProbTop, //momdp->obsProb,
                                     rewardsTop, //momdp->rewards,
                                     initialBeliefStvalTop, //momdp->initialBeliefStval,
                                     initialBeliefXTop, //_momdp->initialBeliefX,
                                     initialBeliefStvalTop->bvec, //using same since it equals the stVal belief //initialBeliefYTop, //momdp->initialBeliefY,
                                     _momdp->discount,
                                     true,
                                     MIXED,
                                     _momdp); //AG121025: bottom momdp (to copy)

    //ag120426: objects passed to generateMOMDP should be deleted by MOMDP or through SharedPointer
    //          previous MOMDP should be deleted by SharedPointer

    DEBUG_HS(HSMOMDP::showMOMDP(_momdpTop, false /*DEBUG_SHOW_TRANS*/ );); //AG12101015: false because DEBUG_SHOW_TRANS already lets show the probs

    if (_topSolver != NULL) {
        DEBUG_HS(cout<<"delete previous top solver"<<endl;);
        delete _topSolver;
    }

    DEBUG_HS(cout<<"create new top solver"<<endl;);

    _topSolver = new HSSolver();
    _topSolver->setLog(_hslog);

    DEBUG_HS(cout << "load in engine"<<endl;);

    _topSolver->setup(_momdpTop, _solverParams);

    _hslog->print(_timer.stopTimer(timerI));
    _hslog->print(_timeIter);

    //printTimeLog("gen top momdp: #X,#Y,#A,#O");
    _hslog->print(_momdpTop->XStates->size());
    _hslog->print(_momdpTop->YStates->size());
    _hslog->print(_momdpTop->actions->size());
    _hslog->printLine(_momdpTop->observations->size());

#ifdef DEBUG_COMPARE_TOP_BOTTOM
    //compare both
    compareTopAndBottomMOMDP();
#endif


//TODO: why is #o=1 -> because not set the letter set of observation???


//#ifdef DEBUG_CHECK_TOP
    checkTopMOMDP();
//#endif
    return true;
}


bool LayeredHSMOMDP::isFinalState(unsigned int x, unsigned int y, vector<int>& xBs, vector<int>& yBs, bool& winSeeker) {
    winSeeker = false;
    bool winHider = false;
    //if winSeeker && WinHider -> winSeeker

    //ag130503: set the final states on top
    if (_segmentSeekerStates) {
        //final state if there is a bottom state of X in a bottom state of Y (i.e. an intersection)
        for(unsigned int xc=0; xc<xBs.size() && !winSeeker; xc++) {
            for(unsigned int yc=0; yc<yBs.size() && !winSeeker; yc++) {
                if (xBs[xc]==yBs[yc]) { //TODO: later use distance to check if final state...
                    winSeeker = true;
                } else if (yBs[yc]==_baseState) {
                    winHider = true;
                }
            }
        }
    } else { //only segment hider states
        //AG130429: if x in super state y -> seeker wins (NOTE: or 1 dist, but first inside..)
        for(unsigned int yc=0; yc<yBs.size() && !winSeeker; yc++) {
            if (x==yBs[yc]) { //TODO: later use distance to check if final state...
                winSeeker = true;
            } else if (yBs[yc]==_baseState) {
                winHider = true;
            }
        }
    }

    return (winSeeker || winHider);
}


#ifdef DEBUG_COMPARE_TOP_BOTTOM
bool LayeredHSMOMDP::compareTopAndBottomMOMDP() {

    int numA,numX,numY,numO;
    numA=numX=numY=numO=0;

    cout << "COMPARING TOP-BOTTOM MOMDP"<<endl;

    //states
    //if (_momdp->XStates->size() != _momdpTop->XStates->size()) {
    cout << "# state vars, bottom: "<<_momdp->getStateList().size() <<": ";
    if (_momdp->getStateList().size() != _momdpTop->getStateList().size()) {
        cout << "NOT OK, top: " << _momdpTop->getStateList().size() <<endl;
        return false;
    } else {
        cout << "OK"<<endl;
    }

    FOR(i,_momdp->getStateList().size()) {
        State bState = _momdp->getStateList()[i];
        State tState = _momdpTop->getStateList()[i];
        cout << "  state var "<<i<<": "<<endl<<"    observed bottom: " <<bState.getObserved() << ": ";
        if (bState.getObserved()==tState.getObserved())
            cout << " OK"<<endl;
        else {
            cout << " NOT OK, not equal"<<endl;
            return false;
        }
        cout << "    size bottom: "<<bState.getValueEnum().size()<<": ";
        if (bState.getValueEnum().size()==tState.getValueEnum().size()) {
            cout << " OK"<<endl;
        } else {
            cout << " NOT OK, top: "<<tState.getValueEnum().size()<<endl;
            return false;
        }

        if (bState.getObserved())
            numX = bState.getValueEnum().size();
        else
            numY = bState.getValueEnum().size();
    }

    cout << "X states, bottom: "<<_momdp->XStates->size()<<": ";
    if (_momdp->XStates->size()==_momdpTop->XStates->size()) {
        cout << "OK"<<endl;
    } else {
        cout << "NOT OK, top: "<<_momdpTop->XStates->size()<<endl;
        return false;
    }
    cout << "Y states, bottom: "<<_momdp->YStates->size()<<": ";
    if (_momdp->YStates->size()==_momdpTop->YStates->size()) {
        cout << "OK"<<endl;
    } else {
        cout << "NOT OK, top: "<<_momdpTop->YStates->size()<<endl;
        return false;
    }
    cout << "Observation list, bottom: "<<_momdp->getObservationList().size()<<": ";
    if (_momdp->getObservationList().size()==_momdpTop->getObservationList().size()) {
        cout << "OK"<<endl;
    } else {
        cout << "NOT OK, top: "<<_momdpTop->getObservationList().size()<<endl;
        return false;
    }
    FORs(o,_momdp->getObservationList().size()) {
        ObsAct oaB = _momdp->getObservationList()[o];
        ObsAct oaT = _momdpTop->getObservationList()[o];
        cout << "  obs "<<o<<", bottom size: "<<oaB.getValueEnum().size()<<": ";
        if (oaB.getValueEnum().size()==oaT.getValueEnum().size()) {
            cout << "OK"<<endl;
        } else {
            cout << "NOT OK, top: "<<oaT.getValueEnum().size()<<endl;
            return false;
        }
    }

    cout << "Action list, bottom: "<<_momdp->getActionList().size()<<": ";
    if (_momdp->getActionList().size()==_momdpTop->getActionList().size()) {
        cout << "OK"<<endl;
    } else {
        cout << "NOT OK, top: "<<_momdpTop->getActionList().size()<<endl;
        return false;
    }
    FORs(a,_momdp->getActionList().size()) {
        ObsAct oaB = _momdp->getActionList()[a];
        ObsAct oaT = _momdpTop->getActionList()[a];
        cout << "  action "<<a<<", bottom size: "<<oaB.getValueEnum().size()<<": ";
        if (oaB.getValueEnum().size()==oaT.getValueEnum().size()) {
            cout << "OK"<<endl;
        } else {
            cout << "NOT OK, top: "<<oaT.getValueEnum().size()<<endl;
            return false;
        }
    }

    cout << "Reward list, bottom: "<<_momdp->getRewardList().size()<<": ";
    if (_momdp->getRewardList().size()==_momdpTop->getRewardList().size()) {
        cout << "OK"<<endl;
    } else {
        cout << "NOT OK, top: "<<_momdpTop->getRewardList().size()<<endl;
        return false;
    }
    FORs(r,_momdp->getRewardList().size()) {
        ObsAct oaB = _momdp->getRewardList()[r];
        ObsAct oaT = _momdpTop->getRewardList()[r];
        cout << "  reward "<<r<<", bottom size: "<<oaB.getValueEnum().size()<<": ";
        if (oaB.getValueEnum().size()==oaT.getValueEnum().size()) {
            cout << "OK"<<endl;
        } else {
            cout << "NOT OK, top: "<<oaT.getValueEnum().size()<<endl;
            return false;
        }
    }


    //actions:
    numA = _momdp->getNumActions();
    cout << "Actions, bottom: "<<numA<<": ";
    if (_momdp->getNumActions()==numA) {
        cout << "OK"<<endl;
    } else {
        cout << "NOT OK, top actions: "<<_momdpTop->getNumActions()<<endl;
        return false;
    }

    //observations:
    numO = _momdp->observations->size();
    cout << "Observations, bottom: "<<numO<<": ";
    if (_momdp->observations->size()==numO) {
        cout << "OK"<<endl;
    } else {
        cout << "NOT OK, top obs: "<<_momdpTop->observations->size()<<endl;
        return false;
    }

    unsigned int nDiff = 0, nTrDiff = 0, nTot=0; //tr=transposed //AG121026

    //X trans
    //cout << "get xtrans: "<<flush;
    StateTransitionX* xTrans = _momdp->XTrans;
    //cout << "ok, xtrans top: "<<flush;
    StateTransitionX* xTransTop = _momdpTop->XTrans;
    //cout << "ok"<<endl<<"xtrans==null:"<<(xTrans==NULL)<<";xtranstop==null:"<<(xTransTop==NULL)<<endl;
    //cout << "xtrans"<<endl;
    FORs(a,numA) {//cout << " a="<<a;
        FORs(x,numX) {
            //cout <<";x="<<x<<flush;
            SharedPointer<SparseMatrix> txSm = xTrans->getMatrix(a,x);
            SharedPointer<SparseMatrix> txSmTop = xTransTop->getMatrix(a,x);            
            //cout << "."<<flush;
            SharedPointer<SparseMatrix> txTrSm = xTrans->getMatrixTr(a,x);
            SharedPointer<SparseMatrix> txTrSmTop = xTransTop->getMatrixTr(a,x);

            //compare nums
            if (txSm->size1()!=txSmTop->size1()) {
                cout << " different size1 for a="<<a<<" x="<<x<<" TX.size1="<<txSm->size1()<<" TXTop.size1="<<txSmTop->size1()<<endl;
            }
            if (txSm->size2()!=txSmTop->size2()) {
                cout << " different size2 for a="<<a<<" x="<<x<<" TX.size2="<<txSm->size2()<<" TXTop.size2="<<txSmTop->size2()<<endl;
            }
            if (txSm->filled()!=txSmTop->filled()) {
                cout << " different filled for a="<<a<<" x="<<x<<" TX.filled="<<txSm->filled()<<" TXTop.filled="<<txSmTop->filled()<<endl;
            }
            if (txTrSm->size1()!=txTrSmTop->size1()) {
                cout << " different size1 for a="<<a<<" x="<<x<<" txTr.size1="<<txTrSm->size1()<<" txTrTop.size1="<<txTrSmTop->size1()<<endl;
            }
            if (txTrSm->size2()!=txTrSmTop->size2()) {
                cout << " different size2 for a="<<a<<" x="<<x<<" txTr.size2="<<txTrSm->size2()<<" txTrTop.size2="<<txTrSmTop->size2()<<endl;
            }
            if (txTrSm->filled()!=txTrSmTop->filled()) {
                cout << " different filled for a="<<a<<" x="<<x<<" txTr.filled="<<txTrSm->filled()<<" txTrTop.filled="<<txTrSmTop->filled()<<endl;
            }




            FORs(y,numY) {
                FORs(xn,numX) {
                    nTot++;
                    //normal matrix
                    double p = (*txSm)(y,xn);
                    double pTop = (*txSmTop)(y,xn);
                    if (p!=pTop) {
                        nDiff++;
                        //cout << " diff(a="<<a<<";x="<<x<<";y="<<y<<";xn="<<xn<<")[p="<<p<<";top_p="<<pTop<<"]"<<flush;
                    }

                    //tr matrix
                    p = (*txTrSm)(xn,y);
                    pTop = (*txTrSmTop)(xn,y);
                    if (p!=pTop) {
                        nTrDiff++;
                        //cout << " diffTR(a="<<a<<";x="<<x<<";y="<<y<<";xn="<<xn<<")[p="<<p<<";top_p="<<pTop<<"]"<<flush;
                    }
                }
            }
        }
    }

    cout << "X transition differencies      : "<<nDiff<<" of "<<nTot<<" ("<<(100.0*nDiff/nTot)<<"%)"<<endl;
    cout << "X TR transition differencies   : "<<nTrDiff<<" of "<<nTot<<" ("<<(100.0*nTrDiff/nTot)<<"%)"<<endl;

/*  //AG121102: Check the data in the SparseMatrix

    SharedPointer<SparseMatrix> txSm = xTrans->getMatrix(0,0);
    SharedPointer<SparseMatrix> txSmTop = xTransTop->getMatrix(0,0);
    cout << "TX data, bottom: "<<endl;
    int zcount=0;
    FORs(i,txSm->data.size()) {
        SparseVector_Entry e = txSm->data[i];
        cout << "   - "<<i<<") ["<< e.index <<"] = "<<e.value<<endl;
        if (e.value==0) zcount++;
    }
    cout << "  -> zeros: "<<zcount<<endl;
    cout << "TX data, top: "<<endl;
    zcount=0;
    FORs(i,txSmTop->data.size()) {
        SparseVector_Entry e = txSmTop->data[i];
        cout << "   - "<<i<<") ["<< e.index <<"] = "<<e.value<<endl;
        if (e.value==0) zcount++;
    }
    cout << "  -> zeros: "<<zcount<<endl;
*/

    nDiff = nTrDiff = nTot = 0;

    //Y trans
    StateTransitionY* yTrans = _momdp->YTrans;
    StateTransitionY* yTransTop = _momdpTop->YTrans;
    FORs(a,numA) {
        FORs(x,numX) {
            SharedPointer<SparseMatrix> tySm = yTrans->getMatrix(a,x,-1);        //last param is xp, but not used in the code
            SharedPointer<SparseMatrix> tySmTop = yTransTop->getMatrix(a,x,-1);

            SharedPointer<SparseMatrix> tyTrSm = yTrans->getMatrixTr(a,x,-1);
            SharedPointer<SparseMatrix> tyTrSmTop = yTransTop->getMatrixTr(a,x,-1);

            FORs(y,numY) {
                FORs(yn,numY) {
                    nTot++;
                    double p = (*tySm)(y,yn);
                    double pTop = (*tySmTop)(y,yn);
                    if (p!=pTop) {
                        nDiff++;
                        //cout << " diff(a="<<a<<";x="<<x<<";y="<<y<<";yn="<<yn<<")[p="<<p<<";top_p="<<pTop<<"]"<<flush;
                    }

                    //tr matrix
                    p = (*tyTrSm)(yn,y);
                    pTop = (*tyTrSmTop)(yn,y);
                    if (p!=pTop) {
                        nTrDiff++;
                        //cout << " diffTR(a="<<a<<";x="<<x<<";y="<<y<<";yn="<<yn<<")[p="<<p<<";top_p="<<pTop<<"]"<<flush;
                    }
                }
            }
        }
    }

    cout << "Y transition differencies      : "<<nDiff<<" of "<<nTot<<" ("<<(100.0*nDiff/nTot)<<"%)"<<endl;
    cout << "Y TR transition differencies   : "<<nTrDiff<<" of "<<nTot<<" ("<<(100.0*nTrDiff/nTot)<<"%)"<<endl;


    nDiff = nTrDiff = nTot = 0;

    //obs prob
    ObservationProbabilities* obsProb = _momdp->obsProb;
    ObservationProbabilities* obsProbTop = _momdpTop->obsProb;
    FORs(a,numA) {
        FORs(xn,numX) {
            SharedPointer<SparseMatrix> oSm = obsProb->getMatrix(a,xn);
            SharedPointer<SparseMatrix> oSmTop = obsProbTop->getMatrix(a,xn);

            SharedPointer<SparseMatrix> oTrSm = obsProb->getMatrixTr(a,xn);
            SharedPointer<SparseMatrix> oTrSmTop = obsProbTop->getMatrixTr(a,xn);

            FORs(yn,numY) {
                FORs(o,numO) {
                    nTot++;
                    double p = (*oSm)(yn,o);
                    double pTop = (*oSmTop)(yn,o);
                    if (p!=pTop) {
                        nDiff++;
                        //cout << " diff(a="<<a<<";xn="<<xn<<";yn="<<yn<<";o="<<o<<")[p="<<p<<";top_p="<<pTop<<"]"<<flush;
                    }

                    //tr matrix
                    p = (*oTrSm)(o,yn);
                    pTop = (*oTrSmTop)(o,yn);
                    if (p!=pTop) {
                        nTrDiff++;
                        //cout << " diffTR(a="<<a<<";xn="<<xn<<";yn="<<yn<<";o="<<o<<")[p="<<p<<";top_p="<<pTop<<"]"<<flush;
                    }
                }
            }
        }
    }

    cout << "O probs differencies      : "<<nDiff<<" of "<<nTot<<" ("<<(100.0*nDiff/nTot)<<"%)"<<endl;
    cout << "O TR probs differencies   : "<<nTrDiff<<" of "<<nTot<<" ("<<(100.0*nTrDiff/nTot)<<"%)"<<endl;


    nDiff = nTot = 0;

    //rewards
    Rewards* rewards = _momdp->rewards;
    Rewards* rewardsTop = _momdpTop->rewards;
    FORs(x,numX) {
        SharedPointer<SparseMatrix> rSm = rewards->getMatrix(x);
        SharedPointer<SparseMatrix> rSmTop = rewardsTop->getMatrix(x);

        FORs(y,numY) {
            FORs(a,numA) {
                nTot++;
                double p = (*rSm)(y,a);
                double pTop = (*rSmTop)(y,a);
                if (p!=pTop) {
                    nDiff++;
                    //cout << " diff(x="<<x<<";y="<<y<<";a="<<a<<")[p="<<p<<";top_p="<<pTop<<"]"<<flush;
                }
            }
        }
    }

    cout << "rewards differencies: "<<nDiff<<" of "<<nTot<<" ("<<(100.0*nDiff/nTot)<<"%)"<<endl;

}
#endif //DEBUG_COMPARE_TOP_BOTTOM


//#ifdef DEBUG_CHECK_TOP
bool LayeredHSMOMDP::checkTopMOMDP() {

    int numA,numX,numY,numO;
    numA=numX=numY=numO=0;

    cout << "CHECKING TOP MOMDP"<<endl;

    //num states X,Y
    FOR(i,_momdpTop->getStateList().size()) {
        State tState = _momdpTop->getStateList()[i];

        if (tState.getObserved())
            numX += tState.getValueEnum().size();
        else
            numY += tState.getValueEnum().size();
    }

    cout << "Num X: "<<numX<<"; num Y: "<<numY<<endl;

    //actions:
    numA = _momdpTop->getNumActions();
    cout << "Num actions: "<<numA<<endl;

    //observations:
    numO = _momdpTop->observations->size();
    cout << "Num observations: "<<numO<<endl;


    unsigned int nDiff = 0, nTrDiff = 0, nTot=0; //tr=transposed //AG121026
    //X trans
    StateTransitionX* xTransTop = _momdpTop->XTrans;
    FORs(a,numA) {
        FORs(x,numX) {
            SharedPointer<SparseMatrix> txSmTop = xTransTop->getMatrix(a,x);
            SharedPointer<SparseMatrix> txTrSmTop = xTransTop->getMatrixTr(a,x);

            FORs(y,numY) {

                //sum prob to check if is 1.0
                double pTot = 0, pTrTot = 0;

                FORs(xn,numX) {
                    nTot++;

                    double p = (*txSmTop)(y,xn);
                    pTot += p;

                    //tr matrix
                    p = (*txTrSmTop)(xn,y);
                    pTrTot += p;
                }

                //check if sum is 1
                if (pTot<1-HSGlobalData::ZERO_EPS || pTot>1+HSGlobalData::ZERO_EPS) {
                    nDiff++;
                }
                if (pTrTot<1-HSGlobalData::ZERO_EPS || pTrTot>1+HSGlobalData::ZERO_EPS) {
                    nTrDiff++;
                }

            }
        }
    }

    cout << "X transition p total not 1     : " <<nDiff<<   " of "  <<nTot<<    " ("<<(100.0*nDiff/nTot)    <<  "%)"<<endl;
    cout << "X TR transition p total not 1  : " <<nTrDiff<< " of "  <<nTot<<    " ("<<(100.0*nTrDiff/nTot)  <<  "%)"<<endl;


    nDiff = nTrDiff = nTot = 0;

    //Y trans
    StateTransitionY* yTransTop = _momdpTop->YTrans;
    FORs(a,numA) {
        FORs(x,numX) {
            SharedPointer<SparseMatrix> tySmTop = yTransTop->getMatrix(a,x,-1); //last param is xp, but not used in the code
            SharedPointer<SparseMatrix> tyTrSmTop = yTransTop->getMatrixTr(a,x,-1);

            FORs(y,numY) {

                //sum prob to check if is 1.0
                double pTot = 0, pTrTot = 0;

                FORs(yn,numY) {
                    nTot++;

                    double p = (*tySmTop)(y,yn);
                    pTot += p;

                    //tr matrix
                    p = (*tyTrSmTop)(yn,y);
                    pTrTot += p;
                }

                //check if sum is 1
                if (pTot<1-HSGlobalData::ZERO_EPS || pTot>1+HSGlobalData::ZERO_EPS) {
                    nDiff++;
                }
                if (pTrTot<1-HSGlobalData::ZERO_EPS || pTrTot>1+HSGlobalData::ZERO_EPS) {
                    nTrDiff++;
                }

            }
        }
    }

    cout << "Y transition p total not 1     : "<<nDiff<<" of "<<nTot<<" ("<<(100.0*nDiff/nTot)<<"%)"<<endl;
    cout << "Y TR transition p total not 1  : "<<nTrDiff<<" of "<<nTot<<" ("<<(100.0*nTrDiff/nTot)<<"%)"<<endl;


    nDiff = nTrDiff = nTot = 0;

    //obs prob
    ObservationProbabilities* obsProbTop = _momdpTop->obsProb;
    FORs(a,numA) {
        FORs(xn,numX) {
            SharedPointer<SparseMatrix> oSmTop = obsProbTop->getMatrix(a,xn);
            SharedPointer<SparseMatrix> oTrSmTop = obsProbTop->getMatrixTr(a,xn);

            FORs(yn,numY) {

                //sum prob to check if is 1.0
                double pTot = 0, pTrTot = 0;

                FORs(o,numO) {
                    nTot++;

                    double p = (*oSmTop)(yn,o);
                    pTot += p;

                    //tr matrix
                    p = (*oTrSmTop)(o,yn);
                    pTrTot += p;
                }

                //check if sum is 1
                if (pTot<1-HSGlobalData::ZERO_EPS || pTot>1+HSGlobalData::ZERO_EPS) {
                    nDiff++;
                }
                if (pTrTot<1-HSGlobalData::ZERO_EPS || pTrTot>1+HSGlobalData::ZERO_EPS) {
                    nTrDiff++;
                }
            }
        }
    }

    cout << "O probs total not 1     : "<<nDiff<<" of "<<nTot<<" ("<<(100.0*nDiff/nTot)<<"%)"<<endl;
    cout << "O TR probs total not 1  : "<<nTrDiff<<" of "<<nTot<<" ("<<(100.0*nTrDiff/nTot)<<"%)"<<endl;


    nDiff = nTot = 0;

    double rewMin, rewMax, rewSum;
    rewSum = 0;
    rewMin = DBL_MAX;
    rewMax = -DBL_MAX;

    //rewards
    Rewards* rewardsTop = _momdpTop->rewards;
    FORs(x,numX) {
        SharedPointer<SparseMatrix> rSmTop = rewardsTop->getMatrix(x);

        FORs(y,numY) {
            FORs(a,numA) {
                nTot++;
                double p = (*rSmTop)(y,a);

                //stats rew
                rewSum += p;
                if (p<rewMin) rewMin = p;
                if (p>rewMax) rewMax = p;
            }
        }
    }

    cout << "rewards: "<<nTot<<", sum: "<<rewSum <<", avg: "<<(rewSum/nTot)<<", min: "<<rewMin<<", max: "<<rewMax << endl;
    cout << endl;

}
//#endif //DEBUG_CHECK_TOP


vector<string> LayeredHSMOMDP::generateTopStates(int numActions, int numStates, Segmenter* segmenter, vector<int>* & listBottomTopStates,
                                                 vector< vector<int> >& listTopBottomStates) {
    DEBUG_HS1(cout << "LayeredHSMOMDP::generateTopStates"<<endl;);

    //list of scores per state
    vector<double> scoreVec(numStates);

    //get rewards for current x -> [y,a]
    SharedPointer<SparseMatrix> rewMat =_momdp->rewards->getMatrix(_currBelief->sval);
    //current Y belief
    SharedPointer<SparseVector> belY = _currBelief->bvec;

    //debug check
    assert(rewMat->size1()==numStates);
    assert(rewMat->size2()==numActions);
    assert(_currBelief != NULL);

    DEBUG_HS1(cout << " init ok, now FOR y "<<endl;);

    //calculate scores
    //NOTE: 1. using reward and belief
    //      2. for rewards the average over all actions is used -> ok??
    FOR(s,numStates) {
        double w=0;
        //sum rewards for all actions
        if (_segmentValueType==SEGMENT_VALUE_REWARD_ONLY || _segmentValueType==SEGMENT_VALUE_BELIEF_REWARD) {
            FOR(a,numActions) {
                w += (*rewMat)(s,a);
            }

            //normalize
            w = w/numActions;
        }

        //include belief
        if (_segmentValueType==SEGMENT_VALUE_BELIEF_REWARD) {
            //multiply reward by belief
            w *= (*belY)(s);
        } else if (_segmentValueType==SEGMENT_VALUE_BELIEF_ONLY) {
            w = (*belY)(s);
        }
        //normalize for actions and multiply by belief

        scoreVec[s] = w;
    }

    //print value
    int i = 0;
    for (int r=0; r<_gmap->rowCount(); r++) {
        for (int c=0; c<_gmap->colCount(); c++) {
            if (_gmap->isObstacle(r,c)) {
                //print obst
                cout << "[XXXXX]";
            } else {
                //print others, check for hider/seeker/base
                double p = scoreVec[i];
                char s1='[',s2=']';

                if (_gmap->isBase(r,c)) {
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



    DEBUG_HS1(cout << " segment states... "<<endl;);

    //ag120426: delete previous segments
    if (listBottomTopStates!=NULL) delete listBottomTopStates;

    //segment
    int segCount = -1;
    //gives for each bottom state i, the top state j ([i]->j)
    listBottomTopStates = segmenter->segment(&scoreVec, segCount);
    if (segCount<=0) {
        cout << "ERROR: @ hsmomdp.generatTopStates: no segments found"<<endl;
        exit(EXIT_FAILURE);
    }

    DEBUG_HS1(cout << " segments found: "<< segCount << "; segment vector size: " << listBottomTopStates->size() <<endl;);

    //now generate lists of bottom states j_x per top state i: [i]->{j_x..}
    listTopBottomStates.clear(); //ag120306: clean previous iteration states
    listTopBottomStates.resize(segCount);
    FOR(i,listBottomTopStates->size()) {
        listTopBottomStates[(*listBottomTopStates)[i]].push_back(i);
    }

    DEBUG_HS(cout << "Bottom states per top state:"<<endl;);

    //generate states
    vector<string> ssVec;
    FOR(t,segCount) {
        stringstream ss;
        ss << "t" << t;
        ssVec.push_back(ss.str());

#ifdef DEBUG_HS_ON
        vector<int>::iterator topSIt;
        cout << " - top "<<t<<": ";
        for(topSIt = listTopBottomStates[t].begin(); topSIt != listTopBottomStates[t].end(); topSIt++) {
            cout << *topSIt << ", ";
        }
        cout << endl;
#endif
    }

    DEBUG_HS(cout << "Segments of the map:"<<endl;);
    DEBUG_HS(segmenter->showMap(listBottomTopStates,_x_state,_observ /*_y_state*/););


    return ssVec;
}


bool LayeredHSMOMDP::compressBelief() {
    DEBUG_HS(cout << "*** LayeredHSMOMDP.compressBelief ***"<<endl;);
    _hslog->print("Compress belief");
    int timerI = _timer.startTimer();

    //num top Y states
    unsigned int numStatesYTop = _listTopBottomStates.size();
    //get bottom belief
    SharedPointer<BeliefWithState> b0Bottom = _solver.getCurrentBeliefState();

    //top belief
    SharedPointer<BeliefWithState> b0Top(new BeliefWithState());
    //copy x state ('belief')
    //AG130110: set top state based on segmentation
    if (_segmentSeekerStates) {
        b0Top->sval = (*_listBottomTopStatesX)[b0Bottom->sval];
    } else {
        b0Top->sval = b0Bottom->sval;
    }

    //now 'compress' bottom belief, by adding probabilities
    SharedPointer<SparseVector> bvecBottom = b0Bottom->bvec;
    SharedPointer<SparseVector> bvecTop = b0Top->bvec;
    bvecTop->resize(numStatesYTop);

    DEBUG_HS(cout <<"calc top b0:"<<endl;);
    FOR(yt,numStatesYTop) {
        //bottom states for top state i
        vector<int> yBs = _listTopBottomStates[yt];
        double p = 0;

        DEBUG_HS(cout << " - " <<yt<<": ";);

        //sum belief of bottom states (NOT average)
        FOR(y,yBs.size()) {
            p += (*bvecBottom)(yBs[y]);
            DEBUG_HS(cout << "," << yBs[y];);
        }

        DEBUG_HS(cout << " - " << p << endl;);

        //set as belief for top state
        bvecTop->push_back(yt,p);
    }


    DEBUG_HS(HSSolver::printBelief(b0Top,"b0 top"););


    //AG130101: WARNING: this should also contain check for X segmenter .. but since it only depends on map, keep it
    if (_action!=HSGlobalData::ACT_H || _segmenter->segmentsCanChangeWhenSeekerHalt()) {
        //AG120904: when last action was halt, and segmentation is same when same position (i.e. halt), then redo
        //now init top enigine
        _topSolver->init(*b0Top, /*_y_state,*/ _applLogStream);
    } else {
        //only set belif
        _topSolver->setBelief(*b0Top);
    }

    _hslog->print(_timer.stopTimer(timerI));
    _hslog->printLine(_timeIter);

#ifdef DEBUG_COMPARE_TOP_BOTTOM
    cout << " comparing beliefs, bottom size: " << bvecBottom->size()<<": ";
    if (bvecBottom->size()==bvecTop->size()) {
        cout << "OK"<<endl;
        unsigned int n=0,d=0;
        FOR(y,bvecTop->size()) {
            n++;
            if ( (*bvecBottom)(y) != (*bvecTop)(y) ) d++;
        }
        cout << "Belief different: "<<d<<" of "<<n<<" ("<<(100.0*d/n)<<"%)"<<endl;
    } else {
        cout << "NOT OK, bottom: "<<bvecTop->size()<<endl;
    }
#endif
    return true;
}


int LayeredHSMOMDP::getBestAction() {
    DEBUG_HS(cout << "*** LayeredHSMOMDP.getBestAction ***"<<endl;);

    //AG130101: WARNING: also depends on segmenterX, but since it only depends on map for now doesn't matter
    if (_action!=HSGlobalData::ACT_H || _segmenter->segmentsCanChangeWhenSeekerHalt()) {
        //AG120904: when last action was halt, and segmentation is same when same position (i.e. halt)
        //  then skip this part; otherwise redo solving
        _hslog->print("Solve");
        int timerI = _timer.startTimer();

        //solve MOMDP (and get the policy)
        //_topSolver->setup(_solver.getMOMDP(),_solverParams);///
        SARSOPSolveState* solveStats = _topSolver->solve(_timeIter);
        //SARSOPSolveState* solveStats = _solver.solve(_timeIter); //AG121016: tested if with orig solver generates same policy.. and it did -> diff in model..


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
    }


    _hslog->print("Best Action");
    int timerI = _timer.startTimer();//_timer.restartTimer(timerI);

    //find the best action to do for our state
    double reward,bottomReward;//ag120904: reward
    int bottomAction,rankTopInBottom,rankBottomInTop;

    //compare actions with offline
    //NOTE: only works without 'best action lookahead'
    if (_compareWithOffline) {
        cout<<"comp with offline"<<endl;

        //vectors of value per action
        vector<double> maxValPerA_top, maxValPerA_bottom;

        cout << " top action: "<<flush;
        //get action and compare action vectors
        _action = _topSolver->getBestAction(reward, &maxValPerA_top);

        cout << _action << endl << " bottom action: "<<flush;
        //bottom layer
        bottomAction = _solver.getBestAction(bottomReward, &maxValPerA_bottom);
        cout << bottomAction << endl;

        cout << "Layered action: "<<_action<<" ; offline action: " << bottomAction<<endl;
        //TODO: compairson of ranking of the values of the actions!!
        //      what is rank of _action in maxValPerA_bottom

        rankTopInBottom = getRankOfAction(_action, &maxValPerA_bottom);
        rankBottomInTop = getRankOfAction(bottomAction, &maxValPerA_top);
        cout << "   rank of layered action in bottom: "<<rankTopInBottom<<endl;
        cout << "   rank of bottom action in layered: "<<rankBottomInTop<<endl;

    } else {
        //only get action
        _action = _topSolver->getBestAction(reward);
    }

    _hslog->print(_timer.stopTimer(timerI));
    _hslog->print(_timeIter);
    _hslog->print(_action);    

    if (_compareWithOffline) {
        _hslog->print(reward);//ag120904: reward
        _hslog->print(bottomAction);
        _hslog->print(bottomReward);
        _hslog->print(rankTopInBottom);
        _hslog->printLine(rankBottomInTop);
    } else {
        _hslog->printLine(reward);//ag120904: reward
    }



    _timeIter++;

    return _action;
}

int LayeredHSMOMDP::getRankOfAction(int a, vector<double>* valPerAction) {
    //get value from which we want rank
    double va = (*valPerAction)[a];

    //sort and remove unique items for ranking
    vector<double> valPerA(*valPerAction);
    sort(valPerA.begin(), valPerA.end());
    valPerA.erase(std::unique(valPerA.begin(), valPerA.end()), valPerA.end());

    //check ranking
    int rank = 1;
    //cout << "[";
    for(int i=0; i<valPerA.size(); i++) {
        //get value
        double v = (valPerA)[i];
        //cout << v <<",";
        //if value > value of action -> increase rank
        if (v>va) rank++;
    }
    //cout << "]"<<endl;

    return rank;
}



int LayeredHSMOMDP::getNextAction(int observ, int x_state) {
    DEBUG_HS(cout << "***-- LayeredHSMOMDP.getNextAction --***"<<endl;);

    //1. Update belief
    if (_action!=-1) { //i.e. not the first step
        updateBelief(observ,x_state);
    }

    //AG130101: WARNING: also depends on segmenterX, but since it only depends on map for now doesn't matter
    if (_action!=HSGlobalData::ACT_H|| _segmenter->segmentsCanChangeWhenSeekerHalt()) {
        //AG120904: when last action was halt, and segmentation is same when same position (i.e. halt)
        //  then skip this part

        //2. Generate Top MOMDP
        generateTopMOMDP();

        /*cout << "----------- after generate top momdp, check, show ------"<<endl;*/

        DEBUG_HS(showMOMDP(_momdpTop, DEBUG_SHOW_TRANS);); //ag130716: only deug mode 
    }

    //3. Compress belief
    compressBelief();


    //4. Get action
    return getBestAction();
}


/*! Get next best action to do, based on the Y state (hider pos), X state (seeker pos) and visibility of the hider
note: for hierarchical can be 1-5 */
int LayeredHSMOMDP::getNextAction(Pos seekerPos, Pos hiderPos, bool visible) {
    if (_segmenter != NULL) _segmenter->setPosition(seekerPos.row, seekerPos.col);
    if (_segmenterX != NULL) _segmenterX->setPosition(seekerPos.row, seekerPos.col); //AG130101
    return HSMOMDP::getNextAction(seekerPos, hiderPos, visible);
}


vector<int> LayeredHSMOMDP::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n) {
    throw CException(_HERE_, "LayeredHSMOMDP::getNextMultipleActions: not yet implemented");
}
