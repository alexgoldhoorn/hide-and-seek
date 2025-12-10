#ifndef __HIDESEEKMODEL_H
#define __HIDESEEKMODEL_H

#include "mcvi/Model.h"

//ag130226: map
#include "HSGame/gmap.h"
#include "HSGame/gplayer.h"


class ParticlesBelief;

/*!
 * HideSeekModel uses the MCVI [1],[2]
 *
 * The State is an array of 4 variables:
 *  1. x seeker
 *  2. y seeker
 *  3. x hider
 *  4. y hider
 *
 *
 * [1] http://bigbird.comp.nus.edu.sg/pmwiki/farm/appl/index.php?n=Main.DownloadMcvi
 * [2] Monte Carlo Value Iteration for continuous-state POMDPs.
 *     Proc. Int. Workshop on the Algorithmic Foundations of Robotics (WAFR) (2011), pp. 175-191.
 *
 * \brief The HideSeekModel class
 */
class HideSeekModel : public Model
{
  public:
    //ag130226: map
    HideSeekModel(GMap* map);

    double sample(const MState& currState, const MAction& action, MState& nextState, MObs& obs, RandStream& randStream);

    double sample(const MState& currState, const MAction& macroAction, long controllerState, MState& nextState, long& nextControllerState, MObs& obs, RandStream& randStream);

    double initPolicy(const MState& currState, const MAction& initAction, long controllerState, MState& nextState, long& nextControllerState, MObs& obs, RandStream& randStream);

    //AG130304: disabled
    MState sampleInitState();
    ParticlesBelief* getInitBelief(int numStates);

    double getObsProb(const MAction& action, const MState& state, const MObs& obs);

    double upperBound(const MState& state);

    double getMaxReward() { return REWARD_WIN; }
    double getMinReward() { return REWARD_LOSE; }

    bool allowableAct(const Belief& belief, const MAction& action);

    inline obsType getObsType(const MObs& obs) { return OtherObs; }

    inline void setObsType(MObs& obs, obsType type) {}

    inline bool isTermState(const MState& state) {
        //return (static_cast<long>(state[0]) == TermState);
        //terminal state if:
        //          hider_pos == seeker_pos
        //AG130226: changed, not sure if needs to be inline and maybe in cpp file
        return ( state[0]==state[2] && state[1]==state[3]) || ( state[2]==_base.row && state[3]==_base.col );
    }

  private:
    //! GMap
    GMap* _gmap;
    Pos _base;
    Player _seekerPlayer, _hiderPlayer;

    //long

    static const double Discount = 0.95;

    /*const int NumDoors = 4;
    const double Noise = 0.01;
    const double EnterReward = 10;
    const double WrongPenalty = -2;*/

    static const double REWARD_WIN = 1;
    static const double REWARD_LOSE = -1;
    static const double REWARD_TIE = -0.5;
    static const double REWARD_MOVEMENT = -0.05;
    //TODO: add time in state (or maybe is due to movementcost)



    static const long NUM_STATE_VARS = 4; //2;   //hider+seeker (x,y)
    static const long NUM_OBS_VARS = 2; //1;     //hider (x,y) or -1,-1

    // The 2nd door from the left is the correct door
    enum actions {actHalt=0, actN,actNE,actE,actSE,actS,actSW,actW,actNW};

    //enum observations {ObsNothing, ObsWrongDoor, ObsLeftEnd, ObsRightEnd};

    static const long NUM_ACTS = 9; //3;    // nine actions

    //const long TermState = -1;

    static const long NUM_MACRO_ACTS = 0;
    static const long NUM_INIT_POLICIES = 1;
};

#endif
