#include <cstdio>
#include <cstdlib>
#include "MAction.h"
using namespace std;

Model* MAction::model;

void MAction::initStatic(Model* model)
{
    MAction::model = model;
}

MAction::MAction(long actNum): actNum(actNum)
{
    computeType();
}

MAction::MAction(actType type, long actNumUser)
{
    actNum = MAction::getBeliefAct(*model,type,actNumUser);
    // be consistent, don't belief the user
    computeType();
}

bool MAction::operator==(const MAction& action) const
{
    if (action.actNum != actNum) return false;
    return true;
}

int MAction::compare(const MAction& a, const MAction& b)
{
    if (a.type < b.type) return 1;
    else if (a.type > b.type) return -1;

    if (a.actNum < b.actNum) return 1;
    else if (a.actNum > b.actNum) return -1;

    return 0;
}

void MAction::setActNum(long actNum)
{
    this->actNum = actNum;

    computeType();
}

void MAction::setActNumUser(actType type, long actNumUser)
{
    this->type = type;
    this->actNumUser = actNumUser;
    actNum = MAction::getBeliefAct(*model,type,actNumUser);
}

actType MAction::getActType()
{
    if (type != None)
        return type;

    computeType();
    return type;
}

long MAction::getActNumUser() const {
    return actNumUser;
}

void MAction::computeType()
{
    if (model == NULL) {
        cerr<<"Action::model has not been initialized\n";
        exit(1);
    }

    actNumUser = actNum;
    if (actNum - model->getNumInitPolicies() - model->getNumMacroActs() >=0) {
        type = Act; // simple action
        actNumUser -= (model->getNumInitPolicies() + model->getNumMacroActs());
    }
    else if (actNum - model->getNumInitPolicies() >= 0) {
        type = Macro; // macro action
        actNumUser -= model->getNumInitPolicies();
    }
    else type = Initial; // initial action
}

long MAction::getBeliefAct(Model& model, actType aType, long aNum)
{
    if (aType == Act)
        return aNum + model.getNumInitPolicies() + model.getNumMacroActs();
    else if (aType == Macro)
        return aNum + model.getNumInitPolicies();
    else return aNum;
}
