#ifndef FOLLOWER_H
#define FOLLOWER_H

#include "autoplayer.h"


/*!
 * \brief The Follower class follows the hider when (s)he is visible, as action it gives a direction or 'halt'.
 *  If the hider is visible, the direction of the hider is given as action, otherwise 'halt'.
 */
class Follower : public AutoPlayer
{
public:
    /*!
     * \brief Follower constructor
     * \param params
     */
    Follower(SeekerHSParams* params);

    virtual ~Follower();


    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    virtual double getNextDirection(Pos seekerPos, Pos hiderPos, bool opponentVisible, bool &haltAction);

    virtual bool isSeeker() const;

    virtual std::string getName() const;

    virtual bool useGetAction();

protected:
    virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

    //! hider last pos
    Pos _lastPos;

//private:

};
#endif // FOLLOWER_H
