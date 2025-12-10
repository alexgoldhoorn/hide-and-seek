#ifndef HIDESEEKMCVI_H
#define HIDESEEKMCVI_H

#include <string>

#include "autoplayer.h"
#include "HSGame/gmap.h"

/*#include "HideSeekModel.h"
#include "RandSource.h"*/
#include "PolicyGraph.h"
/*#include "MAction.h"
#include "MObs.h"
*/
class MObs;
class MAction;
/*class PolicyGraph {
public:
    class Node;
};*/
class RandSource;
class HideSeekModel;
//class MState;
#include "MState.h"


class HideSeekMCVI : public AutoPlayer {
public:
    HideSeekMCVI(std::string policyFile);

    ~HideSeekMCVI();

    /*! Init belief with the GMap it assumes that the gmap alread contains: initial positions of seeker and hider, and the visibility map of the seeker.
     * \brief initBelief
     * \param gmap
     * \param hiderPos
     * \param seekerPos
     * \return
     */
    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible);


    /*!
     * Get next best action to do, based on the hider pos, seeker pos and visibility of the hider
     * \brief getNextAction
     * \param visible is the hider visible to the seeker
     * \param hiderPos hider position
     * \param seekerPos seeker position
     * \return
     */
    virtual int getNextAction(Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible);


private:
    //! map
    GMap* _map;
    //! MCVI model
    HideSeekModel* _model;
    //! the policy
    PolicyGraph* _policy;
    //! random source
    RandSource* _randSource;
    //! policy file
    std::string _policyFile;
    //! state
    MState _currState;
    //! obs
    MObs* _obs;
    //! policy graph node
    PolicyGraph::Node *_currGraphNode;
    //! sum reward
    double _sumReward;
    double _currDiscount;
    double _sumDiscounted;

    MAction* _action;
};

#endif // HIDESEEKMCVI_H
