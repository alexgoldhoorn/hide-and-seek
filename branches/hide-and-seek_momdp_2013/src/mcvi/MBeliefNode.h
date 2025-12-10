#ifndef __BELIEFNODE_H
#define __BELIEFNODE_H

#include <vector>
#include "MAction.h"
#include "PolicyGraph.h"

class MObs;
class ActNode;

/**
   @struct BeliefNode
   @brief Interface for a belief node in the belief tree.
   @details Used as node in the belief tree. Hold the current bounds,
   lastUpdated, links to actNode.
*/

struct BeliefNode
{
    /**
       Initialize the last observation
       @param[in] obs Last observation that generated this belief.
    */
    BeliefNode(const MObs& obs):
            obs(obs),
            lBound(NegInf), uBound(Inf),
            bestLBoundAct(-1), bestUBoundAct(-1),
            bestPolicyNode(NULL),
            lastUpdated(Never)
    {}

    virtual ~BeliefNode() {}

    static void initStatic(Model* model)
    {
        BeliefNode::model = model;
    }

    // The last observation that generated this belief, needed to get
    // the group info
    const MObs obs;

    // The current bounds
    double lBound, uBound;

    MAction bestLBoundAct; // action that provided the lower bound
    MAction bestUBoundAct; // action that provided the upper bound

    // Best policy graph node whose index is up to lastUpdated
    PolicyGraph::Node* bestPolicyNode;

    // Index of the last policy graph node we ran simulation from
    long lastUpdated;

    // All action nodes, ordered by init actions, macro actions and
    // normal action
    std::vector<ActNode*> actNodes;

    static Model* model; // model information
};
#endif
