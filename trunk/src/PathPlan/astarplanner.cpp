
#include "PathPlan/astarplanner.h"

#include <vector>
#include <algorithm>
#include <assert.h>
#include <cmath>

#include "exceptions.h"
#include "HSGame/gmap.h"
#include "Utils/generic.h"
#include "Base/hsglobaldata.h"
#include "Base/hsconfig.h"

#include <limits>
#include <iostream>
#include <sstream>



using namespace std;
using namespace hsutils;


void AStarPlanner::initDistMap() {
    //ag130227: init visible mat
    _distMap = AllocateDynamicArray<double>((unsigned int)_rows,(unsigned int)_cols,(unsigned int)_rows,(unsigned int)_cols);

    FOR(r1,_rows) {
        FOR(c1,_cols) {
            FOR(r2,_rows) {
                FOR(c2,_cols) {
                    if (_map[r1][c1]==GMap::GMAP_OBSTACLE || _map[r2][c2]==GMap::GMAP_OBSTACLE) {
                        _distMap[r1][c1][r2][c2] = GMap::DMAT_OBST;
                    } else {
                        _distMap[r1][c1][r2][c2] = GMap::DMAT_UNSET;
                    }
                }
            }
        }
    }
}


AStarPlanner::AStarPlanner(char **map, int rows, int cols, int neighbours) : PathPlanner() {
    _map = map;
    _rows = rows;
    _cols =cols;
    _allowDiagActions = (neighbours==8); // _neighbours = neighbours;
    _maxDist = rows * cols;

    initDistMap();
}

AStarPlanner::~AStarPlanner(){
    FreeDynamicArray<double>(_distMap, (unsigned int)_rows,(unsigned int)_cols,(unsigned int)_rows);

    //delete AStarNodes
    map<Pos,AStarNode*>::iterator it;
    for (it = _nodeMap.begin(); it != _nodeMap.end(); it++) {
        delete it->second;
    }
}



void AStarPlanner::setMapForGoal(int rg, int cg) {
    throw CException(_HERE_, "AstarPlanner::setMapForGoal: not implemented");
}

AStarNode* AStarPlanner::getNode(Pos p) {
    //find if the pos exists
    map<Pos,AStarNode*>::iterator it = _nodeMap.find(p);
    AStarNode* node = NULL;
    if (it==_nodeMap.end()) {
        //create new
        node = new AStarNode(p);
        _nodeMap[p] = node;
    } else {
        node = it->second; //_nodeMap[p];
    }
    return node;
}

//TODO: check, doesn't work well
AStarNode* AStarPlanner::astar(int c1, int r1, int c2, int r2, AStarNode** startNodeRet) {
    assert(r1>=0 && r1<_rows && c1>=0 && c1<_cols);
    assert(r2>=0 && r2<_rows && c2>=0 && c2<_cols);

    //if illegal node
    if (_map[r1][c1]==GMap::GMAP_OBSTACLE || _map[r2][c2]==GMap::GMAP_OBSTACLE) {
        DEBUG_PATHPLAN(cout<<"Error: path start/end includes obstacle"<<endl;);
        return NULL;
    }

    Pos startPos(r1,c1);
    Pos endPos(r2,c2);



    //if (startPos!=_lastStartPos) {
    /*if (endPos!=_lastGoalPos) {
        //we can't use previous nodes, reset G
        map<Pos,AStarNode*>::iterator it;
        for (it = _nodeMap.begin(); it != _nodeMap.end(); it++) {
            it->second->reset();
        }

        //now set to new end pos
        //_lastStartPos = startPos;
        //we can't reuse it:
        //_nodeMap.clear();
    }else {
        return getNode(endPos);

    }
    _lastGoalPos = endPos;*/

    //std::map<Pos,vector<AStarNode*>> openVec,closedVec;
    //open queue
    //NodesPriorityQueue openQ;
    OpenClosedWrapper openClose;

    //start node
    AStarNode* startNode = new AStarNode(startPos);
    if (startNodeRet!=NULL)
        *startNodeRet = startNode;

    //add to open list
    startNode->inOpen = true;
    //openVec[startPos].push_back(startNode);
    //openQ.push(startNode);
    openClose.addOpen(startNode);

    //A* algorithm
    while (!openClose.openEmpty()) {
        //get first (lowest)
        AStarNode* node = openClose.popOpen();
        //node->inOpen = false;
        //open.pop();

        DEBUG_PATHPLAN(cout<<"Open node:"<<node->toString()<<endl;);

        if (node->pos==endPos) {
            DEBUG_PATHPLAN(cout << "FOUND"<<endl;);
            return node;
            //break;
        }

        DEBUG_PATHPLAN(cout <<"Check neighb:"<<endl;);
        vector<AStarNode*> neighbVec = neighb(node);
        for(AStarNode* nnode : neighbVec) {
            DEBUG_PATHPLAN(cout << " - " <<nnode->toString(nnode)<<endl;);

            //calculate new values
            double d = 1;
            if (_useContSpace && node->pos.row()!=nnode->pos.row() && node->pos.col()!=nnode->pos.col())
                d = M_SQRT2;
            double g = node->g + d;
            double h = 0;
            if (_useContSpace) {
                h = endPos.distanceEuc(nnode->pos);
            } else if (_allowDiagActions) {
                h = abs(endPos.row() - nnode->pos.row()) + abs(endPos.col() - nnode->pos.col());
            }
            double f = g + h;

            //check if already was in open or closed (i.e. in map) AND has a better value
            // if nnode.f<f: then keep it in open or closed, else:
            if (nnode->f >= f) {
                //set new f,g,h and add to open, since the new one is better
                nnode->update(f,g,h,node);
                //nnode->inOpen = true;
                //open.push(nnode);
                openClose.addOpen(nnode);
            }

            if (nnode->pos==endPos) {
                DEBUG_PATHPLAN(cout << "FOUND (as childe)"<<endl;);
                return nnode;
                //break;
            }
        } // for neighbVec

        openClose.addClosed(node);
    } // while !open.empty()


    return NULL;
}


vector<AStarNode*> AStarPlanner::neighb(AStarNode* n) {
    const Pos& p = n->pos;
    vector<AStarNode*> nVec;

    //loop neighbours
    for (int r=p.row()-1;r<=p.row()+1;r++) {
        for(int c=p.col()-1;c<=p.col()+1;c++) {
            if (    (_allowDiagActions || r==p.row() || c==p.col()) && //allow diagonal, or action should be not diagonal
                    !p.equalsInt(r,c) &&
                    //!(r==p.row() && c==p.col()) && //not same cell,
                    r>=0 && r<_rows && c>=0 && c<_cols &&  //and not out of map,
                    _map[r][c]!=GMap::GMAP_OBSTACLE) { //and not obstacle

                Pos np(r,c);
                AStarNode* node = new AStarNode(np);//getNode(np);
                nVec.push_back(node);
            }
        }
    }

    return nVec;
}


double AStarPlanner::distance(int c1, int r1, int c2, int r2){
    //assert pos give are not obstacles
    assert(c1>=0 && c1<_cols && r1>=0 && r1<_rows && r2>=0 && r2<_rows && c2>=0 && c2<_cols);
    double d = -2;
    if (_distMap[r1][c1][r2][c2]==GMap::DMAT_OBST || _distMap[r1][c1][r2][c2]==GMap::DMAT_NO_PATH)  {
        d = -1; //no path
    } else {
        if (_distMap[r1][c1][r2][c2]==GMap::DMAT_UNSET) {
            //setMapForGoal(r2,c2);

            AStarNode* pathNode = astar(c1,r1,c2,r2);
            //AG TEST if loop
            AStarNode* node = pathNode;
            int nodeCount=0;
            cout << "Node path: "<<endl;
            while (node!=NULL){
                nodeCount++;
                cout << node->toString(node)<< " "<<flush;
                node=node->parent;
                if (node==pathNode) {
                    cout << "ERROR loop in A* path, #nodes="<<nodeCount<<endl;
                    break;
                }
            }


            //now update dist matrix
            if (pathNode==NULL) {
                DEBUG_PATHPLAN(cout<<"Warning: No path from r"<<r1<<"c"<<c1<<" to r"<<r2<<"c"<<c2<<endl;);
                _distMap[r1][c1][r2][c2]=_distMap[r2][c2][r1][c1]=GMap::DMAT_NO_PATH;
            } else {
                while (pathNode!=NULL) {
                    Pos p = pathNode->pos;
                    _distMap[r1][c1][p.row()][p.col()] = _distMap[p.row()][p.col()][r1][c1] = pathNode->g;
                    pathNode = pathNode->parent;
                }
            }

        }

        d = _distMap[r1][c1][r2][c2];
    }

    return d;
}

void AStarPlanner::nextStep(int c1, int r1, int c2, int r2, int* nextC, int* nextR) {
    //throw CException(_HERE_, "AstarPlanner::nextStep: not implemented");
    assert(nextC!=NULL && nextR!=NULL);


    AStarNode* startNode = NULL;
    AStarNode* pathNode = astar(c1,r1,c2,r2,&startNode);
    AStarNode* node = pathNode;
    cout <<"StartNode:"<<startNode->toString(startNode)<<endl;
    cout <<"EndNode:"<<pathNode->toString(pathNode)<<endl;
    while (node!=NULL && node->parent!=NULL && node->parent!=startNode) {
        node = node->parent;
    }

    cout << "PRINTING path: for r"<<r1<<"c"<<c1<<" - r"<<r2<<"c"<<c2 <<":"<<endl;
    AStarNode* temp = pathNode;
   /* while (temp!=NULL) {
        cout << " - "<<temp->toString();
        if (temp==pathNode)
            cout <<" [end]";
        if (temp==startNode)
            cout <<" [start]";
        if (temp->parent==startNode)
            cout <<" [parent start]";
        if (temp->parent==NULL)
            cout <<" [root]";
        cout<<endl;
        temp=temp->parent;
    }*/
    cout<<"<<PATH>"<<endl;



    if (node!=NULL) { //startNode!=NULL && startNode->child!=NULL) {
        Pos p = node->pos;
        *nextR = p.row();
        *nextC = p.col();
    } else {
        *nextC = *nextR = -1;
    }
}

void AStarPlanner::printDistMap() {
    throw CException(_HERE_, "AstarPlanner::printDistMap: not implemented");
}

bool AStarPlanner::doesCacheResults() {
    return false;
}

std::string AStarPlanner::getName() {
    return "A* PathPlanner";
}


AStarNode::AStarNode(Pos p) : pos(p) {
    /*if (g==-1)
        this->g=std::numeric_limits<int>::max();
    else
        this->g=g;*/
    reset();
}

void AStarNode::reset() {
    parent /*= child*/ = NULL;
    inOpen = false;
    g = h = 0;
    f = std::numeric_limits<int>::max();
}

bool AStarNode::operator==(const AStarNode& n) const {
    return n.pos==pos;
}
bool AStarNode::operator<(const AStarNode& n) const {
    return n.pos<pos;
}
bool AStarNode::operator>(const AStarNode& n) const {
    return n.pos>pos;
}

std::string AStarNode::toString(AStarNode* self) {
    stringstream ss;
    ss<<"Node-"<<pos.toString();
    if (parent!=NULL && *self==*parent) {
        ss<<"[circular-to-self]";
    } else if (parent!=NULL) {
        ss<<"-path:"<<parent->toString(self);
    }
    return ss.str();
}

std::string AStarNode::toString() {
    stringstream ss;
    ss<<"Node-"<<pos.toString();
    if (parent!=NULL) {
        ss<<"-path:"<<parent->pos.toString();
    }
    return ss.str();
}

void AStarNode::update(double f, double g, double h, AStarNode *parent) {
    this->f=f;
    this->g=g;
    this->h=h;
    this->parent=parent;
}

/*void AStarNode::resetG() {
    this->g=std::numeric_limits<int>::max();
}
*/

PQNodeComparison::PQNodeComparison(const bool& revparam) {
    reverse=revparam;
}

bool PQNodeComparison::operator() (const AStarNode* lhs, const AStarNode* rhs) const {
    // return > to assure to have lowest first
    return lhs->f>rhs->f; //(lhs->operator <(*rhs));
}


void OpenClosedWrapper::addOpen(AStarNode *node) {
    node->inOpen = true;
    _openQ.push(node);
    _openVM[node->pos].push_back(node);
}

AStarNode* OpenClosedWrapper::popOpen() {
    AStarNode* node = _openQ.top();
    if (node==nullptr)
        return nullptr;
    _openQ.pop();
    vector<AStarNode*> nodeVec = _openVM[node->pos];
    nodeVec.erase(std::find(nodeVec.begin(),nodeVec.end(),node));
    node->inOpen = false;
    return node;
}

bool OpenClosedWrapper::openEmpty() {
    return _openQ.empty();
}

void OpenClosedWrapper::addClosed(AStarNode *node) {
    _closedVM[node->pos].push_back(node);
}

vector<AStarNode*> OpenClosedWrapper::findClosed(Pos pos) {
    return _closedVM[pos];
}

vector<AStarNode*> OpenClosedWrapper::findOpen(Pos pos) {
    return _openVM[pos];
}

bool OpenClosedWrapper::hasHigherFopen(Pos pos, double f) {
    auto vec = _openVM[pos];
    for(auto node : vec) {
        if (node->f > f)
            return true;
    }
    return false;
}
bool OpenClosedWrapper::hasHigherFclosed(Pos pos, double f) {
    auto vec = _closedVM[pos];
    for(auto node : vec) {
        if (node->f > f)
            return true;
    }
    return false;
}
