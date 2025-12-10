#ifndef GMap_H
#define GMap_H

#include "hsconfig.h"

#include "PathPlan/pathplanner.h"

#include <string>
#include <vector>
#include <cmath>

#include "HSGame/pos.h"
#include "HSGame/idpos.h"
#include "seekerhsparams.h"


//namespace HSGame {
/*! This class defines the map of the game.
Maps can be loaded using a text file, with the following format:
-------------
cols,rows,
100000
000020
100000
-------------
0=free,1=obstacle,2=base (only 1 allowed)

The map can also be built without file: defining the size, and then adding the base and obstacles.

The class offers options to check the visibility of players in two points (assuming that obstacles completely oclude anything behind it).
Also the distance between two points can be calculated using a path planner.

  AG NOTE/TODO:
  - requires refactoring!!!!
  - more consistent naming
  - more efficiencyf
  - ....
  */
class GMap {
public:
    //constants which define the IDs of the map items
    static const int GMAP_FREE_SPACE    = 0;
    static const int GMAP_OBSTACLE      = 1;
    static const int GMAP_BASE          = 2;
    static const int GMAP_SEEKER        = 8; //10; //not definable on map
    static const int GMAP_HIDER         = 9; //11; //idem

    //ag130228: disabled
   // static const int VMAP_UNDEFINED     =9; //undefined for the visibility map //OLD code chryso
   // static const int VMAP_OBST          =2; //obstacle for the visibility map  //OLD code chryso!!

    //AG140521: set this maximum because of the size of the buffer of distances, which is a 4-dimensional matrix of
    //          rows x cols x rows x cols, with 181 rows and cols this results in 2 GiB
    static const int MAX_ROWS           = 181; //10000; //the max rows that the grid (the map) can have //max int: 32767 (?)
    static const int MAX_COLS           = 181; //10000; //the maximum number of columns.

    static constexpr float DMAT_UNSET        = -2; //not yet set
    static constexpr float DMAT_OBST         = -1; //obstacle in one/both location
    static constexpr float DMAT_NO_PATH      = -3; //no path possible

    //straight distance for a diagonal distance of 1 (i.e. x and y distance
    //depricated, use: M_SQRT1_2
    //static const double SQRT_0_5 = 0.707106781;  //sqrt(0.5);
    //distance of diagonal if x and y movements are 1
    //depricated, use: M_SQRT2
    //static const float SQRT_2 = 1.414213562; //sqrt(2);


    //! Create map with maximum size (MAX_ROWS,MAX_COLS).
    GMap(SeekerHSParams* params);

    GMap(std::string fileName,SeekerHSParams* params);

    //from file (AG120105)
    //GMap(const char* fileName,SeekerHSParams* params);


    GMap(int rows, int cols,SeekerHSParams* params);


    //from file (AG120105)
    /*!
     * \brief GMap load PGM map file, and set base. A zoom out can be set meaning that cells will be grouped
     *  if 1 cell is an obstacle in the group, the zoomed out value will be an obstacle. if zoomOutFactor=1
     * it will be a 1 on 1 grouping, but if it is 2, groups of 2x2 will be taken.
     * \param pgmFileName
     * \param base
     * \param zoomOutFactor
     */
    GMap(std::string pgmFileName, Pos base, unsigned int zoomOutFactor,
         SeekerHSParams* params=NULL);

    /*!
     * \brief GMap
     * \param yamlFileName
     * \param base
     * \param cellSize
     * \param rowDist_m
     * \param colDist_m
     * \param params
     */
    GMap(std::string yamlFileName, Pos base, double cellSize, double rowDist_m, double colDist_m,
         SeekerHSParams* params=NULL);




    void setParams(SeekerHSParams* params);

    SeekerHSParams* getParams() const;

    //AG130301: set map fixed, no changes to maps possible
    void setMapFixed();

    bool isMapFixed() const;

    ~GMap();


    int rowCount() const;

    int colCount() const;

    int width() const;

    int height() const;

    Pos getBase() const;

    char getItem(int row, int col) const;

    void setItem(int row, int col, char c);

    void setBase(int x, int y);

    void setBase(const Pos& base);

    void addObstacle(int x, int y);

    void addObstacle(const Pos& p);

    void deleteItem(int x, int y);


    //TODO: x,y coords starting from low-left

    /*!
     * Print map, with or without visibility map.
     * \brief printMap
     * \param viewR row viewer (-1: not used)
     * \param viewC col viewer
     */
    void printMap(bool showDynObst=true, int sr=-1, int sc=-1, int hr=-1, int hc=-1); //bool useVisib=false);


    void printMap(const Pos& p, bool showDynObst=true);
    void printMap(const Pos& seekerPos, const Pos& hiderPos, bool showDynObst=true);


    //AG130301: TODO CHECK
    //AG130311: disabled
    ///void xcoChangeMap(Pos cur, Pos neo);



    //Create path planner (used by distance)
    //void createAStarPathPlanner();
    //Path planner using propogation
    void createPathPlanner();

    //get distance of path
    //float distance(int x1,int y1,int x2,int y2);
    //AG120425: moved to row-col use!!
    /*!
     * \brief distance shortest path distance
     * \param r1
     * \param c1
     * \param r2
     * \param c2
     * \return
     */
    double distance(int r1,int c1,int r2,int c2);

    /*!
     * \brief distance shortest path distance
     * \param p1
     * \param p2
     * \return
     */
    double distance(const Pos& p1, const Pos& p2);


    /*!
     * \brief distanceCont shortest path distance in continuous space
     * \param r1
     * \param c1
     * \param r2
     * \param c2
     * \return
     */
    double distanceCont(double r1, double c1, double r2, double c2);

    /*!
     * \brief distanceCont shortest path distance in continuous space
     * \param p1
     * \param p2
     * \return
     */
    double distanceCont(const Pos &p1, const Pos &p2);


    /*!
     * \brief distanceEuc eucledian distance
     * \param p1
     * \param p2
     * \return
     */
    double distanceEuc(const Pos &p1, const Pos &p2) const;


    /*!
     * \brief distanceEuc Eucledian distance
     * \param r1
     * \param c1
     * \param r2
     * \param c2
     * \return
     */
    static constexpr double distanceEuc(double r1, double c1, double r2, double c2) {
        return sqrt(pow(r1-r2,2) + pow(c1-c2,2));
    }

    // next node in path
    //AG120425: moved to row-col use!!
    //QPoint nextStep(int r1,int c1,int r2,int c2);
    //QPoint nextStep(int x1, int y1, int x2, int y2);
    //AG140109: not used anymore
    //Pos nextStep(int r1,int c1,int r2,int c2);

    //! count number of free (i.e. non-obstacle) cells
    int numFreeCells();


    //ag120227: returns if is obstacle
    bool isObstacle(const Pos& pos) const;
    bool isObstacle(int r, int c) const;
    bool isObstacle(double r, double c) const;

    //ag120418: check if is base
    bool isBase(int r, int c) const;


    bool isBase(const Pos& p) const;

    //ag130228: disabled
    //count invisible cells (for hider)
    //int invisibleCells();


    /*!
     * \brief getIndexFromCoord @see getIndexFromCoord
     * \param p
     * \return
     */
    int getIndexFromCoord(const Pos& p);


    //ag120227: get index from r,c coordinate
    /*!
     * \brief getIndexFromCoord  get the index from the coordinates, obstacles are skipped and return -1
     * \param r
     * \param c
     * \return
     */
    int getIndexFromCoord(int r, int c);


    //AG130304: get coord from index
    /*!
     * \brief getCoordFromIndex get the coordinate from the index, going from left-right top-bottom, skipping obstacles
     * \param i
     * \return
     */
    Pos getCoordFromIndex(int i);

    //ag120521: for server
    Pos getObstacle(int i) const;

    //! Count # obstacles
    int numObstacles() const;


    //ag130227: is visible for a matrix
    /*!
     * \brief isVisible  checks if (r1,c1) is visible from (r2,c2) and vica versa
     * \param r1
     * \param c1
     * \param r2
     * \param c2
     * \param checkDynObst use dyn. obst. to check visibility
     * \return is visible
     */
    bool isVisible(int r1, int c1, int r2, int c2, bool checkDynObst);

    /*!
     * \brief isVisible checks if (r1,c1) is visible from (r2,c2) and vica versa (uses discrete locations)
     * \param r1
     * \param c1
     * \param r2
     * \param c2
     * \param checkDynObst
     * \return
     */
    bool isVisible(double r1, double c1, double r2, double c2, bool checkDynObst);

    /*!
     * \brief isVisible check is p1 is visible from p2 and vica versa
     * \param p1
     * \param p1
     * \param checkDynObst
     * \return is visible
     */
    bool isVisible(const Pos& p1, const Pos& p2, bool checkDynObst);

    //ag130301
    /*!
     * \brief getInvisiblePoints get invisible points seen from (r,c)
     * \param r
     * \param c
     * \return
     */
    std::vector<Pos> getInvisiblePoints(int r, int c, bool checkDynObst);

    /*!
     * \brief getInvisiblePoints get points which are not visible from r1,c1, nor from r2,c2
     * \param r1
     * \param c1
     * \param r2
     * \param c2
     * \param checkDynObst
     * \return
     */
    std::vector<Pos> getInvisiblePoints(int r1, int c1, int r2, int c2, bool checkDynObst);

    /*!
     * \brief getInvisiblePoints get the invisible points seen from p
     * \param p
     * \return
     */
    std::vector<Pos> getInvisiblePoints(const Pos& p, bool checkDynObst);

    //AG150126
    /*!
     * \brief getInvisiblePoints get the points which are not visible from p1 nor from p2
     * \param p1
     * \param p2
     * \param checkDynObst
     * \return
     */
    std::vector<Pos> getInvisiblePoints(const Pos& p1, const Pos& p2, bool checkDynObst);


    //AG120416: added doInit
    void createMap(int rows, int cols, bool doInit=true);

    //read map from file
    void readMapFile(std::string fileName);

    /*!
     * \brief readPGMMapFile reads the PGM file and constructs a map from it. If using zoomOutFactor>1, a square of cells in the map
     * is grouped, if there is one with an obstacle, it is taken as an obstacle. The (beginRow,beginCol)-(endRow,endCol) defines the
     * area which is used from the map. By default these coordinates are 0 and in that case the left-top and right-bottom are searched
     * which define the smallest region that contains all empty (white) and obstacles (black).
     *
     * \param pgmFileName the PGM file
     * \param base a base
     * \param zoomOutFactor the size of the square of cells that are used to calculate whether it is an obstacle
     * \param beginRow
     * \param beginCol
     * \param endRow
     * \param endCol
     */
    void readPGMMapFile(std::string pgmFileName, const Pos& base, unsigned int zoomOutFactor,
                        unsigned int beginRow=0, unsigned int beginCol=0, unsigned int endRow=0, unsigned int endCol=0);

    /*!
     * \brief readYAMLMapFile Reads the YAML file that contains the PGM, and meta data: resolutino and origin.
     * (rowDist_m,colDist_m) is the size of the rectangle of the map used in the map, unit: meters.
     * \param yamlFileName
     * \param base a base
     * \param zoomOutFactor the size of the square of cells that are used to calculate whether it is an obstacle,
     * \param rowDist_m
     * \param colDist_m
     */
    void readYAMLMapFile(std::string yamlFileName, const Pos& base, double cellSize,
                        double rowDist_m=0, double colDist_m=0);

    /*!
     * \brief writeMapFile write the map to a file, map format
     * \param fileName
     */
    void writeMapFile(std::string fileName);

    /*!
     * \brief isPosInMap Check if pos is in map
     * \param pos
     * \return
     */
    bool isPosInMap(const Pos& pos) const;

    /*!
     * \brief isPosInMap check if pos is in map
     * \param r
     * \param c
     * \return
     */
    bool isPosInMap(int r, int c) const;

    /*!
     * \brief isPosInMap check if pos is in map
     * \param r
     * \param c
     * \return
     */
    bool isPosInMap(double r, double c) const;


    // unit tests
    //! test if visiblity(x,y)==visibility(y,x)
    //! if true-> we can put it in the matrix
    void testVisibility();
    //! distance test
    void testDistance();

    //! set edit mode
    void setEditMode(bool b);

    //! edit mode
    bool getEditMode() const;

    //AG131204
    /*!
     * \brief tryMove try moving from the current position, doing the action; if the action is not possible then a not set Pos is returned (-1,-1)
     * \param action 0-9: action
     * \param pos the start pos     
     * \return the new pos
     */
    Pos tryMove(int action, Pos pos);

    /*!
     * \brief tryMoveDir try moving from the current position in the given direction; returns not set Pos if action not possible
     * \param dir the direction: 0-2pi rad, clockwise with 0 rad being north
     * \param pos the start pos
     * \param dist the distance (grid cells, default 1)
     * \return the new pos
     */
    Pos tryMoveDir(double dir, Pos pos, double dist, bool visibCheck);

    /*!
     * \brief tryMoveDirStep try moving from the current position in the given direction. If moving is not possible the distance (dist) is reduced with
     * the stepSize. If the move is not possible at all, the same position is returned.
     * The a visibility check of the point to navigate to is done iv visibCheck is true, otherwise it is ignored. This can be the case
     * when a real observation does show a certain point while the raytrace said it was not visible.
     * \param dir
     * \param pos
     * \param dist
     * \param stepSize
     * \param visibCheck
     * \return
     */
    Pos tryMoveDirStep(double dir, Pos pos, double dist, double stepSize, bool visibCheck);

    /*!
     * \brief getDirection direction from p1 to p2
     * \param p1
     * \param p2
     * \return direction in rad from 0-2pi, clockwise with 0 rad being north
     */
    double getDirection(const Pos& p1, const Pos& p2) const;

    /*!
     * \brief getNextStep get the next step from p1 to p2, uses the PathPlanner
     * \param p1
     * \param p2
     * \return
     */
    Pos getNextStep(const Pos& p1, const Pos& p2);

    /*!
     * \brief getName name of the map
     * \return
     */
    std::string getName() const;


    //bool usingContActions() const;


    /*!
     * \brief setUsingContActions set using continuous actions refers to using doubles as position and diagonal movements of also distance 1.0
     * \param b
     */
    void setUsingContActions(bool b);

    /*!
     * \brief genRandomPos generates a random pos in the map
     * \return
     */
    Pos genRandomPos() const;


#ifdef GMAP_CAN_DELETE
    void deleteRows(int startRow, int endRow);

    void deleteCols(int startCol, int endCol);
#endif


#ifdef DYN_OBST_COLL
    /*!
     * \brief tooCloseToDynObst checks if the point p is closer than (using eucledian distance) minDist to any dynamic obstacle
     * \param pos
     * \return
     */
    bool tooCloseToWithDynObst(const Pos& pos) const;
#endif


#ifdef USE_QT
    //AG131211: functions to read/write map from stream in GMap to make them equal for all

    /*!
     * \brief readMapFromStream reads map from stream
     * \param in
     */
    void readMapFromStream(QDataStream& in);

    /*!
     * \brief writeMaptoStream writes map to stream
     * \param out
     */
    void writeMaptoStream(QDataStream& out);
#endif


    //! set dynamic obstacles
    void setDynamicObstacles(const std::vector<IDPos>& dynObstVec);

    //! add dyn obst
    void addDynamicObstacle(IDPos p);

    //! return dynamic obstacles
    std::vector<IDPos> getDynamicObstacles() const;

    //! remove dynamic obstacles
    void removeDynamicObstacles();

    //! get the pathplanner
    PathPlanner* getPathPlanner();

    /*!
     * \brief writeDistanceMatrixToFile
     * \param fileName
     */
    void writeDistanceMatrixToFile(std::string fileName);

    /*!
     * \brief readDistanceMatrixToFile read the distance matrix file, and store it in the local distance matrix
     * \param fileName
     */
    void readDistanceMatrixToFile(std::string fileName);
protected:

    static const char VMAT_UNSET        = 0; //not yet set
    static const char VMAT_VIS          = 1; //visible
    static const char VMAT_NOT_VIS      = 2; //not visible
    static const char VMAT_OBST         = 3; //obstacle in one/both location

    static const int PGM_NO_EXPLORED_GRAY = 205;




    char raytrace(char **m, int x0, int y0, int x1, int y1);

    //AG130227: init visibility matrix dependend on map
    void initVisMat();

    //init map to 0s
    void initMap();

    //AG0509: init dist map
    void initDistMat();


    //ag120227: generate a map (2D matrix) with the index of each location in a vector
    // i.e. ignoring obstacles
    void createIndexMap();




private:
    //!the map, see codes GMAP_*
    char** _map;

    //! map of idnex in vector (ie ignoring obstacles)
    int** _indexMap;
    //! map size
    int _rows, _cols;
    //! base pos
    Pos _base;
    //! path planner to calculate the shortest path
    PathPlanner* _pathPlanner;
    //! list of obstacles
    std::vector<Pos> _obstacleVector;
    //! visiblity matrix of all combis of pos, uses VMAT_
    char**** _visibleMat;

    //! visib matrix
    char** _visibleMatDynObst;

    //! used for visible matrix dyn. obst., when the same we can reuse the visible matrix;
    //! also used as boolean to check if dyn. obst. changed
    Pos _lastVisibOrigPos;

    //! map fixed, no change possible
    bool _mapFixed;

    //! index to pos
    std::vector<Pos> _coordVector;

    //! buffer for distances
    float**** _distMat;

    //! buffer to store  which action brings it to what position
    Pos*** _actionsPosMat;

    //! edit mode of map
    bool _editMode;

    //! name of map
    std::string _name;

    //! use continuous actions
    //bool _useContActions;

    //! vector of dynamic obstacles / walkers
    std::vector<IDPos> _dynObstVector;

    //! params
    SeekerHSParams* _params;
};
//}



#endif
