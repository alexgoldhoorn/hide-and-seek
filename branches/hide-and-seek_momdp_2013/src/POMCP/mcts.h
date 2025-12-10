#ifndef MCTS_H
#define MCTS_H

#include "node.h"
#include "simulator.h"
#include "state.h"

class SeekerHSParams;


namespace pomcp {

/*!
 * \brief The MCTS class The Monte Carlo tree Search algorithm
 */
class MCTS
{
public:
    MCTS(SeekerHSParams* params, Simulator* simulator);

    ~MCTS();

    /*!
     * \brief init init which should be done once!
     */
    void init();

    /*!
     * \brief getRoot get root
     * \return
     */
    Node* getRoot();

    /*!
     * \brief ExpandNode
     * \param node to expand
     * \param state
     * \return
     */
    Node* expandNode(Node* node, State* state);

    /*!
     * \brief update
     * \param action
     * \param obs
     * \param reward
     * \return
     */
    bool update(int action, int obs, double reward);

    /*!
     * \brief uctSearch UCT search, generates/grows tree for current root
     */
    void uctSearch();

    /*!
     * \brief greedyUCB get best action according to given tree
     * \param node
     * \param ucb
     * \return best action
     */
    int greedyUCB(Node* node, bool ucb);

    /*!
     * \brief rollout calculating the estimated reward
     * \param state
     * \return rollout reward
     */
    double rollout(State* state);

    /*!
     * \brief selectAction select the action (using UCT search by default)
     * \return
     */
    int selectAction();


protected:
    /*!
     * \brief simulateNode simulate for a node
     * \param node
     * \param state
     * \return
     */
    double simulateNode(Node* node, State* state);

    /*!
     * \brief simulateNodeA simulate for a node action
     * \param nodea
     * \param state
     * \param action
     * \return
     */
    double simulateNodeA(NodeA* nodea, State* state, int action);


    //! the root of the tree
    Node* _root;

    //! the simulator
    Simulator* _simulator;

    //! the history
    History _history;

    //! params
    SeekerHSParams* _params;

    //! tree depth
    unsigned int _treeDepth;
};

}

#endif // MCTS_H
