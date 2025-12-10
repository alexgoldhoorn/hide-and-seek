#ifndef MCTS_H
#define MCTS_H

#include <vector>

#include "POMCP/node.h"
#include "POMCP/simulator.h"
#include "POMCP/state.h"
#include "POMCP/observation.h"

#ifdef GUI_DEBUG
#include <QObject>
#include "POMCP/treewindow.h"
#endif

#include "PeoplePrediction/personpathpredconsumer.h"

//#include "test/include/POMCP/mcts-test.h"


class SeekerHSParams;

namespace pomcp {

/*!
 * \brief The MCTS class The Monte Carlo tree Search algorithm
 */
#ifdef GUI_DEBUG
class MCTS : public QObject {    

    Q_OBJECT

signals:
    void actionChosen(pomcp::Node* root, int a);

#else
class MCTS {
#endif
    //friend class MCSTest;

public:
    MCTS(SeekerHSParams* params, Simulator* simulator, PersonPathPredConsumer* personPathPredConsumer);

    virtual ~MCTS();

    //AG150122: added obs2
    /*!
     * \brief init initializes the belief (should be done once)
     * \param initObs observation (of this seeker)
     * \param initObs2 (optional) observation of other seeker (if available)
     *
     */
    //void init(const State* initObs, const State* initObs2=NULL, double obs1p=-1);

    //AG150611: update to multi
    /*!
     * \brief init initializes the belief (should be done once)
     * Complexity: O_Simulator::getAllInitStates -> O(n)
     * \param obs
     */
    void init(const Observation* obs); // const State* thisSeekerInitObs, const std::vector<State*>* otherObsVec);

    /*!
     * \brief getRoot get root
     * \return
     */
    Node* getRoot();

    /*!
     * \brief ExpandNode expands the node by generating it's children action nodes
     * Complexity: O_Simulator::getActions -> O(n_actions)
     * \param node to expand
     * \param state
     * \return
     */
    Node* expandNode(Node* node, const State* state);

    //AG150122: added obs2
    /*!      
     * \brief update update the belief
     * \param action done action
     * \param obs   observation (of this seeker)
     * \param reward reward (not used)
     * \param obs2 (optional) observation of other seeker (if available)
     * \param obs1p (optional) probability of choosing obs1 when 2 observations are given
     * \return
     */
    //bool update(int action, const State* obs, double reward, const State* obs2=NULL, double obs1p=-1 /*, HSPOMCP* hspomcp=NULL*/);

    //AG150611: update for multi
    /*!
     * \brief update the belief, whereby the action done can be passed
     * \param action
     * \param obs
     * \return
     */
    bool update(int action, const Observation* obs);  //State* thisSeekerObs, const std::vector<State*>* otherObsVec=NULL); //, double reward);

    /*!
     * \brief uctSearch UCT search, generates/grows tree for current root
     * Complexity: O(n_sim * max_depth * O_sim_step (* o_dist_calc) ) -> O(n) (except for not buffered distances)
     */
    void uctSearch();

    /*!
     * \brief greedyUCB get best action according to given tree
     * \param node
     * \param ucb if tree UCB is used which tries to choose actions not only because of highest score but also to explor
     * \return best action
     */
    int greedyUCB(Node* node, bool ucb);

    /*!
     * \brief rollout calculating the estimated reward
     * \param state
     * \return rollout reward
     */
    double rollout(const State* state);

    /*!
     * \brief selectAction select the action (using UCT search by default)
     * \params valueRet [out,optional] the value of the chosen action in the POMCP policy tree.
     * \return the action to do
     */
    int selectAction(double* valueRet=NULL);

    /*!
     * \brief checkTree check if the tree is consistent
     * \param node
     * \return
     */
    bool checkTree(Node* node);

    /*!
     * \brief checkNode check consistency of observation with node and next node
     * \param node
     * \param nextNode
     * \param a
     * \param obs
     * \return
     */
    bool checkNode(Node* node, Node* nextNode, int a, const State* obs);


    /*!
     * \brief getNextMultipleActions
     * \param n
     * \return
     */
    std::vector<int> getNextMultipleActions(int n);

    //! gmap only required to print belief
    //GMap* _gmap;

    //AG140501: made to test mem leak
    //AG150701: moved to MCTSTest
    // function to test the node
    //void testNodes(GMap* map);

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

    //! GMap, note: only for showing the belief on te map
    GMap* _map;

    //AG140908
    //! person path predictor, contains the predicted person's path (here it should reset the time stamps)
    PersonPathPredConsumer* _personPathPredConsumer;


    //! random device
    std::random_device _randomDevice;
    //! random generator
    std::mt19937 _randomGenerator; //(_randomDevice());
    //! probability distribution, 0 to 1
    std::uniform_real_distribution<> _uniformProbDistr;

};

}

#endif // MCTS_H
