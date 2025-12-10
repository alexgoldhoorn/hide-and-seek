#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>

#include "belief.h"


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
    BaseNode(Simulator* simulator);

    /*!
     * \brief getValue Return the average value.
     * \return
     */
    virtual double getValue();

    /*!
     * \brief getCount Return count passed through node.
     * \return
     */
    virtual unsigned int getCount();

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

protected:
    static const std::string TREE_IDENT_STR; // = "\t";

    static unsigned int NODE_COUNT;

    //! value total
    double _totalV;
    //! count
    unsigned int _count;
    //! simulator
    Simulator* _simulator;
    //! unique node id
    unsigned int _nodeID;
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

    ~Node();

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

    /*!
     * \brief childCount number of children
     * \return
     */
    std::size_t childCount();

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
    virtual bool isChildSet(int a);

protected:
    //! the children for each action
    std::vector<NodeA*> _children;
    //! the parent
    NodeA* _parent;
    //! belief
    Belief _belief;
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

    ~NodeA();

    /*!
     * \brief addChild add child for observation o
     * \param o
     * \param count
     * \param value
     * \return
     */
    Node* setChild(unsigned int o, unsigned int count=0, double value=0);

    /*!
     * \brief getChild get child, NULL if not exists
     * \param o observation index
     * \return
     */
    Node* getChild(int o);

    /*!
     * \brief childCount number of children
     * \return
     */
    std::size_t childCount();

    virtual std::string toString(bool showChildren=false, std::string ident="");

    //virtual std::string toStringTree(std::string ident);

    virtual void reset();

    /*!
     * \brief isChildSet returns if the child node already exists
     * \param o
     * \return
     */
    virtual bool isChildSet(int o);

protected:
    //! the children
    std::vector<Node*> _children;
    //! the parent
    Node* _parent;
};

}

#endif // NODE_H
