#ifndef HSMOMDP_H
#define HSMOMDP_H

/*

Author:         Alex Goldhoorn
Created:        20/3/2013
Last change:    idem
  */
///#include <QList>

#include "Base/hsconfig.h"
#include "Utils/hslog.h"
#include "Utils/timer.h"
#include "HSGame/gmap.h"
#include "hssolver.h"
#include "Base/hsglobaldata.h"
#include "Base/autoplayer.h"

#include "MOMDP.h"
#include "solverUtils.h"


#define EPSILON 0.01


/*! Hide&Seek MOMDP, abstract class to implement the problem.
 * Uses the SARSOP solver [1],[2] and the MOMDP model [3].
 *
 *
 * [1] http://bigbird.comp.nus.edu.sg/pmwiki/farm/appl/index.php?n=Main.Download
 * [2] H. Kurniawati, D. Hsu, and W.S. Lee. SARSOP: Efficient point-based POMDP planning by
 *     approximating optimally reachable belief spaces. In Proc. Robotics: Science and Systems, 2008.
 * [3] S.C.W. Ong, S.W. Png, D. Hsu, and W.S. Lee. POMDPs for robotic tasks with mixed observability.
 *     In Proc. Robotics: Science and Systems, 2009.
 *
  */
class HSMOMDP : public AutoPlayer {
public:
    //! show problem (MOMDP), showTransition-> shows all transition probabilities
    static void showMOMDP(SharedPointer<MOMDP> momdp, bool showTransition=true);

    //! Generate a HSMOMDP solver
    HSMOMDP(SeekerHSParams* params);

    virtual ~HSMOMDP();

    /*! Init belief with the GMap
    it assumes that the gmap alread contains: initial positions of seeker and hider, and the visibility map of the seeker
    */ //ag130320: added hider,seeker pos, hider visible
    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible);

    //ag130320: removed y_state -> only obs
    /*! Update belief
      */
    virtual bool updateBelief(int observ, int x_state);


    //ag130320: y_state -> y_obs (if !visible -> not set)
    /*! Update belief
      */
    virtual bool updateBelief(Pos seekerPos, Pos hiderPos, bool visible);

    //ag130320: removed y_state -> only obs (if !visible -> not set)
    /*! Get next best action to do, based on the Y observ (hider pos), X state (seeker pos) and visibility of the hider
    note: for hierarchical can be 1-5 */
    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool visible, int actionDone);


    //virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);


    /*! Get best action in current belief state.
      */
    virtual int getBestAction()=0;

    /*
     * State index to position on the map.
     *
     * \brief stateToPos State index to position on the map
     * \param state
     * \return
     */ //ag130304: use gmap.getIndexFromCoord
    //Pos stateToPos(int state);

    //AG130411: get belief
    /*!
     * \brief getBelief get the current belief for the y state
     * \param y_state
     * \return
     */
    double getBelief(int y_state);

    /*!
     * \brief getBelief get the current belief for hider pos
     * \param r,c
     * \return
     */
    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() const;

    //virtual Pos getInitPos();

    virtual bool isSeeker() const;

    //virtual std::string getName();


protected:
    //! Init with the POMDP file and log files
    virtual bool initBase(const char* pomdpFile, const char* logOutFile, const char *timeLog);


    //ag130320: removed y_state -> only obs
    /*! get next best action to take, based on state and observation index; this is the abstract version */
    virtual int getNextAction(int observ, int x_state)=0;


    //! init the log
    void initHSLog(const char* logFile);

    //! timer
    Timer _timer;

    //! log class
    HSLog* _hslog;

    //! log stream for APPL
    ofstream* _applLogStream;

    //APPL class that analyizes program params and keeps vars
    //will be used for bottom AND top, since only certain params stored here
    //! Parameters stored for APPL solver
    SolverParams* _solverParams;

    //! HS Solver
    HSSolver _solver;

    //! MOMDP
    SharedPointer<MOMDP> _momdp;

    //! current belief
    SharedPointer<BeliefWithState> _currBelief;

    //! when opponent not visible observation index
    int _unobservedObs;

    //! action chosen (used by updatebelief)
    int _action;
    //! current x state and observation
    int _x_state, _observ;
    //130320: y_state removed, already in obs

    //! time iterator
    int _timeIter;

    // pre calculated list of states and the position on the map
    //QList<Pos> _stateToPosList;   //ag130304: disabled

private:
    // init state position index
    //void initStateIndexPos(); //ag130304: use gmap.getIndexFromCoord
};


#endif // HSMOMDP_H

