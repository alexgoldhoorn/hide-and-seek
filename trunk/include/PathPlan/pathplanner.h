#ifndef PATHPLANNER_H
#define PATHPLANNER_H

#include <list>
#include <string>

#include "HSGame/pos.h"


/*!
 * \brief The PathPlanner class plans a path between two points and can be used to calculate the distance between points
 */
class PathPlanner {
public:
    PathPlanner();

    virtual ~PathPlanner();

    /*!
     * \brief distance Calculate the distance between two points
     * NOTE: these are (x,y) points, when using rows and cols pass them as (col,row)
     * \param x1
     * \param y1
     * \param x2
     * \param y2
     * \return
     */
    virtual double distance(int x1, int y1, int x2, int y2)=0;

    /*!
     * \brief distance Calculate the distance between two points
     * \param p1
     * \param p2
     * \return
     */
    virtual double distance(const Pos& p1, const Pos& p2);

    /*!
     * \brief nextStep Get the next steps (nextX,nextY) towards the goal (x2,y2) from (x1,y1)
     * \param x1
     * \param y1
     * \param x2
     * \param y2
     * \param nextX [out] next X
     * \param nextY [out] next Y
     */
    virtual void nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY)=0;

    /*!
     * \brief nextStep Returns the next step from p1 towards the goal p2
     * \param p1
     * \param p2
     */
    virtual Pos nextStep(const Pos& p1, const Pos& p2);


    /*!
     * \brief doesCacheResults whether the class caches results
     * \return
     */
    virtual bool doesCacheResults()=0;

    /*!
     * \brief useContinuousSpace returns whether continuous space is used
     * \return
     */
    virtual bool useContinuousSpace();

    /*!
     * \brief setUseContinuousSpace set whether continuous space should be used
     * \param b
     */
    virtual void setUseContinuousSpace(bool b);

    //! get name
    virtual std::string getName()=0;

protected:
    bool _useContSpace;
};

/*
//A* path planner class: makes use of A* path planner of Joan P (IRI-UPC)
class AStarPathPlanner : public PathPlanner {
public:
    AStarPathPlanner(char** map, int rows, int cols);
    virtual ~AStarPathPlanner();

    virtual double distance(int x1, int y1, int x2, int y2);
    virtual void nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY);

    virtual bool doesCacheResults();

private:
    std::list<Node*>* getPath(int x1, int y1, int x2, int y2);
    void showPath(std::list<Node*>* path);

    bool sq_cl             ;
    bool lin_quad          ;
    int radius             ;
    double quad_factor      ;
    unsigned int max_obst  ;
    unsigned int heurist   ;
    unsigned int tie_break ;

    double robotClearance ;
    double maxRobotStep   ;
    double xFloor         ;
    double yFloor         ;


    MapGrid* _grid;

    AStar* _astar;
};*/


#endif // PATHPLANNER_H
