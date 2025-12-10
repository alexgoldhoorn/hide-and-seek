#ifndef AlphaVectorPolicy_H
#define AlphaVectorPolicy_H

#include <vector>
#include <string>
#include "MOMDP.h"
#include "MObject.h"
using namespace std;
using namespace momdp;

namespace momdp {
class AlphaPlanePoolSet;
#ifdef DEBUG_AG_POL //AG121009: for check of actions
class AlphaPlane;
#endif

class AlphaVectorPolicy : public MObject {
private:
    SharedPointer<MOMDP> problem;
    AlphaPlanePoolSet* alphaPlanePoolSet;
    string policyFile;
public:
    AlphaVectorPolicy(SharedPointer<MOMDP> problem);
    int getBestActionLookAhead(BeliefWithState& b);
    int getBestActionLookAhead(BeliefWithState& b, REAL_VALUE& maxValue);
    int getBestAction(BeliefWithState& b);

    //AG121010: pass vector with max val per action
    int getBestAction(BeliefWithState& b, REAL_VALUE& maxValue, bool showActions = false, vector<double>* maxValPerAction = NULL);

    int getBestActionLookAhead(vector<belief_vector>& b, DenseVector& belX); // SYL07282010 - modify function so that it follows RSS09 paper. Input to function is b_x and b_{y|x}.
    int getBestActionLookAhead(SharedPointer<belief_vector>& b, DenseVector& belX); // SYL07282010 - modify function so that it follows RSS09 paper. Input to function is b_x and b_y.

    int getBestAction(SharedPointer<belief_vector>& b, DenseVector& belX);  // Input to function is b_x and b_y.
    int getBestAction(vector<belief_vector>& b, DenseVector& belX);  // SYL07282010 - added the counterpart to int getBestAction(SharedPointer<belief_vector>& b, DenseVector& belX). Input to function is b_x and b_{y|x}.

    int getBestActionLookAhead_alternative(vector<belief_vector>& b, DenseVector& belX);  // SYL07282010 replaced with code which follows RSS09 paper. Input to function is b_x and b_{y|x}.
    int getBestActionLookAhead_alternative(SharedPointer<belief_vector>& b, DenseVector& belX);  // SYL07282010 replaced with code which follows RSS09 paper. Input to function is b_x and b_y.


    bool readFromFile(const std::string& inFileName);

private:

#ifdef DEBUG_AG_POL  //AG121009: for check of actions


    //AG121008
    // p(o|b,a) = sum_s' [ p(o|s',a) ( sum_s(p(s'|s,a) b(s)) ) ] = 1/eta
    double ag_p_o_ba(SharedPointer<MOMDP> momdp, BeliefWithState& b, int o, int a );


    //AG121009
    // t_M(x,b_y,a,o,x') = eta * p(o|x',y',a)* sum_y( p(x'|x,y,a) p(y'|x,y,x',a) b(x,y)
    double ag_t_M(SharedPointer<MOMDP> momdp, BeliefWithState& b, int x, int a, int o, int xn, int yn );

    //AG121009: create new belief
    void ag_t_M_B(SharedPointer<MOMDP> momdp, BeliefWithState& b, int x, int a, int o, int xn, BeliefWithState* bn );

    //AG121009: get best alpha for new belief point
    SharedPointer<AlphaPlane> ag_max_alpha(SharedPointer<MOMDP> momdp, BeliefWithState& b, AlphaPlanePoolSet* alphaPlanePoolSet, int o, int a, int x, int xn);

    //AG121009: value of 1 field in the alpha vector
    double ag_v_ay(SharedPointer<MOMDP> momdp, BeliefWithState& b, AlphaPlanePoolSet* alphaPlanePoolSet, int a, int x, int y);

    //AG121009: value of 1 field in the alpha vector
    double ag_R_ay(SharedPointer<MOMDP> momdp, BeliefWithState& b, AlphaPlanePoolSet* alphaPlanePoolSet, int a, int x);

    //AG121009: get values for each action
    void ag_values_per_a(SharedPointer<MOMDP> momdp, BeliefWithState& b, AlphaPlanePoolSet* alphaPlanePoolSet);

#endif //DEBUG_AG_POL
};
}

#endif

