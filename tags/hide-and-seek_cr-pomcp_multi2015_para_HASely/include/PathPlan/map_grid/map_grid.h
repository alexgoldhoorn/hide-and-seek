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

#ifndef _MAP_GRID_H_
#define _MAP_GRID_H_

#include <stdio.h>
#include <iostream>

#include <vector>

namespace map_grid
{

//#define _DEBUG_MODE_

static const float UNKNOWN_CELL   = -1;
static const float NAVIGABLE_CELL =  0;
// static const float OBSTACLE_VALUE = 500.f;

class MapGrid
{
  protected:
    float OBSTACLE_VALUE;

    bool is_map_loaded_;
    unsigned int width_, height_;
    float xMin_, xMax_, yMin_, yMax_, gridStep_;
    std::vector<float> vObsDistGrid_;
    std::vector<bool> vVisitedGrid_;
    float maxDist2Obs_;
    
    bool square_circle_dilation_, linear_quadratic_dilation_;
    float dilation_quadratic_factor_;
    unsigned int dilation_radius_;
    unsigned int max_obstacle_cost_;
    float robot_clearance_;
    float max_robot_step_;

    void setObstacleValue();
    bool parseZgrid(const std::string & map_file);
	
	//AG111125: choose parser for map
	bool parseGrid(const std::string & mapPath);
	//AG111125: load HS map
	bool parseHSgrid(const std::string & mapPath);
        //AG111125: load from map directly
        bool loadMap(char** map, int rows, int cols);
	
    void dilateNavigableMap();
    void createObstacleDistanceNavigableMap();
    void createNavigableMapFromMaxStep(const int & ixFloor=0, const int & iyFloor=0);
    void checkNeighbourhood(const unsigned int & ix, const unsigned int & iy, const float & find, const float & subs);
    void checkCircleNeighbourhood(const unsigned int & ix, const unsigned int & iy, const float & radius, const float & find, const float & subs);

  public:
    MapGrid(const std::string & map_file);

    MapGrid(char** map, int rows, int cols); //ag111125

    MapGrid(const MapGrid & map);
    ~MapGrid();

    MapGrid & operator=(const MapGrid& map);

    void configureObstacleDilation(const bool & sq_cl=false, const unsigned int & radius=5, const bool & lin_quad=false, const float & quad_factor=0.5f, const int & max_obst=100);
    void configureRobotParams(const float & clearance=0.7f, const float & max_step=0.07f);
    void createRobotNavigableMap(const float & xFloor=0, const float & yFloor=0);

    void printObstacleDilationParameters() const;
    void printZGridParameters() const;
    void printRobotParameters() const;
    std::string getConfigName() const;

    void clearVisitedGrid();
    void getMinMaxXY(float & xmin, float & xmax, float & ymin, float & ymax) const;
    void getDilationProperties(bool & sq_cl, unsigned int & rad, bool & lin_quad, float & quad_factor, unsigned int & max_obst) const;
    float getGridStep() const { return gridStep_; };
    float getMinMapValue() const { return maxDist2Obs_; };
    std::vector<float> getVectorObsDistGrid() const { return vObsDistGrid_; };
    std::vector<bool> getVectorVisitedGrid() const { return vVisitedGrid_; };
    unsigned int getWidth() const { return width_; };
    unsigned int getHeight() const { return height_; };
    float getRobotClearance() const { return robot_clearance_; }
    float getMaxRobotStep() const { return max_robot_step_; }
    int getMaxObstacleCost() const { return max_obstacle_cost_; }
    void coordinates2tiles(const float & xx, const float & yy, int & ix, int & iy) const;
    void coordinates2tiles(const float & xx, const float & yy, unsigned int & ix, unsigned int & iy) const;
    void tiles2coordinates(const unsigned int & ix, const unsigned int & iy, float & xx, float & yy) const;
    float getCellValue(const float & xx, const float & yy) const;
    float getCellValue(const unsigned int & ix, const unsigned int & iy) const;
    bool isVisited(const unsigned int & ix, const unsigned int & iy) const;
    void setVisited(const unsigned int & ix, const unsigned int & iy);
    void setNotVisited(const unsigned int & ix, const unsigned int & iy);
    bool isObstacle(const unsigned int & ix, const unsigned int & iy) const;
    bool isObstacle(const int & ix, const int & iy) const;
    bool isObstacle(const float & xx, const float & yy) const;
    bool isInMap(const unsigned int & ix, const unsigned int & iy) const;
    bool isInMap(const int & ix, const int & iy) const;
    bool isInMap(const float & xx, const float & yy) const;
    void printGrid() const;
    bool isMapLoaded() const { return is_map_loaded_; }
};

};

#endif

