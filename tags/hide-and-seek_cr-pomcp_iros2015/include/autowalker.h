#ifndef AUTOWALKER_H
#define AUTOWALKER_H

#include <vector>

#include "HSGame/gmap.h"
#include "seekerhsparams.h"
#include "HSGame/idpos.h"

/*!
 * \brief The AutoWalker class an autowalker or walkers that generates new positions of emulated walking people or dynamic obstacles.
 */
class AutoWalker {
public:
    /*!
     * \brief AutoWalker initializes n auto walkers
     * The vector should be filled by the subclass.
     * \param n
     */
    AutoWalker(std::size_t n);

    virtual ~AutoWalker();


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
     * \brief getAllNextPos get all next positions of the automated walkers
     * \param seekerPos
     * \param hiderPos
     * \return
     */
    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos)=0;

    /*!
     * \brief getName get name
     * \return
     */
    virtual std::string getName() const=0;

    /*!
     * \brief setMap set the map
     * \param map
     */
    virtual void setMap(GMap* map)=0;

    /*!
     * \brief getMap get the used map
     * \return
     */
    virtual GMap* getMap() const=0;

    /*!
     * \brief getParams get params
     * \return
     */
    virtual SeekerHSParams* getParams() const=0;

protected:
    /*!
     * \brief checkMovement check if persI can be moved to the new position, the auto walker will be checked.
     * Note that the vector can contain positions of time t and t+1.
     * TODO: maybe take into account a list of positions of t, and  t+1, but we have to avoid that they collide.
     * \param persI
     * \param newPos
     * \return
     */
    bool checkMovement(std::size_t persI, Pos& newPos, Pos& seekerPos, Pos& hiderPos);

    //! auto walkers positions
    std::vector<IDPos> _autoWalkerVec;

};

#endif // AUTOWALKER_H
