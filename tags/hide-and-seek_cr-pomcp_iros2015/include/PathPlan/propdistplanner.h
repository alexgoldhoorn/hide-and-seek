#ifndef PROPDISTPLANNER_H
#define PROPDISTPLANNER_H

#include "PathPlan/pathplanner.h"

class PropDistPlanner : public PathPlanner {
public:
    PropDistPlanner(char** map, int rows, int cols, int neighbours=8);
    virtual ~PropDistPlanner();

    //returns distance, or -1 if no path possible (e.g. goal/init is obstacle)
    virtual double distance(int x1, int y1, int x2, int y2);

    //! calculate the next step from (x1,y1) to goal (x2,y2), return (nextX,nextY)
    //! NOTE: for eficiency it is very important to keep (x2,y2) fixed, and let (x1,x2) be the current location
    virtual void nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY);

    virtual bool doesCacheResults();

    void printDistMap();


    virtual std::string getName();

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
    //AG140120: int -> double
    double**** _distMap;
    //last goal (corresponds to current _distMap)
    int _lastGoalX;
    int _lastGoalY;
    //maximum distance: used for obstacles
    double _maxDist;
    //# neighbours: 4 or 8
    int _neighbours;
};
#endif // PROPDISTPLANNER_H
