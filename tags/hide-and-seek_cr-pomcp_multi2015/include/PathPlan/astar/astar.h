// Copyright (C) 2011 Institut de Robòtica i Informàtica Industrial, CSIC-UPC.
// Author Joan Perez
// All rights reserved.
//
// This file is part of path_planning
// iriutils is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _ASTAR_H_
#define _ASTAR_H_

#include "PathPlan/map_grid/map_grid.h"
#include "PathPlan/map_grid/ppm_image_map.h"

#include <vector>
#include <list>

//#define _DEBUG_MODE_

namespace astar
{

class Node
{
  public:
    static unsigned int NUM_NODES;
    const static unsigned int UNVISITED = 0;
    const static unsigned int OPEN      = 1;
    const static unsigned int CLOSE     = 2;

  protected:
    unsigned int xx_, yy_;
    float cost2go_;
    float cost2come_;
    float cost_;
    unsigned int state_;
    Node * parent_;

  public:
    Node(void);
    Node(const unsigned int & ix, const unsigned int & iy);
    ~Node(void);

    void markAsUnvisited(){ state_ = UNVISITED; }
    void markAsOpen(){      state_ = OPEN;  }
    void markAsClose(){     state_ = CLOSE; }
    
    void parent(Node *N) { parent_ = N; }
    Node * parent() { return parent_; }
    unsigned int xx()    const{ return xx_;        }
    unsigned int yy()    const{ return yy_;        }
    unsigned int state() const{ return state_;     }
    float cost2go()      const{ return cost2go_;   }
    float cost2come()    const{ return cost2come_; }
    float cost()         const{ return cost_;      }
    void cost2go(const float & c2g)   { cost2go_ = c2g;   }
    void cost2come(const float & c2c) { cost2come_ = c2c; }
    void cost(const float & c)        { cost_ = c;        }

    void setNodeCell(const unsigned int & ix, const unsigned int & iy);
    void updateCost();
    std::string printCell() const;
    void print() const;
};

class AStar
{
  class HeapCompareFunction 
  {
    public:
      bool operator() ( const Node *n, const Node *m ) const
      {
        if(n->cost() != m->cost())
        return n->cost() < m->cost();
        else return n->cost2come() < m->cost2come();
      }
  };

  public:
    static const unsigned int CHEBSHEV_HEURISTIC       = 1;
    static const unsigned int CHEBSHEV_2DIAG_HEURISTIC = 2;
    static const unsigned int EUCLIDEAN_HEURISTIC      = 3;
    
    static const unsigned int YOUNG_BREAK              = 1;
    static const unsigned int STRAIGHT_BREAK           = 2;
    static const unsigned int COST2COME_BREAK          = 3;
    
    static const unsigned int SECONDSJANUARY09         = 1230764400;

  protected:
    map_grid::MapGrid grid_;
    std::vector<Node*> vOpenSet_, vVisitedMap_;
    std::list<Node*> vPath_;

    unsigned int gX_, gY_, iX_, iY_;
    unsigned int distHeuristic_, tieBreaking_;
    float maxCircleDist_;
    
    map_grid::PPMImageMap debug_image_;
    std::string image_folder_, image_filename_, exec_folder_;
    unsigned int iterations_, requests_;
    Node* goal_node_;
    
    bool cancel_execution_;

    void clearMaps(void);
    void saveInitMapFrame(void);
    void saveMapFrame(void);
    void createExecutionFolder(void);
    void printCurrentIteration(Node *N) const;
    void printPath(std::list<Node*> path) const;
    void saveImagePath(std::list<Node*> path, const std::string & filename) const;

    float distance(Node* & node, const bool & toGoal) const;
    float Cost2Come(Node* & node) const;
    float Cost2Go(Node* & node) const;
    void computeNodeCosts(Node* & node);
    bool isInOpenSet(const unsigned int & ix, const unsigned int & iy, unsigned int & index) const;
    bool isInCloseSet(const unsigned int & ix, const unsigned int & iy, unsigned int & index) const;
    Node* getBestNode(void);
    inline unsigned int getVectorIndex(const unsigned int & ix, const unsigned int & iy) const;
    void addNode(Node* & node);

    bool findClosestFreeCell(unsigned int & ix, unsigned int & iy);
    bool computePath(const float & ix,  const float & iy, 
                     const float & gx,  const float & gy, 
                     const unsigned int & h=CHEBSHEV_2DIAG_HEURISTIC, 
                     const unsigned int & tb=STRAIGHT_BREAK);
    void checkNeighbourhood(Node* & node);
    void recoverPathFromList();
    void removeRedundantPointsFromPath(Node* & node);
    void addWayPoints(const float & maxCircleDist);
    void removeClosePointsFromPath(const float & minDist);
    void createPath();

  public:
    AStar(const map_grid::MapGrid & map, const float & maxCd=3, 
          const unsigned int & dH=CHEBSHEV_2DIAG_HEURISTIC, 
          const unsigned int & tB=STRAIGHT_BREAK);
    ~AStar(void);
    
    void createRobotNavigableMap(const float & robotClearance=0.7f, const float & maxRobotStep=0.07f,
                                 const float & xFloor=0.f, const float & yFloor=0.f);
    bool computeDensePath(const float & ix,  const float & iy, 
                          const float & gx,  const float & gy, 
                          const unsigned int & h=CHEBSHEV_2DIAG_HEURISTIC, 
                          const unsigned int & tb=STRAIGHT_BREAK);
    bool computeAndRefinePath(const float & ix,  const float & iy, 
                              const float & gx,  const float & gy, 
                              const unsigned int & h=CHEBSHEV_2DIAG_HEURISTIC, 
                              const unsigned int & tb=STRAIGHT_BREAK);
    unsigned int getNumIters() const { return iterations_; }
    void getCurrentConfiguration(void) const;
    double timeStamp(void);//this method returns the seconds from 1st jan 09. microsecond resolution
	
	//ag111125: shortest path_planning	
	std::list<Node*>* getPath() {return &vPath_; }
};

}

#endif
