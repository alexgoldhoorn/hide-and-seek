#ifndef GMap_H
#define GMap_H

#include "../hsconfig.h"

#include "../PathPlan/pathplanner.h"

//#include <QPoint>

#include <string>

#include <cassert>

/*
map file:

1st line: parameters, int, seperated by comma
		  1st 2 params: #rows, #columns
		  <matrix of numbers, not seperated>
*/

/*! Position struct.
  AG-TODO: replace by existing QPoint or other, joining both.

  */
//AG130311: change x,y -> row,col
struct Pos {
    Pos(int row_,int col_) {
        this->row=row_;
        this->col=col_;
    }
    Pos() {
        row=col=-1;
    }

    int row;
    int col;


//    int& col;


    /*!
     * \brief toString return position as (x,y)
     * \return
     */
    string toString();

    /*!
     * \brief equals return true if passed position is the same
     * \param p
     * \return
     */
    bool equals(Pos p);

    /*!
     * \brief set set current pos equals to param pos
     * \param p
     */
    void set(Pos& p);

    /*!
     * \brief isSet returns if is set (-1 if not set)
     * \return
     */
    bool isSet();

    /*!
     * \brief clear clear value (-1,-1)
     */
    void clear();
};
/*! Position struct with extra info.
  AG-TODO: replace by existing QPoint or other
  */
struct PosAct {
    int a;
    int x;
    int y;
    int score;
};


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

    static const int MAX_ROWS           =500; //the max rows that the grid (the map) can have
    static const int MAX_COLS           =500; //the maximum number of columns.

    static const int DMAT_UNSET        = -2; //not yet set
    static const int DMAT_OBST         = -1; //obstacle in one/both location
    static const int DMAT_NO_PATH      = -3; //no path possible


    //! Create map with maximum size (MAX_ROWS,MAX_COLS).
    GMap();

    GMap(string fileName);

    //from file (AG120105)
    GMap(const char* fileName);


    GMap(int rows, int cols);


    //from file (AG120105)
    /*!
     * \brief GMap load PGM map file, and set base. A zoom out can be set meaning that cells will be grouped
     *  if 1 cell is an obstacle in the group, the zoomed out value will be an obstacle. if zoomOutFactor=1
     * it will be a 1 on 1 grouping, but if it is 2, groups of 2x2 will be taken.
     * \param pgmFileName
     * \param base
     * \param zoomOutFactor
     */
    GMap(const char* pgmFileName, Pos base, unsigned int zoomOutFactor=1);


    //AG130301: set map fixed, no changes to maps possible
    void setMapFixed();


    ~GMap();


    int rowCount() {
        return _rows;
    }
    int colCount() {
        return _cols;
    }
    int width() {
        return _cols;
    }
    int height() {
        return _rows;
    }

    Pos getBase() {
        return _base;
    }


    char getItem(int row, int col) {
        /*if (!(row>=0 && row<_rows || col>=0 && col<_cols))
                cout << " !"<<row<<","<<col<<"-max=("<<_rows<<","<<_cols<<")"<<endl;*/
        assert(row>=0 && row<_rows);
        assert(col>=0 && col<_cols);

        return _map[row][col];
    }

    //AG130228: disable
    /*
    char getVisibility(int row, int col) {
        return _visible[row][col];
    }

    void setVisible(int row, int col) {
        _visible[row][col] = 1;
    }

    void setInvisible(int row, int col) {
        _visible[row][col] = 0;
    }

    void InitVisible(int all=0);

    //AG120906: get # visible cells
    int getInvisibleCount();

    char** getVisibletab() {
        return _visible;
    }
    */



    void setrows(int r) {
        assert(!_mapFixed); //ag130301: not allowed to change map when set fixed
        _rows=r;
    }

    void setcols(int c) {
        assert(!_mapFixed); //ag130301: not allowed to change map when set fixed
        _cols=c;
    }
    void setbase() {
        assert(!_mapFixed); //ag130301: not allowed to change map when set fixed
        _map[_base.row][_base.col] = GMAP_BASE;
    }

    void setbase(int x, int y) {
        assert(!_mapFixed); //ag130301: not allowed to change map when set fixed
        _base.row=x;
        _base.col=y;
        _map[x][y]=GMAP_BASE;
    }

    void setBase(Pos base) {
        _base.set(base);
        _map[base.row][base.col] = GMAP_BASE;
    }

    void setInitialh(int x, int y) {        
        _initialhider.row = x;
        _initialhider.col = y;
    }

    void setInitials(int x, int y) {
        _initialseeker.row = x;
        _initialseeker.col = y;
    }

    Pos getInitialh() {
        return _initialhider;
    }

    Pos getInitials() {
        return _initialseeker;
    }

    void addObstacle(int x, int y) {
        assert(!_mapFixed); //ag130301: not allowed to change map when set fixed
        assert(x>=0 && x<_rows);
        assert(y>=0 && y<_cols);

        _map[x][y]=GMAP_OBSTACLE;
        //_visible[x][y]=2;
        _obstacleVector.push_back(Pos(x,y)); //ag130301: added obstacle
    }


    //AG130301: TODO: re-check
    void setHider(Pos p) {
        for(int r=0; r<_rows; r++) {
            for(int c=0; c<_cols; c++) {
                if(_map[r][c]==9)
                {
                    if(_base.row==r && _base.col==c)
                        _map[r][c]=GMAP_BASE;
                    else
                        _map[r][c]=GMAP_FREE_SPACE;
                }
            }
        }
        _map[p.row][p.col]=9;
    }

    void setSeeker(Pos p) {
        //if(_base.x==p.x && _base.y==p.y)
        //   return;
        for(int r=0; r<_rows; r++) {
            for(int c=0; c<_cols; c++) {
                if(_map[r][c]==8)
                {
                    if(_base.row==r && _base.col==c)
                        _map[r][c]=GMAP_BASE;
                    else
                        _map[r][c]=GMAP_FREE_SPACE;
                }
            }
        }
        _map[p.row][p.col]=8;
    }

    void deleteItem(int x, int y) {
        if (_base.row==x &&_base.col==y)
            _map[x][y]=GMAP_BASE;
        else
            _map[x][y]=GMAP_FREE_SPACE;
    }


    //TODO: x,y coords starting from low-left

    /*!
     * Print map, with or without visibility map.
     * \brief printMap
     * \param viewR row viewer (-1: not used)
     * \param viewC col viewer
     */
    void printMap(int viewR=-1, int viewC=-1); //bool useVisib=false);


    void printMap(Pos p) {
        printMap(p.row,p.col);
    }
    void printMap(Pos seekerPos, Pos hiderPos);


    //AG130301: TODO CHECK
    //AG130311: disabled
    ///void xcoChangeMap(Pos cur, Pos neo);



    //Create path planner (used by distance)
    void createAStarPathPlanner();
    //Path planner using propogation
    void createPropDistPlanner();

    //get distance of path
    //float distance(int x1,int y1,int x2,int y2);
    //AG120425: moved to row-col use!!
    float distance(int r1,int c1,int r2,int c2);

    float distance(Pos &p1, Pos &p2);

    // next node in path
    //AG120425: moved to row-col use!!
    //QPoint nextStep(int r1,int c1,int r2,int c2);
    //QPoint nextStep(int x1, int y1, int x2, int y2);
    Pos nextStep(int r1,int c1,int r2,int c2);

    //! count number of free (i.e. non-obstacle) cells
    int numFreeCells();

    //AG120424
    // Count # obstacles
    // ag130304 -> numObstacles
    //int numtObstacles();


    //get the index of the state (used by APPL in its POMDP and policy file)
    //ag130304: use getCoordFromIndex
    //int getPosIndex(Pos pos);
    //int getPosIndex(int r, int c);

    //returns if pos visible
    /*bool isVisible(Pos pos);
    bool isVisible(int r, int c);*/

    //ag120227: returns if is obstacle
    bool isObstacle(Pos pos);
    bool isObstacle(int r, int c);

    //ag120418: check if is base
    bool isBase(int r, int c);

    //ag130228: disabled
    //count invisible cells (for hider)
    //int invisibleCells();


    /*!
     * \brief getIndexFromCoord @see getIndexFromCoord
     * \param p
     * \return
     */
    int getIndexFromCoord(Pos p) {
        return getIndexFromCoord(p.row,p.col);
    }


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
    Pos getObstacle(int i) { //get the pos of the "i" obstacle
        return _obstacleVector[i];//ag130301: changed to vector //_obstacles[i];
    }

    //! Count # obstacles
    int numObstacles() {
        return _obstacleVector.size(); //ag130301: changed to vector //_obstnum;
    }


    //ag130227: is visible for a matrix
    /*!
     * \brief isVisible  checks if (r1,c1) is visible from (r2,c2) and vica versa
     * \param r1
     * \param c1
     * \param r2
     * \param c2
     * \return is visible
     */
    bool isVisible(int r1, int c1, int r2, int c2);

    /*!
     * \brief isVisible check is p1 is visible from p2 and vica versa
     * \param p1
     * \param p1
     * \return is visible
     */
    bool isVisible(Pos p1, Pos p2);

    //ag130301
    /*!
     * \brief getInvisiblePoints get invisible points seen from (r,c)
     * \param r
     * \param c
     * \return
     */
    vector<Pos> getInvisiblePoints(int r, int c);


    /*!
     * \brief getInvisiblePoints get the invisible points seen from p
     * \param p
     * \return
     */
    vector<Pos> getInvisiblePoints(Pos p);


    //AG120416: added doInit
    void createMap(int rows, int cols, bool doInit=true);

    //read map from file
    void readMapFile(const char* fileName);

    void readPGMMapFile(const char* pgmFileName, Pos base, unsigned int zoomOutFactor) ;

    void writeMapFile(const char *fileName);

    /*!
     * \brief isPosInMap Check if pos is in map
     * \param pos
     * \return
     */
    bool isPosInMap(Pos& pos);

    /*!
     * \brief isPosInMap check if pos is in map
     * \param r
     * \param c
     * \return
     */
    bool isPosInMap(int r, int c);


    // unit tests
    //! test if visiblity(x,y)==visibility(y,x)
    //! if true-> we can put it in the matrix
    void testVisibility();
    //! distance test
    void testDistance();

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


    //the map, see codes GMAP_*
    char** _map;

    //ag130228: disabled
    //char** _visible; //9 for undefined cells, 0 for invisible cells, 1 for visible cells, 2 for obstacles

    //ag120227: map of idnex in vector (ie ignoring obstacles)
    int** _indexMap;
    //map size
    int _rows, _cols;
    //base pos
    Pos _base;
    //pos hider/seeker at init
    Pos _initialhider;
    Pos _initialseeker;

    //AG111125
    PathPlanner* _pathPlanner;

    //ag120521: for server
    //Pos *_obstacles;
    //int _obstnum;
    //AG130301: changed to vector
    vector<Pos> _obstacleVector;

    //AG130227: visiblity matrix of all combis of pos, uses VMAT_
    char**** _visibleMat;

    //AG130301: map fixed, no change possible
    bool _mapFixed;

    //AG130301: num free cells    ag130304 -> use numObstacles since it is vector
    //int _numFreeCells;

    //index to pos
    vector<Pos> _coordVector;


    //AG130509: buffer for distances
    float**** _distMat;
};
//}



#endif
