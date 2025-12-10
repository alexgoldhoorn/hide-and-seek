#ifndef RANDOMLISTHIDER_H
#define RANDOMLISTHIDER_H

#include "AutoHider/randomhider.h"
#include <vector>

/*!
 * \brief The RandomListHider class reads the initial position from a file, and from then all the actions.
 *  Note that one action file is written for a specific map and base position.
 */
class RandomListHider : public RandomHider
{
public:
    RandomListHider(SeekerHSParams* params, std::string actionFileName);

    virtual ~RandomListHider();

    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1);

    virtual Pos getInitPos();

    std::string getActionListName() const;

    virtual std::string getName() const;

    virtual int getHiderType() const;

protected:

    void readActionFile(std::string actionFileName);

    int getNextActionFromList();



    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);


    std::vector<int> _recActionList;
    std::string _recActionListName;
    int _recordActionI; //index of recorded action
    Pos _initActionListPos; //AG120911: init pos from action list

    std::string _name;

};

#endif // RANDOMLISTHIDER_H
