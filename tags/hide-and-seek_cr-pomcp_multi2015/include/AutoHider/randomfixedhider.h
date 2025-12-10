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

    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1);

    //virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);

    virtual int getHiderType() const;

    virtual std::string getName() const;

    //virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);

    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos);


protected:
    //! current pos for this walker
    //Pos _curPos;

    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);
};


#endif // RANDOMFIXEDHIDER_H
