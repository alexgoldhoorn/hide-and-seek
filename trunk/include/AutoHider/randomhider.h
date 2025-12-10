#ifndef RANDOMHIDER_H
#define RANDOMHIDER_H

#include <vector>

#include "AutoHider/autohider.h"


#include "autowalker.h"

/*!
 * \brief The RandomHider class Initializes at a random hidden position (if available), then choses actions randomly.
 */
class RandomHider : public AutoWalker
{
public:
    /*!
     * \brief RandomHider
     * \param params
     * \param n number of auto walkers, default 0, and only 1 'hider'
     */
    RandomHider(SeekerHSParams* params, std::size_t n = 0);

    virtual ~RandomHider();

    virtual std::string getName() const;

    virtual int getHiderType() const;

    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos);

protected:

    virtual bool initBeliefRun();

    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);

};

#endif // RANDOMHIDER_H
