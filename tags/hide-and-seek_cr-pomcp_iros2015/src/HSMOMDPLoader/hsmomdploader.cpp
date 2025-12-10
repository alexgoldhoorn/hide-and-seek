/*
c. problem loading 'big' map → out of memory
This happened by 40x40 POMDPX.
Solution: we have a lot of 0 or 1s → and determenistic
We can store the following functions differently:
p(x'|x,a): deterministic → by action, pos and map
p(y'|x,y,x',a):
when we user the RandomHider → deterministic (ie uniformly distributed)
we can also use historical data to calculate them, but then it is sufficient to store
p(ay|x,y) → probability of doing a certain user action given state
→ this can be transformed to p(y'|x,y,x',a)
p(o|x',y'): depends directly on the visibility which can be stored in a |X|x|Y| matrix
The probability is the deterministic based on that matrix.
R(x,y,a): formula based on distance, can be done online only storing the distance matrix:
|X|x|Y|


NOTE: instead of generating the matrices,
a specific matrix Class has to be made for each prob-matrix: p(x|..); p(y|..); p(o|..); ...
 */

#ifdef _NEW_CODE

#include "HSMOMDPLoader/hsmomdploader.h"

HSMOMDPLoader::HSMOMDPLoader()
{
}

SharedPointer<MOMDP> HSMOMDPLoader::loadMOMDPFromFile(char *file) {
    //bool LayeredHSMOMDP::generateTopMOMDP() {
    assert(_momdp.get()!=NULL); //TODO: check what to use for NULL check for boost shared pointer

    DEBUG_HS(cout << "*** LayeredHSMOMDP.generateTopMOMDP ***"<<endl;);
    _hslog->print("Generate Top MOMDP");
    cout << "#X:"<<_momdp->XStates->size()<<endl;
    cout << "#Y:"<<_momdp->YStates->size()<<endl;
    //_hslog->print(" * generate states");
    int timerI = _timer.startTimer();


    //first generate new states
    DEBUG_HS1(cout << "get bottom states: "<<flush;);
//cout <<"statlist.sz="    <<_momdp->getStateList().size()<<endl;
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

    ///[ag120322: here was state y gen]


    DEBUG_HS1(cout << "ok"<<endl<<"generate state X,Y: "<<flush;);


    //generate top Y state
    State stateYTop;
    stateYTop.setObserved(false);
    stateYTop.setVNamePrev("y0");
    stateYTop.setVNameCurr("y1");

    DEBUG_HS1(cout << "gen top "<<flush;);

    //segment to generate regions
    vector<string> ssyvec = generateTopStates(numActions, numStatesY);

    cout <<"Y top:"<<flush;
    for(vector<string>::iterator vecit=ssyvec.begin();vecit!=ssyvec.end();vecit++) {
        cout << (*vecit)<<","<<flush;
    }
    cout << endl;


    //set state values of top state y
    stateYTop.setValueEnum(ssyvec);
    //num states
    unsigned int numStatesYTop = ssyvec.size();

    //gen x state vector
   /* vector<string> ssxvec;
    for(unsigned int i=0;i<numStatesX;i++) {
        stringstream ss;
        ss << "X"<<i;
        ssxvec.push_back(ss.str());
    }
    stateXTop->setValueEnum(ssxvec);*/


    //add states to list
    stateListTop.push_back(*stateX);
    stateListTop.push_back(stateYTop);

    DEBUG_HS1(cout << "ok"<<endl<<""<<flush;);

    _hslog->print(_timer.stopTimer(timerI));


    DEBUG_HS(cout << "TOP momdp count: #A: " << numActions << ", #Y (b): "<<numStatesY<<", # Y (top): "<<numStatesYTop<<", #X: "<<numStatesX<<", #O: "<<numObs<<endl;);
    DEBUG_HS(if (numObs<=numStatesY) cout << "WARNING: NOT right number of observation"<<endl;);

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

    //create top transition X
    FOR (a, numActions) {
        xmatrixTop[a].resize(numStatesX);
        xmatrixTopTr[a].resize(numStatesX);

        //cout << " a="<<a<<endl;

        FOR (x, numStatesX) {
            //bottom transitions (a,x)->[y,x']
            SharedPointer<SparseMatrix> txSm = xTrans->getMatrix(a,x);

            //top transitions
            SharedPointer<SparseMatrix> txTopSm(new SparseMatrix(numStatesYTop,numStatesX));
            SharedPointer<SparseMatrix> txTopSmTr(new SparseMatrix(numStatesX,numStatesYTop));


            //cout << " x="<<x<<endl;

            FOR(c, numStatesX) {
                FOR(r, numStatesYTop) {
                   // cout <<"("<<r<<","<<c<<")"<<flush;
                    //current y states (list of bottom states) in top state r
                    vector<int> yBs = _listTopBottomStates[r];
                    double p =0;

                    //sum all probabilities: p(x'|y,x,a) with: y in Gamma(y_T),
                    FOR(yc, yBs.size()) {
                        p += (*txSm)(yBs[yc], c);
                    }
                    //get average
                    p /= yBs.size();
                    //put in top matrix
                    txTopSm->push_back(r,c,p);
                    txTopSmTr->push_back(c,r,p);
                }
            }
            //put in bigger matrix
            xmatrixTop[a][x] = txTopSm;
            xmatrixTopTr[a][x] = txTopSmTr;
            //cout<<"x";
        }
        //cout<<endl;
    }

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
            cout <<", s1:"<<s1<<", s2:"<<s2<<endl;

            for (int i=0;i<s1;i++) {
                for (int j=0;j<s2;j++) {
                    cout <<"    ("<<i<<","<<j<<"): "<<flush;
                    cout <<(*mat)(i,j)<<endl;
                }
            }
        }
    }
#endif


    DEBUG_HS(cout << "Calc Y trans top .."<<endl;);

    //_hslog->print(" * calculate Y top trans probs");
    _timer.restartTimer(timerI);

    //get Y trans of bottom
    StateTransitionY* yTrans = _momdp->YTrans;

    //create top transition Y
    FOR (a, numActions) {
        ymatrixTop[a].resize(numStatesX);
        ymatrixTopTr[a].resize(numStatesX);

        FOR (x, numStatesX) {
            //bottom transitions (a,x)->[y,y']
            SharedPointer<SparseMatrix> tySm = yTrans->getMatrix(a,x,0); //note: 0: xp-> not used
            //SharedPointer<SparseMatrix> tySmTr(new SparseMatrix(numStatesY,numStatesY));

            //top transitions
            SharedPointer<SparseMatrix> tyTopSm(new SparseMatrix(numStatesYTop,numStatesYTop));
            SharedPointer<SparseMatrix> tyTopSmTr(new SparseMatrix(numStatesYTop,numStatesYTop));

            FOR(c, numStatesYTop) {
                //new y states (list of bottom states) in top state c
                vector<int> yBSn = _listTopBottomStates[c];

                FOR(r, numStatesYTop) {
                    //current y states (list of bottom states) in top state r
                    vector<int> yBs = _listTopBottomStates[r];
                    double p =0;

                    //sum all probabilities: p(y'|y,x,a) with: y in Gamma(y_T),y' in Gamma(y_T')
                    FOR(yc, yBs.size()) {
                        FOR(yn, yBSn.size()) {
                            p += (*tySm)(yBs[yc], yBSn[yn]);
                        }
                    }

                    p /= yBs.size();

                    tyTopSm->push_back(r,c,p);
                    tyTopSmTr->push_back(c,r,p);
                }
            }

            ymatrixTop[a][x] = tyTopSm;
            ymatrixTopTr[a][x] = tyTopSmTr;
        }
    }

    //generate T_{Y,new}
    StateTransitionXY* yTransTop = new StateTransitionXY(ymatrixTop,ymatrixTopTr);

    _hslog->print(_timer.stopTimer(timerI));

#if defined(DEBUG_HS_ON) && defined(DEBUG_SHOW_TRANS_ON)
    cout << "top YTrans-print:"<<endl;
    FOR(a,numActions) {
        FOR(x,numStatesX) {
            FOR(y,numStatesYTop) {
                cout << " - a:"<<a<<",x:"<<x<<", y:"<<y;

                SharedPointer<SparseMatrix> mat = yTransTop->getMatrix(a,x,y);
                int s1 = mat->size1();
                int s2 = mat->size2();
                cout <<", s1:"<<s1<<", s2:"<<s2<<endl;

                for (int i=0;i<s1;i++) {
                    for (int j=0;j<s2;j++) {
                        cout <<"    ("<<i<<","<<j<<"): "<<flush;
                        cout <<(*mat)(i,j)<<endl;
                    }
                }

            }
        }
    }
#endif


    DEBUG_HS(cout <<"Calc O prob top .."<<endl;);

    //_hslog->print(" * calculate top observations probs");
    _timer.restartTimer(timerI);

    //get probab of o
    ObservationProbabilities* obsProb = _momdp->obsProb;

    //create top obs probs
    FOR (a, numActions) {
        omatrixTop[a].resize(numStatesX);
        omatrixTopTr[a].resize(numStatesX);

        FOR (x, numStatesX) {
            //bottom transitions (a,x)->[y,y']
            SharedPointer<SparseMatrix> oSm = obsProb->getMatrix(a,x); //note: 0: xp-> not used
            //SharedPointer<SparseMatrix> oSmTr(new SparseMatrix(numStatesY,numStatesY));

            //top obs prob
            SharedPointer<SparseMatrix> oTopSm(new SparseMatrix(numStatesYTop,numObs));
            SharedPointer<SparseMatrix> oTopSmTr(new SparseMatrix(numObs,numStatesYTop));

            FOR(c, numObs) {
                FOR(r, numStatesYTop) {
                    //bottom states in top state r
                    vector<int> yBSn = _listTopBottomStates[r];
                    double p =0;

                    //sum all probabilities: p(o'|x',y',a) with: y' in Gamma(y_T')
                    FOR(yn, yBSn.size()) {
                        p += (*oSm)(yBSn[yn], c);
                    }

                    p /= yBSn.size();

                    oTopSm->push_back(r,c,p);
                    oTopSmTr->push_back(c,r,p);
                }
            }

            omatrixTop[a][x] = oTopSm;
            omatrixTopTr[a][x] = oTopSmTr;
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
            cout <<", s1:"<<s1<<", s2:"<<s2<<endl;

            for (int i=0;i<s1;i++) {
                for (int j=0;j<s2;j++) {
                    cout <<"    ("<<i<<","<<j<<"): "<<flush;
                    cout <<(*mat)(i,j)<<endl;
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

    //create top rewards
    FOR(x, numStatesX) {
        //bottom reward (x)->[y,a]
        SharedPointer<SparseMatrix> rSm = rewards->getMatrix(x);
        SharedPointer<SparseMatrix> rTopSm(new SparseMatrix(numStatesYTop,numActions));

        FOR(c, numActions) {
            FOR(r, numStatesYTop) {
                //bottom states in top state r
                vector<int> yBs = _listTopBottomStates[r];
                double p =0;

                //sum all rewards: R(x,y,a) with: y in Gamma(y_T)
                FOR(yc, yBs.size()) {
                    p += (*rSm)(yBs[yc], c);
                }

                p /= yBs.size();

                rTopSm->push_back(r,c,p);
            }
        }

        rmatrixTop[x] = rTopSm;
    }

    //top rewards
    Rewards* rewardsTop = new Rewards(rmatrixTop);

    _hslog->print(_timer.stopTimer(timerI));

#if defined(DEBUG_HS_ON) && defined(DEBUG_SHOW_TRANS_ON)
    cout << "Print Reward:"<<endl;
    //for (int a=0;a<numA;a++) {
    FOR(x,numStatesX) {
        cout << " - x:"<<x;

        SharedPointer<SparseMatrix> mat = rewardsTop->getMatrix(x);
        int s1 = mat->size1();
        int s2 = mat->size2();
        cout <<", s1:"<<s1<<", s2:"<<s2<<endl;

        for (int i=0;i<s1;i++) {
            for (int j=0;j<s2;j++) {
                cout <<"    ("<<i<<","<<j<<"): "<<flush;
                cout <<(*mat)(i,j)<<endl;
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
    initialBeliefStvalTop->sval = _momdp->initialBeliefStval->sval;
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

        DEBUG_HS(cout << p << ",");
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


    _momdpTop = MOMDP::generateMOMDP(stateListTop, //stateList,
                                     _momdp->getObservationList(),
                                     _momdp->getActionList(),
                                     _momdp->getRewardList(),
                                     xTransTop, //momdp->XTrans,
                                     yTransTop, //momdp->YTrans,
                                     obsProbTop, //momdp->obsProb,
                                     rewardsTop, //momdp->rewards,
                                     initialBeliefStvalTop, //momdp->initialBeliefStval,
                                     _momdp->initialBeliefX,
                                     initialBeliefStvalTop->bvec, //using same since it equals the stVal belief //initialBeliefYTop, //momdp->initialBeliefY,
                                     _momdp->discount,
                                     true,
                                     MIXED);


    DEBUG_HS(HSMOMDP::showMOMDP(_momdpTop, DEBUG_SHOW_TRANS););

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

    return true;

}

#endif
