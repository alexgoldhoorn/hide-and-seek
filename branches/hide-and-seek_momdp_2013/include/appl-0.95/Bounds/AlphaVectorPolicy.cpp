#include <fstream>
#include <cfloat>
#include <cstdlib>
#include <cstdio>
extern "C" {
#include "xml_parse_lib.h"
}
#include "AlphaVectorPolicy.h"
#include "AlphaPlanePool.h"
#include "AlphaPlanePoolSet.h"

#define MaxStr 2048


using namespace std;
using namespace momdp;

AlphaVectorPolicy::AlphaVectorPolicy(SharedPointer<MOMDP> problem) {
    this->problem = problem;
    this->policyFile = policyFile;
    alphaPlanePoolSet = new AlphaPlanePoolSet(NULL);
    alphaPlanePoolSet->setProblem(problem);
    alphaPlanePoolSet->initialize();
}
/**********************************************************************
*READ POLICY
**********************************************************************/
static std::string stripWhiteSpace(const std::string& s) {
    string::size_type p1, p2;
    p1 = s.find_first_not_of(" \t");
    if (string::npos == p1) {
        return s;
    } else {
        p2 = s.find_last_not_of(" \t")+1;
        return s.substr(p1, p2-p1);
    }
}

bool AlphaVectorPolicy::readFromFile(const std::string& inFileName) {

    char tag[MaxStr], contents[MaxStr], tagname[MaxStr], attrname[MaxStr], value[MaxStr];
    float x1, y1, z1, x2, y2, z2, t0, t1;
    int linum=0;
    FILE *infile=0, *outfile=0;

    infile = fopen(inFileName.c_str(),"r");
    if(infile==0) {
        cerr << "ERROR: couldn't open " << inFileName << " for reading " << endl;
        exit(EXIT_FAILURE);
    }
    xml_parse( infile, tag, contents, MaxStr, &linum );
    xml_grab_tag_name( tag, tagname, MaxStr );	/* Get tag name. */
    xml_grab_attrib( tag, attrname, value, MaxStr );	/* Get next attribute, if any. */
    while(value[0] != '\0') {
        xml_grab_attrib( tag, attrname, value, MaxStr );	/* Get next attribute, if any. */
    }

    xml_parse( infile, tag, contents, MaxStr, &linum );
    xml_grab_tag_name( tag, tagname, MaxStr );	/* Get tag name. */
    if(string(tagname)!="Policy") {
        cerr << "ERROR:\n\tExpected Policy tag as root" << endl;
        exit(EXIT_FAILURE);
    }

    xml_grab_attrib( tag, attrname, value, MaxStr );	/* Get any attributes within tag. */
    while(value[0] != '\0') {
        if(string(attrname)=="type" && string(value)!="value") {
            cerr << "ERROR:\n\tOnly policy of type \"value\" is supported" << endl;
            exit(EXIT_FAILURE);
        }
        xml_grab_attrib( tag, attrname, value, MaxStr );	/* Get next attribute, if any. */
    }

    xml_parse( infile, tag, contents, MaxStr, &linum );
    xml_grab_tag_name( tag, tagname, MaxStr );	/* Get tag name. */
    if(string(tagname)!="AlphaVector") {
        cerr << "ERROR:\n\tExpected AlphaVector tag" << endl;
        exit(EXIT_FAILURE);
    }
    int vectorLength = -1;

    xml_grab_attrib( tag, attrname, value, MaxStr );	/* Get any attributes within tag. */
    while(value[0] != '\0') {
        if(string(attrname)=="vectorLength") {
            vectorLength = atoi(value);
        }
        xml_grab_attrib( tag, attrname, value, MaxStr );
    }

    if(vectorLength == -1) {
        cerr << "ERROR:\n\tCannot find integer attribute vectorLength in AlphaVector Tag" << endl;
        exit(EXIT_FAILURE);
    }
    if(vectorLength != problem->YStates->size()) {
        cerr << "ERROR:\n\tVector length does not match problem unobserved state size" << endl;
        exit(EXIT_FAILURE);
    }

    AlphaPlane plane;
    double entryVal;
    xml_parse_tag_only( infile, tag, MaxStr, &linum );		//read the vector tag
    xml_grab_tag_name( tag, tagname, MaxStr);
    while( tag[0]!='\0') {
        //initialise the alpha plane
        plane.alpha->resize(problem->YStates->size());
        bool foundAct = false;
        bool foundObs = false;
        xml_grab_attrib( tag, attrname, value, MaxStr );
        while(value[0] != '\0') {
            if(string(attrname)=="action") {
                plane.action = atoi(value);
                foundAct = true;
            } else if(string(attrname)=="obsValue") {
                plane.sval = atoi(value);
                foundObs = true;
            }
            xml_grab_attrib( tag, attrname, value, MaxStr );
        }

        if(!foundAct) {
            cerr << "ERROR:\n\tCannot find integer attribute action in Vector tag" << endl;
            assert(false);
            exit(EXIT_FAILURE);
        }
        if(!foundObs) {
            cerr << "ERROR:\n\tCannot find integer attribute obsValue in Vector tag" << endl;
            exit(EXIT_FAILURE);
        }

        if(plane.sval >= alphaPlanePoolSet->set.size()) {
            cerr << "Policy file has more observed state than given POMDP model's, are you using the correct policy file?" << endl;
            exit(EXIT_FAILURE);
        }

        //check if vector is dense or sparse
        if(string(tagname)=="Vector") {
            FOR(i, vectorLength) {
                char  dvalue[200];
                if(fscanf(infile, "%s", &dvalue)==EOF) {
                    cerr << "ERROR:\n\tVector is too short, are you using the correct policy file?" << endl;
                    exit(EXIT_FAILURE);
                }
                plane.alpha->data[i] =atof(dvalue);
            }
            alphaPlanePoolSet->set[plane.sval]->planes.push_back(plane.duplicate());
        } else if(string(tagname)=="SparseVector") {
            xml_parse( infile, tag, contents, MaxStr, &linum );
            xml_grab_tag_name( tag, tagname, MaxStr);
            while(string(tagname)=="Entry") {
                double value;
                int index;
                sscanf(contents, "%d %f", &index, &value);
                plane.alpha->data[index] = value;

                xml_parse( infile, tag, contents, MaxStr, &linum );
                xml_parse( infile, tag, contents, MaxStr, &linum );
                xml_grab_tag_name( tag, tagname, MaxStr);
            }
            alphaPlanePoolSet->set[plane.sval]->planes.push_back(plane.duplicate());
        }
        xml_parse(infile, tag,contents, MaxStr, &linum );
        xml_parse_tag_only( infile, tag, MaxStr, &linum );		//read the vector tag
        xml_grab_tag_name( tag, tagname, MaxStr);
    }
    fclose(infile);

    return true;
}

int AlphaVectorPolicy::getBestAction(BeliefWithState& b) {
    double maxValue;
    return getBestAction(b, maxValue);
}


#ifdef DEBUG_AG_POL   //AG121009: for check of actions


//AG121008
// p(o|b,a) = sum_s' [ p(o|s',a) ( sum_s(p(s'|s,a) b(s)) ) ] = 1/eta
double AlphaVectorPolicy::ag_p_o_ba(SharedPointer<MOMDP> momdp, BeliefWithState& b, int o, int a ) {
    //probabilities
    StateTransitionX* transX = momdp->XTrans;
    StateTransitionY* transY = momdp->YTrans;
    ObservationProbabilities* obsProb = momdp->obsProb;

    cout << "< p(o|b,a) >"<<endl;

    double p_o_ba = 0;

    //calc probs
    FORs(xn, momdp->XStates->size()) {
        //cout << "           xn="<<xn<<endl;
        //obs prob
        SharedPointer<SparseMatrix> oPmat = obsProb->getMatrix(a,xn);

        FORs(yn, momdp->YStates->size()) {
            //cout << "           yn="<<yn<<endl;
            double oP = (*oPmat)(yn,o);

            double sumIn = 0;

            FORs(x, momdp->XStates->size()) {
                //cout << "           x="<<x<<endl;

                //x trans mat
                SharedPointer<SparseMatrix> xTransMat = transX->getMatrix(a,x);

                FORs(y, momdp->YStates->size()) {
                    //cout << "           y="<<y<<endl;

                    double xP = (*xTransMat)(y,xn);

                    //y trans mat
                    SharedPointer<SparseMatrix> yTransMat = transY->getMatrix(a,x,xn);
                    double yP = (*yTransMat)(y,yn);

                    double bxy = (*b.bvec)(y);
                    sumIn += xP * yP * bxy;
                }
            }

            p_o_ba += oP * sumIn;
        }
    }

    return p_o_ba;
}


//AG121009
// t_M(x,b_y,a,o,x') = eta * p(o|x',y',a)* sum_y( p(x'|x,y,a) p(y'|x,y,x',a) b(x,y)
double AlphaVectorPolicy::ag_t_M(SharedPointer<MOMDP> momdp, BeliefWithState& b, int x, int a, int o, int xn, int yn ) {
    //probabilities
    StateTransitionX* transX = momdp->XTrans;
    StateTransitionY* transY = momdp->YTrans;
    ObservationProbabilities* obsProb = momdp->obsProb;

    cout << "< t_M(x,b_y,a,o,x) >"<<endl;

    double t_M = 0;

    //eta_ = 1/eta
    double eta_ = ag_p_o_ba(momdp, b, o, a);

    cout<<" get opmat a="<<a<<";xn="<<xn<<endl;
    SharedPointer<SparseMatrix> oPmat = obsProb->getMatrix(a,xn);
    cout <<" opmat got,now prob: "<<flush;
    double oP = (*oPmat)(yn,o);
    cout << oP <<endl;


    FORs(x, momdp->XStates->size()) {
        //cout << "           x="<<x<<endl;

        //x trans mat
        SharedPointer<SparseMatrix> xTransMat = transX->getMatrix(a,x);

        FORs(y, momdp->YStates->size()) {
            //cout << "           y="<<y<<endl;

            double xP = (*xTransMat)(y,xn);

            //y trans mat
            SharedPointer<SparseMatrix> yTransMat = transY->getMatrix(a,x,xn);
            double yP = (*yTransMat)(y,yn);

            double bxy = (*b.bvec)(y);
            t_M += xP * yP * bxy;
        }
    }

    //multiply with obs prob and normalizer
    t_M *= (oP/eta_);

    return t_M;
}

//AG121009: create new belief
void AlphaVectorPolicy::ag_t_M_B(SharedPointer<MOMDP> momdp, BeliefWithState& b, int x, int a, int o, int xn, BeliefWithState* bn ) {
    cout << "< t_M Belief >"<<endl;

    bn->sval = xn;
    bn->bvec->resize(b.bvec->size());
    FOR(yn, momdp->YStates->size()) {
        //cout << "           yn="<<yn<<endl;
        REAL_VALUE v = ag_t_M(momdp, b, x,a,o,xn,yn);
        bn->bvec->push_back(yn, v);
    }
}

//AG121009: get best alpha for new belief point
SharedPointer<AlphaPlane> AlphaVectorPolicy::ag_max_alpha(SharedPointer<MOMDP> momdp, BeliefWithState& b, AlphaPlanePoolSet* alphaPlanePoolSet, int o, int a, int x, int xn) {
    BeliefWithState* bn = new BeliefWithState();

    cout << "< max_alpha >"<<endl;

    //get new belief (bn)
    ag_t_M_B(momdp, b, x, a, o, xn, bn);

    REAL_VALUE maxValue = 0;
    cout<<"[bestA x="<<bn->sval<<"; bn.y.sz="<<bn->bvec->size()<<";o="<<o<<"; a="<<a<<"; x="<<x<<"; xn="<<xn <<flush;
    cout << "; alphaplanepoolset size="<<alphaPlanePoolSet->set.size()<<";"<<flush;
    cout << "planes: " << alphaPlanePoolSet->set[bn->sval]->planes.size() <<flush;
    SharedPointer<AlphaPlane> bestAlpha = alphaPlanePoolSet->set[bn->sval]->getBestAlphaPlane1( bn->bvec, &maxValue);
    cout <<"; maxV="<<maxValue << flush;
    cout << ";action="<<bestAlpha->action<< "]"<<endl;

    return bestAlpha;
}

//AG121009: value of 1 field in the alpha vector
double AlphaVectorPolicy::ag_v_ay(SharedPointer<MOMDP> momdp, BeliefWithState& b, AlphaPlanePoolSet* alphaPlanePoolSet, int a, int x, int y) {
    Rewards* rewards = momdp->rewards;
    //probabilities
    StateTransitionX* transX = momdp->XTrans;
    StateTransitionY* transY = momdp->YTrans;
    ObservationProbabilities* obsProb = momdp->obsProb;

    cout << "< v(a,y,x) Belief >"<<endl;

    double r = (*rewards->getMatrix(x))(y,a);

    double v = 0;


    FORs(xn, momdp->XStates->size()) {
        //cout << "       xn="<<xn<<endl;

        //x trans mat
        SharedPointer<SparseMatrix> xTransMat = transX->getMatrix(a,x);
        double xP = (*xTransMat)(y,xn);

        //obs prob
        SharedPointer<SparseMatrix> oPmat = obsProb->getMatrix(a,xn);

        FORs(o, momdp->observations->size()) {            

            //cout << "       o="<<o<<endl;

            //best alpha vector
            SharedPointer<AlphaPlane> alpha = ag_max_alpha(momdp,b,alphaPlanePoolSet,o,a,x,xn);

            FORs(yn, momdp->YStates->size()) {

                //cout << "       yn="<<yn<<endl;

                //y trans mat
                SharedPointer<SparseMatrix> yTransMat = transY->getMatrix(a,x,xn);
                double yP = (*yTransMat)(y,yn);
                //o prob
                double oP = (*oPmat)(yn,o);


                double ay = (*alpha->alpha)(yn);

                v += xP * yP * oP * ay;

            }
        }
    }

    v = r + momdp->discount * v;

    return v;
}

//AG121009: value of 1 field in the alpha vector
double AlphaVectorPolicy::ag_R_ay(SharedPointer<MOMDP> momdp, BeliefWithState& b, AlphaPlanePoolSet* alphaPlanePoolSet, int a, int x) {
    double r = 0;

    cout << "< R[alpha](a,x) Belief >"<<endl;

    FORs(y, momdp->YStates->size()) {
        //cout << "       y="<<y<<endl;
        double v = ag_v_ay(momdp,b,alphaPlanePoolSet,a,x,y);
        v *= (*b.bvec)(y);

        r += v;
    }

    return r;
}

//AG121009: get values for each action
void AlphaVectorPolicy::ag_values_per_a(SharedPointer<MOMDP> momdp, BeliefWithState& b, AlphaPlanePoolSet* alphaPlanePoolSet) {
    cout << "b.size: "<<b.bvec->size()<<" x="<<b.sval<<endl;
    cout << "values per action: "<<endl;

    FORs(a,momdp->actions->size()) {
        cout << " -> action "<<a<<endl;
        double v = ag_R_ay(momdp,b,alphaPlanePoolSet,a,b.sval);
        cout << " <- action "<<a << " = " << v <<endl;
    }
}

#endif //DEBUG_AG_POL


int AlphaVectorPolicy::getBestAction(BeliefWithState& b, REAL_VALUE& maxValue, bool showActions,vector<double>* maxValPerAction) {
    //AG121010: pass vector of max val per action
    SharedPointer<AlphaPlane> bestAlpha = alphaPlanePoolSet->set[b.sval]->getBestAlphaPlane1(b.bvec, &maxValue, showActions, maxValPerAction);

    //AG120914: this is already done in AlphaPlanePool.getBestAlphaPlane1 --> therefore a param
    //maxValue = inner_prod(*bestAlpha->alpha, *b.bvec);

    //>>AG121008: use this alpha vector to calc next step -> debugging
#ifdef DEBUG_AG_POL
    //ag_values_per_a(problem, b, alphaPlanePoolSet);
#endif
    //<<AG

    //cout << "APPL0.95: MAX VALUE="<<maxValue<<" action="<<bestAlpha->action<<endl; ///AG
    return bestAlpha->action;
}

int AlphaVectorPolicy::getBestActionLookAhead(BeliefWithState& b) {
    double maxValue;
    return getBestActionLookAhead(b, maxValue);
}

int AlphaVectorPolicy::getBestActionLookAhead(BeliefWithState& b, REAL_VALUE& maxValue) {
    double maxActionLB = -DBL_MAX;
    int maxAction = 0;
    obs_prob_vector  opv; // outcome_prob_vector opv;
    SharedPointer<BeliefWithState> nextb;
    obsState_prob_vector spv;  // outcome probability for values of observed state
    SharedPointer<SparseVector> jspv (new SparseVector());

    DEBUG_TRACE(cout << "getBestActionLookAhead" << endl; );

    for(Actions::iterator aIter = problem->actions->begin(); aIter != problem->actions->end(); aIter ++) {
        int a = aIter.index();
        DEBUG_TRACE(cout << "a " << a << endl; );

        double sum = 0.0;
        double immediateReward = problem->rewards->getReward(b, a);

        if (problem->XStates->size() == 1) {
            DEBUG_TRACE(cout << "spv1 " << endl; );
            spv.resize(1);
            spv.push_back(0, 1.0);
        } else {
            DEBUG_TRACE(cout << "spv2 " << endl; );
            problem->getObsStateProbVector(spv, b, a); // P(Xn|cn.s,a)
        }

        DEBUG_TRACE( cout << "spv " << endl; );
        DEBUG_TRACE( spv.write(cout) << endl; );

        FOR(Xn, spv.size()) {
            DEBUG_TRACE(cout << "Xn " << Xn << endl; );
            double sprob = spv(Xn);
            if (sprob > OBS_IS_ZERO_EPS) {
                problem->getJointUnobsStateProbVector(*jspv, &b, a, Xn);

                DEBUG_TRACE( cout << "jspv " << endl; );
                DEBUG_TRACE( jspv->write(cout) << endl; );


                //problem->getStatenObsProbVectorFast(ospv, a, Xn, jspv);
                problem->getObsProbVectorFast(opv, a, Xn, *jspv); // only the joint prob is useful for later but we calculate the observation prob P(o|Xn,cn.s,a)
                //problem->getObsProbVector(opv, cn.s, a, Xn);

                DEBUG_TRACE( cout << "opv " << endl; );
                DEBUG_TRACE( opv.write(cout) << endl; );



                FOR(o, opv.size()) {
                    DEBUG_TRACE(cout << "o " << o << endl; );

                    double oprob = opv(o);
                    if (oprob > OBS_IS_ZERO_EPS) {
                        double obsProb = oprob * sprob; // P(o,Xn|cn.s,a) = P(Xn|cn.s,a) * P(o|Xn,cn.s,a)
                        //nextb = problem->getNextBeliefStvalFast(sp, a, o, Xn, jspv);

                        nextb = problem->beliefTransition->nextBelief2(NULL, a, o, Xn, jspv);

                        DEBUG_TRACE( cout << "nextb sval" << nextb->sval << endl; );
                        DEBUG_TRACE( nextb->bvec->write(cout) << endl; );

                        SharedPointer<AlphaPlane> bestAlpha = alphaPlanePoolSet->getBestAlphaPlane1(*nextb);
                        double childLB = inner_prod(*bestAlpha->alpha, *nextb->bvec);
                        sum += childLB * obsProb;

                        DEBUG_TRACE( cout << "childLB " << childLB << endl; );
                        DEBUG_TRACE( cout << "sum " << sum << endl; );

                        // nextb deletion will be handled by smart pointers

                    }
                }
            }
        }
        sum *= problem->getDiscount();
        sum += immediateReward;

        DEBUG_TRACE( cout << "sum " << sum << endl; );

        if(sum > maxActionLB) {
            maxActionLB = sum;
            maxAction = a;
            DEBUG_TRACE( cout << "maxAction = " << maxAction << endl; );
        }
        assert(maxActionLB !=  -DBL_MAX);
    }
    return maxAction;
}

// SYL07282010 - modify function so that it follows RSS09 paper
int AlphaVectorPolicy::getBestActionLookAhead(std::vector<belief_vector>& belYs, DenseVector& belX) {
    unsigned int bestAction  = 0;
    double bestValue, observationValue, actionValue;
    // observationValue is the summation over x'; actionValue is the summation over o; bestValue tracks the highest value among the actions, a.

    SharedPointer<BeliefWithState> nextStB (new BeliefWithState());
    SharedPointer<BeliefWithState> currStB (new BeliefWithState());

    FOR (a, problem->getNumActions()) {
        actionValue = 0;

        FOR (o, problem->observations->size()) {
            observationValue = 0;
            FOR (xn, problem->XStates->size()) {
                // for a particular x'
                if (!((problem->obsProb->getMatrix(a, xn))->isColumnEmpty(o))) {
                    SparseVector jspv_sum;  // sum of P(x',y' | x, b_{y|x}, a)  * b_x(x) over values of x
                    jspv_sum.resize(problem->YStates->size());
                    belief_vector next_belY; // for P(x',y',o | b_x, b_{y|x}, a) and P(y' | x',  b_x, b_{y|x}, a, o)

                    bool nonzero_p_xn = false;  // flag to indicate if P(x',y' | b_x, b_{y|x}, a) == 0 for this particular combination of x', o, and a, and for all y
                    // loop over x
                    FOR (xc, problem->XStates->size()) {
                        if (!(belX(xc) == 0)) {
                            // for a particular x
                            // skip all the calculations to add to jspv_sum if *(momdpProblem->XTrans->getMatrix(a, xc)) is all zero for column xn
                            if (!((problem->XTrans->getMatrix(a, xc))->isColumnEmpty(xn))) {
                                DenseVector tmp, tmp1;
                                DenseVector Bc;
                                SparseVector jspv;

                                copy(Bc, belYs[xc]);

                                emult_column( tmp, *(problem->XTrans->getMatrix(a, xc)), xn, Bc );

                                mult( tmp1, *(problem->YTrans->getMatrixTr(a, xc, xn)), tmp );
                                copy(jspv, tmp1);  // P(x',y' | x, b_{y|x}, a) for a particular x and x'

                                // multiply with belX(xc) and add to sum over x values
                                jspv *= belX(xc);
                                jspv_sum += jspv;

                                if (nonzero_p_xn == false) nonzero_p_xn = true;
                            }
                        }
                    }

                    if (nonzero_p_xn) {
                        emult_column( next_belY, *(problem->obsProb->getMatrix(a, xn)), o,  jspv_sum); // P(x',y',o | b_x, b_{y|x}, a)

                        // check that it's not an all-zero vector!!
                        if (!(next_belY.data.size() == 0)) {

                            // the normalization factor is P(x',o | b_x, b_{y|x}, a)
                            double jp_xn_and_o = next_belY.norm_1();

                            // normalize to get P(y'| x', b_x, b_{y|x}, a, o)
                            next_belY *= (1.0/next_belY.norm_1());

                            nextStB->sval = xn;

                            //nextStB->bvec = &next_belY;
                            *nextStB->bvec = next_belY;

                            //get value at (x', P(y'| x', b_x, b_{y|x}, a, o))
                            double childLB = inner_prod(*alphaPlanePoolSet->getBestAlphaPlane1(*nextStB)->alpha, *nextStB->bvec);

                            //multiply V(x', P(y'| x', b_x, b_{y|x}, a, o)) with P(x',o | b_x, b_{y|x}, a) and add to summation over x'
                            observationValue += jp_xn_and_o * childLB;
                        }
                    }
                }
                // else don't need to do anything, i.e. add 0 to observationValue
            }
            // add to actionValue
            actionValue += observationValue;
        }
        // after exiting observation loop, multiply observationValue by discount factor and add to R(b,a)
        actionValue = problem->getDiscount() * actionValue;

        FOR (xc, problem->XStates->size()) {
            if (!(belX(xc) == 0)) {
                currStB->sval = xc;

                *currStB->bvec = belYs[xc];
                //currStB->bvec = &belYs[xc];

                actionValue += (belX(xc) * problem->rewards->getReward(*currStB, a));
            }
        }

        // track best value over the actions, a.
        if (a == 0) {
            bestValue = actionValue;
        } else {
            if (actionValue > bestValue) {
                bestValue = actionValue;
                bestAction = a;
            }
        }
    }
    return bestAction;
}

// SYL07282010 - modify function so that it follows RSS09 paper
int AlphaVectorPolicy::getBestActionLookAhead(SharedPointer<belief_vector>& belY, DenseVector& belX) {
    unsigned int bestAction  = 0;
    double bestValue, observationValue, actionValue;
    // observationValue is the summation over x'; actionValue is the summation over o; bestValue tracks the highest value among the actions, a.

    SharedPointer<BeliefWithState> nextStB (new BeliefWithState());
    SharedPointer<BeliefWithState> currStB (new BeliefWithState());

    FOR (a, problem->getNumActions()) {
        actionValue = 0;

        FOR (o, problem->observations->size()) {
            observationValue = 0;
            FOR (xn, problem->XStates->size()) {
                // for a particular x'
                if (!((problem->obsProb->getMatrix(a, xn))->isColumnEmpty(o))) {
                    SparseVector jspv_sum;  // sum of P(x',y' | x, b_{y|x}, a)  * b_x(x) over values of x
                    jspv_sum.resize(problem->YStates->size());
                    belief_vector next_belY; // for P(x',y',o | b_x, b_{y|x}, a) and P(y' | x',  b_x, b_{y|x}, a, o)

                    bool nonzero_p_xn = false;  // flag to indicate if P(x',y' | b_x, b_{y|x}, a) == 0 for this particular combination of x', o, and a, and for all y
                    // loop over x
                    FOR (xc, problem->XStates->size()) {
                        if (!(belX(xc) == 0)) {
                            // for a particular x
                            // skip all the calculations to add to jspv_sum if *(momdpProblem->XTrans->getMatrix(a, xc)) is all zero for column xn
                            if (!((problem->XTrans->getMatrix(a, xc))->isColumnEmpty(xn))) {
                                DenseVector tmp, tmp1;
                                DenseVector Bc;
                                SparseVector jspv;

                                copy(Bc, *belY);

                                emult_column( tmp, *(problem->XTrans->getMatrix(a, xc)), xn, Bc );

                                mult( tmp1, *(problem->YTrans->getMatrixTr(a, xc, xn)), tmp );
                                copy(jspv, tmp1);  // P(x',y' | x, b_{y|x}, a) for a particular x and x'

                                // multiply with belX(xc) and add to sum over x values
                                jspv *= belX(xc);
                                jspv_sum += jspv;

                                if (nonzero_p_xn == false) nonzero_p_xn = true;
                            }
                        }
                    }

                    if (nonzero_p_xn) {
                        emult_column( next_belY, *(problem->obsProb->getMatrix(a, xn)), o,  jspv_sum); // P(x',y',o | b_x, b_{y|x}, a)

                        // check that it's not an all-zero vector!!
                        if (!(next_belY.data.size() == 0)) {

                            // the normalization factor is P(x',o | b_x, b_{y|x}, a)
                            double jp_xn_and_o = next_belY.norm_1();

                            // normalize to get P(y'| x', b_x, b_{y|x}, a, o)
                            next_belY *= (1.0/next_belY.norm_1());

                            nextStB->sval = xn;

                            //nextStB->bvec = &next_belY;
                            *nextStB->bvec = next_belY;

                            //get value at (x', P(y'| x', b_x, b_{y|x}, a, o))
                            double childLB = inner_prod(*alphaPlanePoolSet->getBestAlphaPlane1(*nextStB)->alpha, *nextStB->bvec);

                            //multiply V(x', P(y'| x', b_x, b_{y|x}, a, o)) with P(x',o | b_x, b_{y|x}, a) and add to summation over x'
                            observationValue += jp_xn_and_o * childLB;
                        }
                    }
                }
                // else don't need to do anything, i.e. add 0 to observationValue
            }
            // add to actionValue
            actionValue += observationValue;
        }
        // after exiting observation loop, multiply observationValue by discount factor and add to R(b,a)
        actionValue = problem->getDiscount() * actionValue;

        FOR (xc, problem->XStates->size()) {
            if (!(belX(xc) == 0)) {
                currStB->sval = xc;

                *currStB->bvec = *belY;

                actionValue += (belX(xc) * problem->rewards->getReward(*currStB, a));
            }
        }

        // track best value over the actions, a.
        if (a == 0) {
            bestValue = actionValue;
        } else {
            if (actionValue > bestValue) {
                bestValue = actionValue;
                bestAction = a;
            }
        }
    }
    return bestAction;
}

// SYL07282010 - replaced with code which follows RSS09 paper
// this function implements: for each action, for each possible (x,b_{y|x}), do one-step-lookahead, then weigh the resultant values by belX(x).
int AlphaVectorPolicy::getBestActionLookAhead_alternative(std::vector<belief_vector>& b, DenseVector& belX) {
    unsigned int bestAction  = 0;
    double bestValue, xValue, actionValue;
    //obs_prob_vector StatenObs_pv; //(problem->numObservations);
    obs_prob_vector spv, opv;

    SharedPointer<BeliefWithState> currStB (new BeliefWithState());
    SharedPointer<BeliefWithState> nextStB;


    FOR (a, problem->getNumActions()) {
        actionValue = 0;
        FOR (xc, problem->XStates->size()) {

            if (!(belX(xc) == 0)) {

                xValue = 0;
                currStB->sval = xc;
                //currStB->bvec = &b[xc];
                *currStB->bvec = b[xc];

                problem->getObsStateProbVector(spv, *currStB, a);

                FOR (xn, spv.size()) {
                    double sprob = spv(xn);
                    if (sprob > OBS_IS_ZERO_EPS) {
                        problem->getObsProbVector(opv, *currStB, a, xn);
                        //problem->getStatenObsProbVector(StatenObs_pv, currStB, a, xn);

                        //#cout << "xn : " << xn << endl;
                        FOR(o, opv.size()) {

                            double oprob = opv(o);
                            if (oprob > OBS_IS_ZERO_EPS) {
                                //problem->getNextBeliefStval(nextStB, currStB, a, o, xn);
                                nextStB = problem->beliefTransition->nextBelief(currStB, a, o, xn);
                                //get value at {xn, next belief}
                                double childLB = inner_prod(*alphaPlanePoolSet->getBestAlphaPlane1(*nextStB)->alpha, *nextStB->bvec);
                                //multiply by  StatenObs_pv(o), add to xValue
                                //xValue +=  StatenObs_pv(o) * childLB;
                                xValue +=  oprob * sprob * childLB;

                            }
                        }
                    }
                }

                xValue = problem->rewards->getReward(*currStB, a) + problem->getDiscount() * xValue;
                actionValue += (belX(xc) * xValue);
            }
        }

        if (a == 0) {
            bestValue = actionValue;
        } else {
            if (actionValue > bestValue) {
                bestValue = actionValue;
                bestAction = a;
            }

        }

    }
    return bestAction;
}

// SYL07282010 replaced with code which follows RSS09 paper
// this function implements: for each action, for each possible (x,b_y), do one-step-lookahead, then weigh the resultant values by belX(x).
int AlphaVectorPolicy::getBestActionLookAhead_alternative(SharedPointer<belief_vector>& b, DenseVector& belX) {

    unsigned int bestAction  = 0;
    double bestValue, xValue, actionValue;
    //obs_prob_vector StatenObs_pv; //(problem->numObservations);
    obs_prob_vector spv, opv;
    SharedPointer<BeliefWithState> currStB (new BeliefWithState());
    currStB->bvec = b;

    SharedPointer<BeliefWithState> nextStB;

    FOR (a, problem->getNumActions()) {
        actionValue = 0;
        FOR (xc, problem->XStates->size()) {

            if (!(belX(xc) == 0)) {

                xValue = 0;
                currStB->sval = xc;


                problem->getObsStateProbVector(spv, *currStB, a);

                FOR (xn, spv.size()) {
                    double sprob = spv(xn);
                    if (sprob > OBS_IS_ZERO_EPS) {
                        problem->getObsProbVector(opv, *currStB, a, xn);
                        //problem->getStatenObsProbVector(StatenObs_pv, currStB, a, xn);

                        //#cout << "xn : " << xn << endl;
                        FOR(o, opv.size()) {

                            double oprob = opv(o);
                            if (oprob > OBS_IS_ZERO_EPS) {
                                //problem->getNextBeliefStval(nextStB, currStB, a, o, xn);
                                nextStB = problem->beliefTransition->nextBelief(currStB, a, o, xn);
                                //get value at {xn, next belief}
                                double childLB = inner_prod(*alphaPlanePoolSet->getBestAlphaPlane1(*nextStB)->alpha, *nextStB->bvec);
                                //multiply by  StatenObs_pv(o), add to xValue
                                //xValue +=  StatenObs_pv(o) * childLB;
                                xValue +=  oprob * sprob * childLB;

                            }
                        }
                    }
                }

                xValue = problem->rewards->getReward(*currStB, a) + problem->getDiscount() * xValue;
                actionValue += (belX(xc) * xValue);
            }
        }
        if (a == 0) {
            bestValue = actionValue;
        } else {
            if (actionValue > bestValue) {
                bestValue = actionValue;
                bestAction = a;
            }

        }

    }
    return bestAction;
}


// note that this function implements: for each possible (x,b_y), lookup the action and value, weigh the value by belX(x), then choose the action corresponding to the highest weighed value
int AlphaVectorPolicy::getBestAction(SharedPointer<belief_vector>& b, DenseVector& belX) {
    int maxAction;
    double maxValue =  -DBL_MAX;

    FOR (xc, problem->XStates->size()) {
        if (!(belX(xc) == 0)) {

            SharedPointer<AlphaPlane> bestAlpha = alphaPlanePoolSet->set[xc]->getBestAlphaPlane1(b);;
            double xvalue = inner_prod(*bestAlpha->alpha, *b);
            xvalue *= belX(xc) ;
            if(xvalue > maxValue) {
                maxValue = xvalue;
                maxAction = bestAlpha->action;
            }
        }
    }
    return maxAction;
}

// SYL07282010 added
// note that this function implements: for each possible (x,b_{y|x}), lookup the action and value, weigh the value by belX(x), then choose the action corresponding to the highest weighed value
int AlphaVectorPolicy::getBestAction(std::vector<belief_vector>& belYs, DenseVector& belX) {
    int maxAction;
    double maxValue =  -DBL_MAX;

    SharedPointer<belief_vector> Bc (new belief_vector());

    FOR (xc, problem->XStates->size()) {
        if (!(belX(xc) == 0)) {
            *Bc = belYs[xc];

            SharedPointer<AlphaPlane> bestAlpha = alphaPlanePoolSet->set[xc]->getBestAlphaPlane1(Bc);
            //SharedPointer<AlphaPlane> bestAlpha = alphaPlanePoolSet->set[xc]->getBestAlphaPlane1(b);
            double xvalue = inner_prod(*bestAlpha->alpha, *Bc);
            //double xvalue = inner_prod(*bestAlpha->alpha, *b);
            xvalue *= belX(xc) ;
            if(xvalue > maxValue) {
                maxValue = xvalue;
                maxAction = bestAlpha->action;
            }
        }
    }
    return maxAction;
}

