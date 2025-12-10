#ifndef __ACTION_H
#define __ACTION_H

#include "Model.h"

enum actType{Initial, Macro, Act, None}; // types of actions

class MAction
{
  public:
    // actNum is for internal use (arrange as InitActs, MacroActs,
    // NormalActs), no need for type.

    // actNumUser is for the user (type + index of the action with
    // this type)
    long actNum, actNumUser;
    actType type;
    static Model* model;

    explicit MAction(long actNum);
    MAction(actType type, long actNumUser);
    virtual ~MAction() {}

    bool operator==(const MAction& action) const;

    static int compare(const MAction& a, const MAction& b);

    /**
       Set actNum, compute the type and actNumUser
       @param [in] actNum
    */
    void setActNum(long actNum);

    /**
       Set type, actNumUser, compute actNum
       @param [in] type
       @param [in] actNumUser
    */
    void setActNumUser(actType type, long actNumUser);

    /**
       Get the type of action, computed using actNum
    */
    actType getActType();

    /**
       Get actNumUser
    */
    long getActNumUser() const;

    /**
       Compute the type based on actNum
    */
    void computeType();

    /**
       Given the type \a aType and actNumUser number \a aNum, return
       the actNum.
       @param [in] model
       @param [in] aType : the type
       @param [in] aNum  : actNumUser number
       @return actNum
    */
    static long getBeliefAct(Model& model, actType aType, long aNum);

    /**
       Set the model for querying number of InitActs, MacroActs
       @param [in] model
    */
    static void initStatic(Model* model);
};

#endif
