#ifndef PROPDISTPLANNER_H
#define PROPDISTPLANNER_H

#include "PathPlan/pathplanner.h"

/*!
 * \brief The PropDistPlanner class distance calculation by propogating the distance in the map until it does not change anymore.
 * It buffers the results, and for each new location the distance from all positions to a certain goal is calculated (and stored in buffer).
 *
 * Complexity
 * Space complexity (of buffer): Θ(n^2)       (n=#rows*#cols)
 * @see setMapForGoal
 *
 */
class PropDistPlanner : public PathPlanner {
public:
    PropDistPlanner(char** map, int rows, int cols, int neighbours=8);
    virtual ~PropDistPlanner();

    /*!
     * \brief distance returns distance, or -1 if no path possible (e.g. goal/init is obstacle).
     * Complexity:
     * if set before (i.e. read from buffer): O(1),
     * otherwise @see setMapForGoal()
     * \param x1
     * \param y1
     * \param x2
     * \param y2
     * \return
     */
    virtual double distance(int x1, int y1, int x2, int y2);

    //! calculate the next step from (x1,y1) to goal (x2,y2), return (nextX,nextY)
    //! NOTE: for eficiency it is very important to keep (x2,y2) fixed, and let (x1,x2) be the current location
    virtual void nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY);

    virtual bool doesCacheResults();

    void printDistMap();


    virtual std::string getName();

protected:
    /*!
     * \brief setMapForGoal calculates the distances for the given goal (rg,cg) from each point on the map by scanning through the distance
     * map (initialized with unknown) and finding the smallest neighbour value (distance to goal) and then set the value of the current cell to
     * that distance plus the distance to that neighbour. This process is repeated until no changes occur.
     * Complexity:
     * In best case the goal is at the left-top corner -> Ω(n)
     * and worst case at the right-bottom -> O(n^2)
     *
     * \param rg
     * \param cg
     */
    void setMapForGoal(int rg, int cg);

    /*!
     * \brief initDistMap initilize the distance map (flags all obstacles, and flags other cells as 'not calculated')
     * Complexity: Θ(n^2)
     */
    void initDistMap();
private:
    //! map (like in GMap)
    char** _map;
    //! number of columns
    int _cols;
    //! number of rows
    int _rows;

    //distance map for last goal
    //each distance at obstacle is: _maxDist
    //AG140120: int -> double
    //! four dimensional distance map (buffer)
    double**** _distMap;

    //! last X goal (corresponds to current _distMap)
    int _lastGoalX;
    //! last Y goal (corresponds to current _distMap)
    int _lastGoalY;

    //! maximum distance: used for obstacles
    double _maxDist;
    //! # neighbours: 4 or 8
    int _neighbours;
};
#endif // PROPDISTPLANNER_H
