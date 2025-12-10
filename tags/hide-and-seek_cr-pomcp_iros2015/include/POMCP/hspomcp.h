#ifndef HSPOMCP_H
#define HSPOMCP_H

#include "autoplayer.h"

#include "POMCP/mcts.h"
#include "POMCP/hsstate.h"

#include "HSGame/gmap.h"

#include "PeoplePrediction/peoplepredictionwrapper.h"



class SeekerHSParams;


namespace pomcp {

class HSSimulator;

/*!
 * The POMCP class (Partially Observable Monte-Carlo Planning) [1] uses Monte-Carlo
 * Tree Search.
 *
 * [1] Silver, D., & Veness, J. (2010). Monte-Carlo planning in large POMDPs.
 * Advances in Neural Information Processing Systems, 1–9.
 * Retrieved from http://www0.cs.ucl.ac.uk/staff/D.Silver/web/Publications_files/pomcp.pdf
 */
class HSPOMCP : public AutoPlayer
{
public:
    HSPOMCP(SeekerHSParams* params, GMap* map=NULL);

    virtual ~HSPOMCP();


    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    virtual bool initBelief2(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible, Pos seeker2InitPos, Pos hiderObs2InitPos, double obs1p);

    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone);

    virtual int getNextAction2(Pos seekerPos, Pos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p, int actionDone);

    virtual std::vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone, int n);

    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() const;

    /*!
     * \brief getGameState return whether the game is running or a player won
     * \param seekerInitPos
     * \param hiderInitPos
     * \param opponentVisible
     * \return
     */
    virtual char getGameState(Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);


    virtual bool isSeeker() const;

    virtual std::string getName() const;

    virtual bool canScoreObservations();

    virtual double scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone=1);

    virtual Pos getClosestSeekerObs(Pos seekerPos);

    virtual double getReward();

    virtual bool handles2Obs() const;

protected:
    virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

    virtual Pos getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p, std::vector<int> &actions, int actionDone, int n);

    //AG150127
    //! function only for init belief, called by initBelief and initBelief2 (which are inherrited from AutoPlayer)
    virtual bool initBelief2(GMap *gmap, HSState* seeker1InitObs, HSState* seeker2InitObs, double obs1p);

    //AG150127
    //! function only for init belief, called by initBelief and initBelief2 (which are inherrited from AutoPlayer)
    //virtual int getNextAction2(HSState* seeker1Obs, HSState* seeker2Obs, double obs1p, int actionDone);

    //AG150127
    //! function only for init belief, called by initBelief and initBelief2 (which are inherrited from AutoPlayer)
    //! if seeker2Pos and hiderObs2Pos are NULL, then getNextAction is used, otherwise getNextAction2
    virtual Pos getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos* seeker2Pos, Pos* hiderObs2Pos, double obs1p, std::vector<int> &actions, int actionDone, int n);


    /*!
     * \brief updatePeoplePrediction update people prediction through wrapper (if used)
     * \param hiderPos
     */
    virtual void updatePeoplePrediction(const Pos& hiderPos);

    //! MC tree search
    MCTS* _mcts;

    //! simulator
    HSSimulator* _simulator;

    //! last action
    int _action;

    //AG140908
    //! person path predictor consumer
    PersonPathPredConsumer* _personPathPredConsumer;

    //! People Prediction wrapper, used to simulate movemente of person
    //! if no hider location then the expected is used (?), and if no observation so far, nothing is passed
    PeoplePredictionWrapper* _ppWrapper;
};

}
#endif // HSPOMCP_H
