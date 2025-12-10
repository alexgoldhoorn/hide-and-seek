#ifndef RANDOMFIXEDHIDER_H
#define RANDOMFIXEDHIDER_H


#include <vector>

#include "AutoHider/randomhider.h"

/*!
 * \brief The RandomFixedHider class starts at a random point and stays there.
 */
class RandomFixedHider : public RandomHider {
public:
    RandomFixedHider(SeekerHSParams* params, std::size_t n = 0);

    virtual ~RandomFixedHider();

    virtual int getHiderType() const;

    virtual std::string getName() const;

    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos);


protected:
    //virtual bool initBeliefRun();

    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);
};


#endif // RANDOMFIXEDHIDER_H
