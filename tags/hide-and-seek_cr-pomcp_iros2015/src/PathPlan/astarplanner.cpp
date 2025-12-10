
#include "PathPlan/astarplanner.h"

#include <vector>
#include <algorithm>
#include <assert.h>

#include "exceptions.h"
#include "HSGame/gmap.h"
#include "Utils/generic.h"
#include "hsglobaldata.h"
#include "hsconfig.h"

#include <limits>
#include <iostream>
#include <sstream>


using namespace std;

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
    map<Pos,AStarNode*>::iterator it = _nodeMap.find(p);
    AStarNode* node = NULL;
    if (it==_nodeMap.end()) {
        //create new
        node = new AStarNode(p);
        _nodeMap[p] = node;
    } else {
        node = _nodeMap[p];
    }
    return node;
}

//TODO: check, doesn't work well
AStarNode* AStarPlanner::astar(int c1, int r1, int c2, int r2, AStarNode** startNodeRet) {
    assert(r1>=0 && r1<_rows && c1>=0 && c1<_cols);
    assert(r2>=0 && r2<_rows && c2>=0 && c2<_cols);

    Pos startPos(r1,c1);
    Pos endPos(r2,c2);

    /*if (endPos!=_lastGoalPos) {
        //we can't use previous nodes, reset G
        map<Pos,AStarNode*>::iterator it;
        for (it = _nodeMap.begin(); it != _nodeMap.end(); it++) {
            it->second->resetG();
        }

        //now set to new end pos
        _lastGoalPos = endPos;
    }*/
    if (startPos!=_lastStartPos) {
        //we can't use previous nodes, reset G
        map<Pos,AStarNode*>::iterator it;
        for (it = _nodeMap.begin(); it != _nodeMap.end(); it++) {
            it->second->resetG();
        }

        //now set to new end pos
        _lastStartPos = startPos;
    }

    //open queue
    NodesPriorityQueue open;

    //start node
    AStarNode* startNode = getNode(startPos);
    startNode->g=0;
    if (startNodeRet!=NULL)
        *startNodeRet = startNode;


    //add to open list
    startNode->inOpen = true;
    open.push(startNode);

    //A* algorithm
    while (!open.empty()) {
        //get first (lowest)
        AStarNode* node = open.top();
        node->inOpen = false;
        open.pop();
        DEBUG_PATHPLAN(cout<<"Open node:"<<node->toString()<<endl;);

        if (node->pos==endPos) {
            DEBUG_PATHPLAN(cout << "FOUND"<<endl;);
            return node;
            //break;
        }

        DEBUG_PATHPLAN(cout <<"Check neighb:"<<endl;);
        vector<AStarNode*> neighbVec = neighb(node);
        for(AStarNode* nnode : neighbVec) {
            DEBUG_PATHPLAN(cout << " - " <<nnode->toString()<<endl;);
            if (nnode->g > node->g + 1) { //1 is step from neighbour to node
                double d = 1;
                if (_useContSpace && node->pos.row()!=nnode->pos.row() && node->pos.col()!=nnode->pos.col())
                    d = M_SQRT2;

                //increase distance
                nnode->g = node->g+d;
                nnode->parent = node;
                //node->child = nnode;
                if (!nnode->inOpen) {
                    nnode->inOpen = true;
                    open.push(nnode);
                }
            }
        }

    }

    return NULL;
}


vector<AStarNode*> AStarPlanner::neighb(AStarNode* n) {
    const Pos& p = n->pos;
    vector<AStarNode*> nVec;

    //loop neighbours
    for (int r=p.row()-1;r<=p.row()+1;r++) {
        for(int c=p.col()-1;c<=p.col()+1;c++) {
            if (    (_allowDiagActions || r==p.row() || c==p.col()) && //allow diagonal, or action should be not diagonal
                    !(r==p.row() && c==p.col()) && //not same cell,
                    r>=0 && r<_rows && c>=0 && c<_cols &&  //and not out of map,
                    _map[r][c]!=GMap::GMAP_OBSTACLE) { //and not obstacle

                Pos np(r,c);
                AStarNode* node = getNode(np);
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
    while (node!=NULL && node->parent!=NULL && node->parent!=startNode) {
        node = node->parent;
    }

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


AStarNode::AStarNode(Pos p, int g) : pos(p) {
    if (g==-1)
        this->g=std::numeric_limits<int>::max();
    else
        this->g=g;
    parent /*= child*/ = NULL;
    inOpen = false;
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

std::string AStarNode::toString() {
    stringstream ss;
    ss<<"Node-"<<pos.toString();
    if (parent!=NULL) {
        ss<<"-path:"<<parent->pos.toString();
    }
    return ss.str();
}

void AStarNode::resetG() {
    this->g=std::numeric_limits<int>::max();
}

PQNodeComparison::PQNodeComparison(const bool& revparam) {
    reverse=revparam;
}

bool PQNodeComparison::operator() (const AStarNode* lhs, const AStarNode* rhs) const {
    return (lhs->operator <(*rhs));
}
