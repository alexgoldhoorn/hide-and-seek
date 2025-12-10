#include "node.h"

#include <cstddef>
#include <cassert>
#include <sstream>

#include "simulator.h"

#include "Utils/generic.h"

//iriutils
#include "exceptions.h"

using namespace pomcp;
using namespace std;



// ---- BaseNode ----//

const string BaseNode::TREE_IDENT_STR = "\t";
unsigned int BaseNode::NODE_COUNT = 0;

BaseNode::BaseNode(Simulator* simulator) : _count(0), _totalV(0), _simulator(simulator) {
    assert(simulator!=NULL);
    _nodeID = NODE_COUNT++;
}


unsigned int BaseNode::getCount() {
    return _count;
}

double BaseNode::getValue() {
    if (_count==0) {
        //throw CException(_HERE_,"count is 0, no values stored yet");
        return _totalV; //like in Silver's code
    } else {
        return _totalV/_count;
    }
    //return (_count==0 ? _totalV : _totalV/_count );
}

void BaseNode::setCountAndValue(int count, double value) {
    assert(count>=0);
    _count = count;
    _totalV = value;
}

void BaseNode::addValue(double value) {
    _count++;
    _totalV += value;
}

void BaseNode::reset() {
    _count = 0;
    _totalV = 0;
}

// ---- Node ----//


Node::Node(Simulator* simulator, NodeA* parent) : BaseNode(simulator), _parent(parent) {
    //create node pointer for each child with init value NULL
    _children.resize(_simulator->getNumActions(),NULL);
}

Node::~Node() {
    for(vector<NodeA*>::iterator it = _children.begin(); it != _children.end(); it++) {
        if (*it == NULL)
            delete *it;
    }
}

NodeA* Node::setChild(unsigned int a, unsigned int count, double value) {
    assert(a<_simulator->getNumActions());

    /*if (a>=_children.size()) {
        _children.resize(a+1);
    }*/

    //new node
    /*NodeA* child = new NodeA(_simulator,this);
    if (count>0 && value>0) {
        child->setCountAndValue(count,value);
    }

    if (_children[a]!=NULL)
        delete _children[a];

    //add to list
    _children[a] = child; //.push_back(child);
    */

    NodeA* child = _children[a];
    if (child==NULL) {
        _children[a] = child = new NodeA(_simulator,this);
    } else {
        child->reset();
    }
    child->setCountAndValue(count,value);

    return child;
}

Belief* Node::getBelief() {
    return &_belief;
}

size_t Node::childCount() {
    return _children.size();
}

NodeA* Node::getChild(int a) {
    assert(a>=0 && a<_simulator->getNumActions());

    NodeA* child = _children[a];
    if (child == NULL) {
        _children[a] = child = new NodeA(_simulator, this);
    }

    return child;
}
/*
void Node::setBelief(Belief &belief) {
    _belief = belief;
}*/

string Node::toString(bool showChildren, string ident) {
    stringstream ss;
    ss << "N["<< _nodeID <<"]:"<<(_parent==NULL?"!":"")<<"v="<<getValue()<<" (#="<< _count <<"),#b="<<_belief.size()<<",#c="<<_children.size();
    if (showChildren) {
        string ident2 = ident + TREE_IDENT_STR;
        ss<<":"<<endl;
        int a=0;        
        FOREACH(NodeA*,n,_children) {            
            if ((*n)==NULL) {
                //ss << "- ";
            } else {
                ss <<ident2<<"a="<<a<<": "<<(*n)->toString(true,ident2)<<endl;
            }
            a++;
        }
        //ss<<">";
    }
    //ss<<"]";
    return ss.str();
}

void Node::reset()  {
    BaseNode::reset();

    FOR(i,_children.size()) {
        if (_children[i]!=NULL) {
            delete _children[i];
            _children[i] = NULL;
        }
    }
}

bool Node::isChildSet(int a) {
    assert(a>=0 && a<_simulator->getNumActions());
    return _children[a]!=NULL;
}

// ---- NodeA ----//

NodeA::NodeA(Simulator* simulator, Node* parent) : BaseNode(simulator), _parent(parent) {
    assert(parent!=NULL); //cannot be parent

    //init
    //create node pointer for each child with init value NULL
    _children.resize(_simulator->getNumObservations(),NULL);
}

NodeA::~NodeA() {
    for(vector<Node*>::iterator it = _children.begin(); it != _children.end(); it++) {
        if (*it == NULL)
            delete *it;
    }
}

Node* NodeA::setChild(unsigned int o, unsigned int count, double value) {
    assert(o<_simulator->getNumObservations());
    /*if (o>=_children.size()) {
        _children.resize(o+1);
    }*/

    Node* child = _children[o];
    if (child==NULL) {
        _children[o] = child = new Node(_simulator,this);
    } else {
        child->reset();
    }

    //child->setCountAndValue(count,value);
    return child;
}


size_t NodeA::childCount() {
    return _children.size();
}

Node* NodeA::getChild(int o) {
    assert(o>=0 && o<_simulator->getNumObservations());

    Node* child = _children[o];
    if (child==NULL) {
        //create new if not created
        _children[o] = child = new Node(_simulator,this);
    }
    return child;
}

string NodeA::toString(bool showChildren, string ident) {
    stringstream ss;
    ss << "A["<<_nodeID<<"]:"<<(_parent==NULL?"!":"")<<"v="<<getValue()<<" (#="<< _count <<"),#c="<<_children.size();

    if (showChildren) {
        string ident2 = ident + TREE_IDENT_STR;
        ss<<":"<<endl;
        int o=0;
        FOREACH(Node*,n,_children) {            
            if (*n==NULL) {
                //ss << "- ";
            } else {
                ss <<ident2<< "o="<<o<<": "<<(*n)->toString(true,ident2)<<endl;
            }
            o++;
        }
        //ss<<">";
    }
    //ss<<"]";
    return ss.str();
}

void NodeA::reset() {
    BaseNode::reset();

    FOR(i,_children.size()) {
        if (_children[i]!=NULL) {
            delete _children[i];
            _children[i] = NULL;
        }
    }
}


bool NodeA::isChildSet(int o) {
    assert(o>=0 && o<_simulator->getNumObservations());
    return _children[o]!=NULL;
}
