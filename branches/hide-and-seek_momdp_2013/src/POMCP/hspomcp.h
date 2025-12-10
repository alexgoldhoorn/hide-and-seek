#ifndef HSPOMCP_H
#define HSPOMCP_H

#include "autoplayer.h"

#include "mcts.h"

#include "HSGame/gmap.h"


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

    ~HSPOMCP();


    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);


    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible);

    virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);

    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible);




    //TODO: maybe later can do a check of belief
    virtual double getBelief(int r, int c) {
        return 0;
    }

    virtual bool tracksBelief() {
        return false;
    }

    /*!
     * \brief getGameState return whether the game is running or a player won
     * \param seekerInitPos
     * \param hiderInitPos
     * \param opponentVisible
     * \return
     */
    virtual char getGameState(Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);



protected:
    //! MC tree search
    MCTS* _mcts;

    //! params
    SeekerHSParams* _params;

    //! simulator
    HSSimulator* _simulator;

    //! map
    GMap* _map;

    //! last action
    int _action;
};

}
#endif // HSPOMCP_H
