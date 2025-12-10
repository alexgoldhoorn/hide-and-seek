#ifndef COMBINEDSEEKER_H
#define COMBINEDSEEKER_H

#include "autoplayer.h"

/*!
 * \brief The CombinedSeeker class this class uses two kinds of seekers, one in cases when the hider is visible
 *  (can be a simple, heuristic one), and one when the hider is hidden. The second one is always called (to update its internal beliefs),
 *  and it is used to get the belief.
 */
class CombinedSeeker : public AutoPlayer
{
public:
    /*!
     * \brief CombinedSeeker constructor for CombinedSeeker, auto player used for when hider is visible, and for when hider not visible
     * \param params
     * \param autoPlayerForVisib
     * \param autoPlayerForNotVisib
     */
    CombinedSeeker(SeekerHSParams* params, AutoPlayer* autoPlayerForVisib, AutoPlayer* autoPlayerForNotVisib);

    virtual ~CombinedSeeker();


    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1);

    virtual std::vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1, int n=1);


    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() const;

    virtual bool isSeeker() const;

    virtual std::string getName() const;

    virtual bool canScoreObservations();

    virtual double scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone=-1);

    virtual Pos getClosestSeekerObs(Pos seekerPos);

protected:
    virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);


    //! auto player used when hider visible
    AutoPlayer* _autoPlayerForVisib;
    //! auto player used when hider not visible
    AutoPlayer* _autoPlayerForNotVisib;

    int _action;

};


#endif // COMBINEDSEEKER_H
