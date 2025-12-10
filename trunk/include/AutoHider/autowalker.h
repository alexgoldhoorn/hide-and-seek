#ifndef AUTOWALKER_H
#define AUTOWALKER_H

#include <vector>

#include "AutoHider/autohider.h"

/*!
 * \brief The AutoWalker class an autowalker or walkers that generates new positions of emulated walking people or dynamic obstacles.
 */
class AutoWalker : public AutoHider {
public:
    //! maximum tries to find start position
    static const unsigned int MAX_TRIES_FIND_START_POS = 10000;

    /*!
     * \brief AutoWalker initializes n auto walkers
     * The vector should be filled by the subclass.
     * \param params
     * \param n
     */
    AutoWalker(SeekerHSParams* params, std::size_t n);

    virtual ~AutoWalker();

    /*!
     * \brief getAllNextPos get all next positions of the automated walkers
     * \param seekerPos
     * \param hiderPos
     * \return
     */
    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos)=0;

    //AG160129: TODO should improve
    //! sets start position,
    void setStartPosRelativeTo(const std::vector<IDPos>& relativePoses, bool startVisib, double dist=-1.0);

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

    virtual bool checkSumObsProbIs1();

    virtual bool initBeliefRun();

    //! auto walkers positions
    std::vector<IDPos> _autoWalkerVec;

    //! start position, can be set in setStartPosRelativeTo
    Pos _startPos;

};

#endif // AUTOWALKER_H
