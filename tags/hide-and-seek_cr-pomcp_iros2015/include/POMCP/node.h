#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>
#include <map>

#include "POMCP/belief.h"


/*  TODO !!!
 *
 * WHAT TO DO WITH deleting nodes?? also children -> NO! (?) if we cut the tree from above,
 * we should cut the parents
 *
 */

namespace pomcp {

class Simulator;

/*!
 * \brief The BaseNode class Basic Node class that keeps track of average value.
 */
class BaseNode {
public:
    static unsigned long generatedNodeCount();

    BaseNode(Simulator* simulator);

    virtual ~BaseNode();

    /*!
     * \brief getValue Return the average value.
     * \return
     */
    virtual double getValue() const;

    /*!
     * \brief getCount Return count passed through node.
     * \return
     */
    virtual unsigned int getCount() const;

    /*!
     * \brief setCountAndValue set the count and value
     * \param count
     * \param value
     */
    virtual void setCountAndValue(int count,double value);

    /*!
     * \brief addValue add a value (and adds count)
     * \param value
     */
    virtual void addValue(double value);

    /*!
     * \brief toString to string
     * \return
     */
    virtual std::string toString(bool showChildren=false, std::string ident="")=0;

    /*!
     * \brief toStringTree to string tree where tree is printed with indents
     * \param ident
     * \return
     */
    //virtual std::string toStringTree(std::string ident)=0;

    /*!
     * \brief reset the class to reuse
     */
    virtual void reset();

    /*! Get node id.
      */
    virtual unsigned long getNodeID() const;

    /*!
     * \brief deleteChildrenExceptFor delete all children except for the child node (and it's children)
     * \param doNotDeleteNode
     */
    virtual void deleteChildrenExceptFor(BaseNode* doNotDeleteNode)=0;

#ifdef DEBUG_POMCP_LAST_VALUE
    double getLastValue();
#endif

protected:
    static const std::string TREE_IDENT_STR; // = "\t";

    static unsigned long NODE_COUNT;

    //! value total
    double _totalV;
    //! count
    unsigned int _count;
    //! simulator
    Simulator* _simulator;
    //! unique node id
    unsigned long _nodeID;

#ifdef DEBUG_POMCP_LAST_VALUE
    //AG140128: debug, last value
    double _lastValue;
#endif
};

class NodeA;

/*!
 * \brief The Node class Node that contains value and children for (each) action(s).
 */
class Node : public BaseNode
{
public:
    /*!
     * \brief Node
     * \param simulator
     * \param parent NULL if root
     */
    Node(Simulator* simulator, NodeA* parent);

    virtual ~Node();

    /*!
     * \brief addChild add child for certain action
     * \param a
     * \param count
     * \param value
     */
    NodeA* setChild(unsigned int a, unsigned int count=0, double value=0);


    /*!
     * \brief getBelief get the belief of this node
     * \return
     */
    Belief* getBelief();

    /*!
     * \brief getChild get child, NULL if not exists
     * \param a action index
     * \return
     */
    NodeA* getChild(int a);

    //AG130829: create the child
    /*!
     * \brief createChild create the child
     * \param a
     * \return
     */
    NodeA* createChild(int a);

    /*!
     * \brief childCount number of children
     * \return
     */
    std::size_t childCount() const;

    /*!
     * \brief setBelief set a new belief
     * \param belief
     */
    //void setBelief(Belief& belief);

    virtual std::string toString(bool showChildren=false, std::string ident="");

    virtual void reset();

    /*!
     * \brief isChildSet returns if the child node already exists
     * \param a
     * \return
     */
    virtual bool isChildSet(int a) const;

    /*!
     * \brief check check if the observation is consistent with the node
     * \param obs
     * \return
     */
    //virtual bool check(int obs, NodeA* parentNode);

    virtual void deleteChildrenExceptFor(BaseNode* doNotDeleteNode);

protected:
    //! the children for each action
    std::vector<NodeA*> _children;
    //! the parent
    NodeA* _parent;
    //! belief
    Belief _belief;
};

/*!
 * \brief The ObsNodePair struct pair of observation (State*) and node
 */
struct ObsNodePair {
    ObsNodePair();
    ObsNodePair(State* obs_,Node* node_);
    State* observation;
    Node* node;
};

/*!
 * \brief The NodeA class contains value and list for observations
 */
class NodeA : public BaseNode
{
public:
    /*!
     * \brief NodeA
     * \param simulator
     * \param parent cannot be NULL (ie root)
     */
    NodeA(Simulator* simulator, Node* parent);

    virtual ~NodeA();

    /*!
     * \brief addChild add child for observation o
     * \param o
     * \param count
     * \param value
     * \return
     */
    //AG131204: disabled because not used
    //Node* setChild(State* o, unsigned int count=0, double value=0);

    /*!
     * \brief getChild get child, NULL if not exists
     * \param o observation index
     * \return
     */
    Node* getChild(State* o);

    /*!
     * \brief getChild
     * \param i
     * \return
     */
    //ObsNodePair getChildByIndex(size_t i);

    /*!
     * \brief getChildItem the ith item
     * \param i
     * \return
     */
    ObsNodePair* getChildItem(int i);


    //AG130829: create the child
    /*!
     * \brief createChild
     * \param o
     * \return
     */
    Node* createChild(State* o, bool deleteAfterUsage=false);

    /*!
     * \brief childCount number of children
     * \return
     */
    std::size_t childCount() const;

    virtual std::string toString(bool showChildren=false, std::string ident="");

    //virtual std::string toStringTree(std::string ident);

    virtual void reset();

    /*!
     * \brief isChildSet returns if the child node already exists
     * \param o
     * \return
     */
    virtual bool isChildSet(State* o) const;

    /*!
     * \brief check check if the action is consistent with the node and the parent
     * \param a
     * \return
     */
    //virtual bool check(int a, Node* parentNode);

    /*!
     * \brief itBeginChild return iterator begin of children
     * \return
     */
    std::map<std::string,ObsNodePair>::iterator itBeginChild();

    /*!
     * \brief itEndChild return iterator end of children
     * \return
     */
    std::map<std::string,ObsNodePair>::iterator itEndChild();

    virtual void deleteChildrenExceptFor(BaseNode* doNotDeleteNode);

protected:
    //! the children
    //std::vector<Node*> _children;
    //std::map<State*,Node*> _children;

//TODO
    //use the getHash (create it) of the state
    std::map<std::string,ObsNodePair> _children;

    //! the parent
    Node* _parent;
};

}

#endif // NODE_H
