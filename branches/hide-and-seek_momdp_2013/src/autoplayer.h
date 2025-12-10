#ifndef AUTOPLAYER_H
#define AUTOPLAYER_H

#include "HSGame/gmap.h"

/*!
 * This interface is required for automated seekers. Used by GameConnector to get the seeker's actions.
 *
 * \brief The AutoPlayer class
 */
class AutoPlayer {
public:


    //AG130320: init hider and seeker pos
    /*! Init belief with the GMap it assumes that the gmap alread contains: initial positions of seeker and hider, and the visibility map of the seeker.
     * \brief initBelief
     * \param gmap
     * \param hiderPos
     * \param seekerPos
     * \return
     */
    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible)=0;


    /*!
     * Get next best action to do, based on the hider pos, seeker pos and visibility of the hider
     * \brief getNextAction
     * \param visible is the hider visible to the seeker
     * \param hiderPos hider position
     * \param seekerPos seeker position
     * \return
     */
    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible)=0;

    //AG130729
    /*!
     * Get the next multiple actions. Note that the next actions might not be the most optimal, since
     * the position of the hider is not known yet.
     * \brief getNextMultipleActions
     * \param seekerPos
     * \param hiderPos
     * \param opponentVisible
     * \param n number of actions required
     * \return list of next n actions
     */
    virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n)=0;


    //virtual vector<int> getNextMultipleActions(Pos seekerPos, vector<Pos> seekerPos, int n)=0;


    /*!
     * \brief getBelief get the belief of a hider's state
     * \param r
     * \param c
     * \return
     */
    virtual double getBelief(int r, int c)=0;

    /*!
     * \brief getBelief get the belief of a hider's state
     * \param pos
     * \return
     */
    double getBelief(Pos pos) {
        return getBelief(pos.row,pos.col);
    }


    /*!
     * \brief tracksBelief does this method track a belief (if false-> getBelief should NOT be used)
     * \return
     */
    virtual bool tracksBelief()=0;

};


#endif // AUTOPLAYER_H
