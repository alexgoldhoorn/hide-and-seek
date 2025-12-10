#ifndef PATHPLANNER_H
#define PATHPLANNER_H

//#include <QPoint>
#include "map_grid/map_grid.h"
#include "astar/astar.h"
#include <list>

using namespace map_grid;
using namespace astar;
using namespace std;

/*!
 * \brief The PathPlanner class plans a path between two points and can be used to calculate the distance between points
 */
class PathPlanner {
public:
    /*!
     * \brief distance Calculate the distance between two points
     * \param x1
     * \param y1
     * \param x2
     * \param y2
     * \return
     */
    virtual float distance(int x1, int y1, int x2, int y2)=0;

    /*!
     * \brief nextStep get the next steps
     * \param x1
     * \param y1
     * \param x2
     * \param y2
     * \param nextX
     * \param nextY
     */
    virtual void nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY)=0;

    /*!
     * \brief doesCacheResults whether the class caches results
     * \return
     */
    virtual bool doesCacheResults()=0;
};

//A* path planner class: makes use of A* path planner of Joan P (IRI-UPC)
class AStarPathPlanner : public PathPlanner {
public:
    AStarPathPlanner(char** map, int rows, int cols);
    ~AStarPathPlanner();

    virtual float distance(int x1, int y1, int x2, int y2);
    virtual void nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY);

    virtual bool doesCacheResults() {
        return false;
    }

private:
    std::list<Node*>* getPath(int x1, int y1, int x2, int y2);
    void showPath(std::list<Node*>* path);

    bool sq_cl             ;
    bool lin_quad          ;
    int radius             ;
    float quad_factor      ;
    unsigned int max_obst  ;
    unsigned int heurist   ;
    unsigned int tie_break ;

    float robotClearance ;
    float maxRobotStep   ;
    float xFloor         ;
    float yFloor         ;


    MapGrid* _grid;

    AStar* _astar;
};

//path planner using distance propogation from goal.
//warning: not very efficient
//      only efficient part is that it doesn't recalculate distance map when the same goal is given
class PropDistPlanner : public PathPlanner {
public:
    PropDistPlanner(char** map, int rows, int cols, int neighbours=8);
    ~PropDistPlanner();

    //returns distance, or -1 if no path possible (e.g. goal/init is obstacle)
    virtual float distance(int x1, int y1, int x2, int y2);
    virtual void nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY);

    virtual bool doesCacheResults() {
        return true;
    }

    void printDistMap();
protected:
    //AG130617: moved to rowgoal and colgoal (was x,y)
    void setMapForGoal(int rg, int cg);

    //! init dist map
    void initDistMap();
private:
    //map (like in GMap)
    char** _map;
    //size map
    int _cols;
    int _rows;

    //distance map for last goal
    //each distance at obstacle is: _maxDist
    int**** _distMap;
    //last goal (corresponds to current _distMap)
    int _lastGoalX;
    int _lastGoalY;
    //maximum distance: used for obstacles
    int _maxDist;
    //# neighbours: 4 or 8
    int _neighbours;
};

#endif // PATHPLANNER_H
