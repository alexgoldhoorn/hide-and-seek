#include "POMCP/node.h"

#include <cstddef>
#include <cassert>
#include <sstream>
#include <iostream>

#include "POMCP/simulator.h"

#include "Utils/generic.h"

//iriutils
#include "exceptions.h"

using namespace pomcp;
using namespace std;



// ---- BaseNode ----//

const string BaseNode::TREE_IDENT_STR = "\t";
unsigned long BaseNode::NODE_COUNT = 0;

BaseNode::BaseNode(Simulator* simulator) : _totalV(0), _count(0), _simulator(simulator) {
    assert(simulator!=NULL);
    _nodeID = NODE_COUNT++;
#ifdef DEBUG_POMCP_LAST_VALUE
    _lastValue = 0;
#endif
}

BaseNode::~BaseNode(){
}

unsigned long BaseNode::generatedNodeCount() {
    return NODE_COUNT;
}


unsigned int BaseNode::getCount() const {
    return _count;
}
//create child
//NodeA* child = node->setChild(*it);
double BaseNode::getValue() const {
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
#ifdef DEBUG_POMCP_LAST_VALUE
    _lastValue = value;
#endif
}

void BaseNode::reset() {
    _count = 0;
    _totalV = 0;
}

#ifdef DEBUG_POMCP_LAST_VALUE
double BaseNode::getLastValue() {
    return _lastValue;
}
#endif

unsigned long BaseNode::getNodeID() const {
    return _nodeID;
}

// ---- Node ----//


Node::Node(Simulator* simulator, NodeA* parent) : BaseNode(simulator), _parent(parent) {
    //create node pointer for each child with init value NULL
    _children.resize(_simulator->getNumActions(),NULL);
}

Node::~Node() {
    /*for(vector<NodeA*>::iterator it = _children.begin(); it != _children.end(); it++) {
        if (*it != NULL) //AG130903: MEM LEAK -> this was: '*it == NULL' -> only deleting NULL!
            delete *it;
    }*/
}

void Node::deleteChildrenExceptFor(BaseNode *doNotDeleteNode) {
    for(vector<NodeA*>::iterator it = _children.begin(); it != _children.end(); it++) {
        if ((*it)!=NULL && (doNotDeleteNode==NULL || (*it)->getNodeID()!=doNotDeleteNode->getNodeID())) {
            (*it)->deleteChildrenExceptFor(doNotDeleteNode);
            delete *it;
        }
    }
}

NodeA* Node::setChild(unsigned int a, unsigned int count, double value) {
    assert(a<_simulator->getNumActions());

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

size_t Node::childCount() const {
    return _children.size();
}

NodeA* Node::getChild(int a) {
    assert(a>=0 && a<(int)_simulator->getNumActions());

    //NOTE: will be NULL if it has not yet been set
    return  _children[a]; //child;
}


NodeA* Node::createChild(int a) {
    assert(a>=0 && a<(int)_simulator->getNumActions());

    if (_children[a]!=NULL) {
        cout << "Node::createChild: child action "<< a <<" already exists" << endl;
        throw CException(_HERE_, "Node::createChild: child action already exists");
    }

    _children[a] = new NodeA(_simulator, this);

    return _children[a];
}


string Node::toString(bool showChildren, string ident) {
    stringstream ss;


    ss << "O["<< _nodeID <<"]:"<<(_parent==NULL?"!":"")<<"v="<<getValue()<<" (#="<< _count <<"),#b="<<_belief.size()<<",#c="<<_children.size();
    if (showChildren) {
        string ident2 = ident + TREE_IDENT_STR;

        ss<<":"<<endl;
        if (_belief.size()>0)
            ss<<ident2<<"belief="<<_belief.toString(DEBUG_NUM_BELIEF_TOSTRING);

        int a=0;
        FOREACH(NodeA*,n,_children) {
            ss <<ident2<<"a="<<a<<": "<< ((*n)==NULL ? "NULL" : (*n)->toString(true,ident2) )<<endl;
            a++;
        }

    }

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

bool Node::isChildSet(int a) const {
    assert(a>=0 && a<(int)_simulator->getNumActions());
    return _children[a]!=NULL;
}


// ---- NodeA ----//

NodeA::NodeA(Simulator* simulator, Node* parent) : BaseNode(simulator), _parent(parent) {
    assert(parent!=NULL); //cannot be parent
}

NodeA::~NodeA() {    
/*    for(map<string,ObsNodePair>::iterator it = _children.begin(); it != _children.end(); it++) {
        assert(it->second.observation != NULL);
        delete it->second.observation;
        assert(it->second.node != NULL);
        delete it->second.node;
    }*/
}



void NodeA::deleteChildrenExceptFor(BaseNode *doNotDeleteNode) {
    /*for(vector<NodeA*>::iterator it = _children.begin(); it != _children.end(); it++) {
        if ((*it)!=NULL && it->getNodeID()!=doNotDeleteNode->getNodeID())
            delete *it;
    }*/

    for(map<string,ObsNodePair>::iterator it = _children.begin(); it != _children.end(); it++) {
        assert(it->second.observation != NULL);
        delete it->second.observation;
        assert(it->second.node != NULL);
        if (doNotDeleteNode==NULL || it->second.node->getNodeID()!=doNotDeleteNode->getNodeID()) {
            it->second.node->deleteChildrenExceptFor(doNotDeleteNode);
            delete it->second.node;
        }
    }
}


/*Node* NodeA::setChild(State* o, unsigned int count, double value) {
    assert(o!=NULL);

    string oHash = o->getHash();

    Node* child = NULL;
    if (_children.find(oHash) == _children.end()) {
        //add child
        _children[oHash].node = child = new Node(_simulator,this);
        _children[oHash].observation = o->copy();
    } else {        
        child = _children[oHash].node;
        child->reset();
    }

    //NOTE: why are the value and count not set ???

    return child;
}
*/

size_t NodeA::childCount() const {
    return _children.size();
}

ObsNodePair* NodeA::getChildItem(int i) {
    assert(i>=0 && i<(int)_children.size());

    map<string,ObsNodePair>::iterator it = _children.begin();
    std::advance(it,i);

    return &(it->second);
}

Node* NodeA::getChild(State* o) {
    assert(o != NULL);
    string oHash = o->getHash();

    if (_children.find(oHash) == _children.end()) {
        return NULL;
    } else {
        return _children[oHash].node;
    }
}


Node* NodeA::createChild(State* o, bool deleteAfterUsage) {
    assert(o != NULL);
    string oHash = o->getHash();

    if (_children.find(oHash) != _children.end()) {
        cout << "NodeA::createChild: obs " << o->toString() << " already exists as child"<<endl;
        throw CException(_HERE_, "NodeA::createChild: obs already exists as child");
    }

    _children[oHash].node = new Node(_simulator,this);
    if (deleteAfterUsage) { //AG140502: re-use this object
        _children[oHash].observation = o;
    } else {
        _children[oHash].observation = o->copy();
    }

    return _children[oHash].node;
}

string NodeA::toString(bool showChildren, string ident) {
    stringstream ss;
    ss << "A["<<_nodeID<<"]:"<<(_parent==NULL?"!":"")<<"v="<<getValue()<<" (#="<< _count <<"),#c="<<_children.size();

    if (showChildren) {
        string ident2 = ident + TREE_IDENT_STR;
        ss<<":"<<endl;

        for(map<string,ObsNodePair>::iterator it = _children.begin(); it != _children.end(); it++) {
            ss <<ident2<< "o="<<it->second.observation->toString()<<": "<<it->second.node->toString(true,ident2)<<endl;
        }
    }
    return ss.str();
}

void NodeA::reset() {
    BaseNode::reset();

    for(map<string,ObsNodePair>::iterator it = _children.begin(); it != _children.end(); it++) {
        assert(it->second.observation != NULL);
        delete it->second.observation;
        assert(it->second.node != NULL);
        delete it->second.node;
    }
}


bool NodeA::isChildSet(State* o) const {
    //assert(o>=0 && o<_simulator->getNumObservations());
    //return _children[o]!=NULL;
    assert(o!=NULL);
    return (_children.find(o->getHash()) != _children.end());
}

std::map<std::string,ObsNodePair>::iterator NodeA::itBeginChild() {
    return _children.begin();
}

std::map<std::string,ObsNodePair>::iterator NodeA::itEndChild() {
    return _children.end();
}


ObsNodePair::ObsNodePair(State *obs_, Node *node_) {
    node = node_;
    observation = obs_;
}

ObsNodePair::ObsNodePair() {
    node = NULL;
    observation = NULL;
}
