#ifndef ASTARPLANNER_H
#define ASTARPLANNER_H


#include "PathPlan/pathplanner.h"

#include <string>
#include <queue>
#include <map>


/*!
 * \brief The AStarNode struct Node for A* planner
 */
struct AStarNode {
    AStarNode(Pos p, int g=-1);

    //! check equality only through pos
    bool operator==(const AStarNode& n) const;
    //! for comparison (using g)
    bool operator<(const AStarNode& n) const;
    bool operator>(const AStarNode& n) const;

    //! to string
    std::string toString();
    //! reset G
    void resetG();

    //! pos
    Pos pos;
    //! estimation path length
    int g;
    //! parent
    AStarNode* parent;
    //! is in open list
    bool inOpen;
};

/*!
 * \brief The PQNodeComparison class used by PriorityQueue to compare AStarNodes
 */
class PQNodeComparison
{
  bool reverse;
public:
  PQNodeComparison(const bool& revparam=false);
  bool operator() (const AStarNode* lhs, const AStarNode* rhs) const;
};

//! priority queue list used
typedef std::priority_queue<int,std::vector<AStarNode*>,PQNodeComparison> NodesPriorityQueue;

/*!
 * \brief The AStarPlanner class A* planner
 */
class AStarPlanner : public PathPlanner {
public:
    AStarPlanner(char** map, int rows, int cols, int neighbours=8);
    virtual ~AStarPlanner();

    //returns distance, or -1 if no path possible (e.g. goal/init is obstacle)
    virtual double distance(int x1, int y1, int x2, int y2);
    virtual void nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY);

    virtual bool doesCacheResults();

    void printDistMap();

    virtual std::string getName();

protected:
    //AG130617: moved to rowgoal and colgoal (was x,y)
    void setMapForGoal(int rg, int cg);

    /*!
     * \brief astar find the path from r1,c1 -> r2,c2
     * \param c1
     * \param r1
     * \param c2
     * \param r2
     * \param startNode [out] start node of the path
     * \return end node of the path
     */
    AStarNode* astar(int c1, int r1, int c2, int r2, AStarNode** startNode=NULL);

    /*!
     * \brief AStarPlanner::neighb get the neighbor nodes
     * \param n
     * \return
     */
    std::vector<AStarNode*> neighb(AStarNode* n);

    /*!
     * \brief getNode use the map to get a AStarNode, or generate it if it doesn't exist
     * \param p
     * \return
     */
    AStarNode* getNode(Pos p);

    //! init dist map
    void initDistMap();


private:
    //! map (like in GMap)
    char** _map;
    //! size map
    int _cols;
    int _rows;

    double**** _distMap;

    //! last goal
    Pos _lastGoalPos;
    //! last start
    Pos _lastStartPos;

    //maximum distance: used for obstacles
    double _maxDist;

    //! # neighbours: 4 or 8
    //int _neighbours;
    //! allow diagonal actions
    bool _allowDiagActions;

    //! map of nodes
    std::map<Pos,AStarNode*> _nodeMap;
};

#endif // ASTARPLANNER_H
