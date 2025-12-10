#ifndef HSMOMDP_RUNPOLICY_H
#define HSMOMDP_RUNPOLICY_H


#include "hsmomdp.h"
#include "../Segment/segment.h"

#include "AlphaVectorPolicy.h"

#include <vector>


/*! Run Policy  HSMOMDP
  Uses an (offline) learned policy to play the game.

  Note: _momdp is used as Bottom MOMDP!
  */
class RunPolicyHSMOMDP : public HSMOMDP {
public:
    RunPolicyHSMOMDP();
    ~RunPolicyHSMOMDP();

    //! Init with the POMDP file, policy file, and log files
    bool init(const char* pomdpFile, const char* logOutFile, const char* policyFile, const char* timeLog = NULL);

    //! Get best action to do at current belief state.
    virtual int getBestAction();

    virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);

protected:
    //! get next best action to do, based on the Y state (hider pos), X state (seeker pos) and visibility of the hider
    virtual int getNextAction(int observ, int x_state);    


};


#endif // HSMOMDP_RUNPOLICY_H

