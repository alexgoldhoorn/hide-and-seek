#ifndef RANDOMLISTHIDER_H
#define RANDOMLISTHIDER_H

#include "randomhider.h"
#include <vector>

class RandomListHider : public RandomHider
{
public:
    RandomListHider(string actionFileName);

    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible);

    virtual Pos getInitPos();

    string getActionListName();

    virtual std::string getName() {
        return _name;
    }

    virtual int getHiderType() {
        return HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST;
    }

protected:

    void readActionFile(string actionFileName);

    int getNextActionFromList();


    vector<int> _recActionList;
    string _recActionListName;
    int _recordActionI; //index of recorded action
    Pos _initActionListPos; //AG120911: init pos from action list

    string _name;

};

#endif // RANDOMLISTHIDER_H
