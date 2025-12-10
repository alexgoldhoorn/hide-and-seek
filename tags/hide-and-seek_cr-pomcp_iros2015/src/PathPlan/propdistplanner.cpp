#include "PathPlan/propdistplanner.h"

#include <vector>
#include <algorithm>
#include <assert.h>

#include "exceptions.h"
#include "HSGame/gmap.h"
#include "Utils/generic.h"

#include <limits>
#include <iostream>

//AG140527: REQUIRED 'using namespace std' for std::numeric_limits<double> (see GMap.cpp comment)
using namespace std;

// ----- ag propogated distance planner

void PropDistPlanner::initDistMap() {
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


PropDistPlanner::PropDistPlanner(char **map, int rows, int cols, int neighbours) : PathPlanner() {
    _map = map;
    _rows = rows;
    _cols =cols;
    _neighbours = neighbours;
    _lastGoalX = _lastGoalY = -1;
    _maxDist = rows * cols;
    /*_distMap = new int*[_rows]; //AllocateDynamicArray<int>(2*_cols,_rows);
    for(int r=0;r<_rows;r++) _distMap[r] = new int[_cols];*/
    //AG120416: use of allocatedynamic array
    //AG130617: update, use a matrix for all idstances
    //_distMap = AllocateDynamicArray<int>(_rows,_cols);
    initDistMap();
}

PropDistPlanner::~PropDistPlanner(){
    FreeDynamicArray<double>(_distMap, (unsigned int)_rows,(unsigned int)_cols,(unsigned int)_rows);
}


/*TODO TRY -> segmentation fault!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
build/release/momdp/hsmomdp -tm -sc -A 0 -a ~/MyProjects/Experiments/maps/map3_6x5.txt*/

void PropDistPlanner::setMapForGoal(int rg, int cg) {

    if (_lastGoalX==cg && _lastGoalY==rg) return; //already set for this goal

    //init distance map
    //TODO: chekc if goal is obstacle
    /*for(int r=0;r<_rows;r++) {
        for(int c=0;c<_cols;c++) {

            if (y==r && x==c) {
                //TODO: check if is obstacle
                _distMap[x][y][r][c] = 0;
            } else if (_map[r][c] == GMap::GMAP_OBSTACLE) {
                _distMap[x][y][r][c] = _maxDist;
            } else {
                _distMap[x][y][r][c] = -1; //to calculate
            }
        }
    }*/

    _distMap[rg][cg][rg][cg] = 0;

    //loop all cells until all calculated
    bool allVisited = false;

    while (!allVisited) {
        allVisited = true;

        for(int r=0;r<_rows;r++) {
            for(int c=0;c<_cols;c++) {

                if (_distMap[rg][cg][r][c]==GMap::DMAT_UNSET) { //not yet calculated
                    double minDist = _maxDist;
                    bool allNeighbObst = true;

                    //TODO: create bool to flag if previous action (minDist) is from diagonal
                    //          if from diagonal -> add sqrt(2) instead of 1
                    bool fromDiag = false;

                    //now loop neighbours
                    for(int r1=r-1;r1<=r+1;r1++) {
                        for(int c1=c-1;c1<=c+1;c1++) {
                            if (/*r1!= r && c1!=c &&*/ r1>=0 && r1<_rows && c1>=0 && c1<_cols && (_neighbours==8 || r1==r || c1==c) ) {
                                //coords within range and fit to either 8-neighbours (all cases) or 4-neighbours (when either same row or same col as orig)


                                double d = _distMap[rg][cg][r1][c1];
                                //set min dist
                                if (d>=0 && d < minDist) { //d<MAX_DIST &&
                                    minDist = d;
                                    fromDiag = (r!=r1 && c!=c1);
                                }
                                if (d!=GMap::DMAT_OBST) allNeighbObst = false;
                            }
                        }
                    }
                    if (minDist == _maxDist) {
                        if (allNeighbObst) {
                            //unreachable
                            _distMap[rg][cg][r][c] = GMap::DMAT_NO_PATH; //_maxDist;
                        } else {
                            //all neighbours still unititialized
                            allVisited = false;
                        }
                        //allVisited = false;
                    } else {
                        //found minimum distance
                        _distMap[rg][cg][r][c] = minDist + (fromDiag && _useContSpace ? M_SQRT2 : 1.0); //TODO SAME BELLOW
                    }

                } // if distMat == unset
            }
        }
    } // while !allVisited

    //AG130614 FIX: check if the distances are minimum, try and adjust it untill all no distance is changed anymore

    //now check if minimum
    allVisited = false;
    while (!allVisited) {
        allVisited = true;

        for(int r=0;r<_rows;r++) {
            for(int c=0;c<_cols;c++) {

                if (_distMap[rg][cg][r][c]>=0) { //calculated, but not obst/unreachable
                    double minDist = _maxDist;
                    //bool allNeighbObst = true;

                    bool fromDiag = false;

                    //now loop neighbours
                    for(int r1=r-1;r1<=r+1;r1++) {
                        for(int c1=c-1;c1<=c+1;c1++) {
                            if (/*r1!= r && c1!=c &&*/ r1>=0 && r1<_rows && c1>=0 && c1<_cols && (_neighbours==8 || r1==r || c1==c) ) {
                                //coords within range and fit to either 8-neighbours (all cases) or 4-neighbours (when either same row or same col as orig)

                                double d = _distMap[rg][cg][r1][c1];
                                //set min dist
                                if (d>=0 && d+std::numeric_limits<double>::epsilon() < minDist) { //d<MAX_DIST &&
                                    minDist = d;
                                    fromDiag = (r!=r1 && c!=c1);
                                }
                                //if (d < _distMap[r][c]-1 /*_maxDist*/) allNeighbObst = false;
                            }
                        }
                    }

                    double stepDist = (fromDiag && _useContSpace ? M_SQRT2 : 1.0);
                    if (minDist+stepDist+std::numeric_limits<double>::epsilon() < _distMap[rg][cg][r][c]) {
                        _distMap[rg][cg][r][c] = minDist + stepDist;
                        allVisited = false;
                    }


                } //if not obst/unreachable
            }
        }
    }

    //AG130617: now store the opposite distance
    for(int r=0;r<_rows;r++) {
        for(int c=0;c<_cols;c++) {
            _distMap[r][c][rg][cg] = _distMap[rg][cg][r][c];
        }
    }


    _lastGoalX = cg;
    _lastGoalY = rg;
}

//AG WARNING: HERE X-Y is COL-ROW !!!
double PropDistPlanner::distance(int c1, int r1, int c2, int r2){
    //assert pos give are not obstacles
    assert(c1>=0 && c1<_cols && r1>=0 && r1<_rows && r2>=0 && r2<_rows && c2>=0 && c2<_cols);
    double d = -2;
    if (_distMap[r1][c1][r2][c2]==GMap::DMAT_OBST || _distMap[r1][c1][r2][c2]==GMap::DMAT_NO_PATH)  {
        d = -1; //no path
    } else {
        if (_distMap[r1][c1][r2][c2]==GMap::DMAT_UNSET) {
            setMapForGoal(r2,c2);
        }

        d = _distMap[r1][c1][r2][c2];
    }

    return d;
}

void PropDistPlanner::nextStep(int x1, int y1, int x2, int y2, int* nextX, int* nextY) {
    //set goal (if not set)
    setMapForGoal(y2,x2); //AG140518: this was still in the wrong order (col,row): x2,y2);

    //QPoint next(x1,y1);
    *nextX = x1;
    *nextY = y1;
    double minDist = _distMap[y1][x1][y2][x2];

    //now loop neighbours
    for(int r1=y1-1;r1<=y1+1;r1++) {
        for(int c1=x1-1;c1<=x1+1;c1++) {
            if (r1>=0 && r1<_rows && c1>=0 && c1<_cols && (_neighbours==8 || r1==y1 || c1==x1) ) {
                //coords within range and fit to either 8-neighbours (all cases) or 4-neighbours (when either same row or same col as orig)

                double d = _distMap[r1][c1][y2][x2];
                //set min dist
                if (d>=0 && d < minDist) { //d<MAX_DIST &&
                    minDist = d;
                    //next.setX(c1);
                    //next.setY(r1);
                    *nextX = c1;
                    *nextY = r1;
                }
                //if (d < _maxDist) allNeighbObst = false;
            }
        }
    }

    //return next;

}

void PropDistPlanner::printDistMap() {
    throw CException(_HERE_, "PropDistPlanner::printDistMap: not implemented");

    //todo: should be adapted to
    /*
     char x;
    //draw map
    for (int r=-2; r<=_rows; r++) {
         for (int c=-2; c<=_cols; c++) {

              //draw corners and line borders
              if (r<0 || r==_rows) {
                   if (c<0 || c==_cols)
                       x='+';
                   else if (r == -2)
                       x = (c % 10) + '0';
                   else
                       x='-';
              } else if (c<0 || c==_cols) {
                  if (c == -2)
                      x = (r % 10) + '0';
                  else
                      x = '|';
              } else {
                   //show map item

                  x=0;
                  int d = _distMap[r][c];
                  if (d==_maxDist) {
                      cout << "[X]";
                  } else {
                      if (d<10) cout <<'0';
                      printf("%d ",d);
                  }
              }

             if (x!=0) cout<<x;
         }

         cout <<endl;
    }*/
}

bool PropDistPlanner::doesCacheResults() {
    return true;
}

std::string PropDistPlanner::getName() {
    return "Propagation PathPlanner";
}
