#include "PathPlan/map_grid/map_grid.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>
#include <algorithm>

#include "exceptions.h"
//#include "ppm_image_map.h"

using namespace std;

namespace map_grid
{

MapGrid::MapGrid(const std::string & map_file) :
  is_map_loaded_(false),
  width_(0),
  height_(0),
  xMin_(0),
  xMax_(0),
  yMin_(0),
  yMax_(0),
  gridStep_(1), 
  square_circle_dilation_(false),
  linear_quadratic_dilation_(false),
  dilation_quadratic_factor_(0.5f),
  dilation_radius_(5),
  max_obstacle_cost_(100),
  robot_clearance_(0.7f),
  max_robot_step_(0.07f)
{
  setObstacleValue();

  is_map_loaded_ = parseGrid(map_file); //AG111125: parseZgrid --> parseGrid

  if (!is_map_loaded_)
  {
    std::stringstream ss;
    ss << "Map could NOT be loaded, please validate input map file: " << map_file;
    throw CException(_HERE_, ss.str());
  }
}


MapGrid::MapGrid(char** map, int rows, int cols) :
    is_map_loaded_(false),
    width_(0),
    height_(0),
    xMin_(0),
    xMax_(0),
    yMin_(0),
    yMax_(0),
    gridStep_(1),

    square_circle_dilation_(false),
    linear_quadratic_dilation_(false),
    dilation_quadratic_factor_(0.5f),
    dilation_radius_(5),
    max_obstacle_cost_(100),
    robot_clearance_(0.7f),
    max_robot_step_(0.07f) {

    setObstacleValue();

    is_map_loaded_ = loadMap(map, rows, cols); //AG111125: parseZgrid --> parseGrid

    if (!is_map_loaded_)
    {
      std::stringstream ss;
      ss << "Map could NOT be loaded (char**) ";
      throw CException(_HERE_, ss.str());
    }

}

MapGrid::MapGrid(const MapGrid & map)
{
  *this = map;
}

MapGrid& MapGrid::operator=(const MapGrid & map)
{
  is_map_loaded_= map.isMapLoaded();
  width_        = map.getWidth();
  height_       = map.getHeight();
  gridStep_     = map.getGridStep();
  vObsDistGrid_ = map.getVectorObsDistGrid();
  vVisitedGrid_ = map.getVectorVisitedGrid();
  robot_clearance_ = map.getRobotClearance();
  max_robot_step_  = map.getMaxRobotStep();
  map.getMinMaxXY(xMin_, xMax_, yMin_, yMax_);
  map.getDilationProperties(square_circle_dilation_, dilation_radius_, 
                            linear_quadratic_dilation_, dilation_quadratic_factor_, 
                            max_obstacle_cost_);
  setObstacleValue();

  //return map; //ag130417
}

MapGrid::~MapGrid()
{
}

void MapGrid::setObstacleValue()
{
  OBSTACLE_VALUE = max_obstacle_cost_ + max_obstacle_cost_*0.5;
}

bool MapGrid::parseZgrid(const std::string & mapPath)
{
  is_map_loaded_ = false;

  //open depth file
  std::ifstream zMapFile;
  zMapFile.exceptions(std::ifstream::failbit);

  try
  {
    zMapFile.open(mapPath.c_str(), std::ifstream::in);

    //read grid parameters from file
    if(zMapFile.good())
    {

      zMapFile >> xMin_;
      zMapFile >> xMax_;
      zMapFile >> yMin_;
      zMapFile >> yMax_;
      zMapFile >> gridStep_;

      //clear vectors from previous executions
      vObsDistGrid_.clear();
      vVisitedGrid_.clear();

      //derived parameters
      width_  = (int)((xMax_-xMin_)/gridStep_) + 1;
      height_ = (int)((yMax_-yMin_)/gridStep_) + 1;
      float numCells = width_ * height_;
      float cell;
	
      //reads grid values and loads them to the vObsDistGrid_ std::vector
      for(unsigned int ii=0; ii<numCells; ii++)
      {
        if(zMapFile.good())
        {
          zMapFile >> cell;
          vObsDistGrid_.push_back(cell);
          vVisitedGrid_.push_back(false);
        }
      }
    }

    //close depth file
    zMapFile.close();
    

    printZGridParameters();
    
    is_map_loaded_ = true;
  }
  catch (std::ifstream::failure e)
  {
    std::cout << "Exception opening map file, please check file input path: " << mapPath.c_str() << std::endl;
  }

  return is_map_loaded_;
}

//Alex G 111125: load HS or Z grid map, choose by ','
bool MapGrid::parseGrid(const std::string & mapPath)
{
  is_map_loaded_ = false;

  //open depth file
  std::ifstream zMapFile;
  zMapFile.exceptions(std::ifstream::failbit);

  try
  {
    zMapFile.open(mapPath.c_str(), std::ifstream::in);	

	//by default: normal file
	bool normalFile = true;
	
    //read grid parameters from file
    if(zMapFile.good())
    {
		 string output;
		 zMapFile >> output;
		 
		 //if first line has a comma --> not normal file (-> HS file)
		 normalFile = (output.find(",") == string::npos);
    }

    //close file
    zMapFile.close();
	
	//parse file according to format
	if (normalFile) {		  
		  is_map_loaded_ = parseZgrid(mapPath); 
	} else {		  
		  is_map_loaded_ = parseHSgrid(mapPath);
	}
  }
  catch (std::ifstream::failure e)
  {
    std::cout << "Exception opening map file, please check file input path: " << mapPath.c_str() << std::endl;
  }

  return is_map_loaded_;
}


//Alex G 111125: load a HS map
bool MapGrid::parseHSgrid(const std::string & mapPath)
{
  is_map_loaded_ = false;

  //map file
  std::ifstream zMapFile;
  zMapFile.exceptions(std::ifstream::failbit);

  try
  {	
    zMapFile.open(mapPath.c_str(), std::ifstream::in);

    //read grid parameters from file
    if(zMapFile.good())
    {
	  char output[100];	  
	  zMapFile.getline(output,256,',');
	  xMin_ = 0;
	  xMax_ = atoi(output)-1;
	  zMapFile.getline(output,256,',');
	  yMin_ = 0;
	  yMax_ = atoi(output)-1;
      gridStep_ = 1;

      //clear vectors from previous executions
      vObsDistGrid_.clear();
      vVisitedGrid_.clear();

      //derived parameters
      width_  = (int)((xMax_-xMin_)/gridStep_) + 1;
      height_ = (int)((yMax_-yMin_)/gridStep_) + 1;
      //float numCells = width_ * height_;
      float cell;
	  
	  
	  for(int r=0; r<=yMax_; r++) {
		   zMapFile >> output;
		   
		   for(int c=0; c<=xMax_; c++) {
				if (output[c] == '1') {
					 cell = -20; //obstacle 
				} else {
					cell = 0;
				}
				
				vObsDistGrid_.push_back(cell);
				vVisitedGrid_.push_back(false);
		   }		   
	  }
    }

    //close depth file
    zMapFile.close();
    

    printZGridParameters();
    
    is_map_loaded_ = true;
  }
  catch (std::ifstream::failure e)
  {
    std::cout << "Exception opening map file, please check file input path: " << mapPath.c_str() << std::endl;
  }

  return is_map_loaded_;
}

bool MapGrid::loadMap(char** map, int rows, int cols)
{
  is_map_loaded_ = false;

      xMin_ = 0;
      xMax_ = cols-1;
      yMin_ = 0;
      yMax_ = rows-1;
      gridStep_ = 1;

      //clear vectors from previous executions
      vObsDistGrid_.clear();
      vVisitedGrid_.clear();

      //derived parameters
      width_  = cols; // (int)((xMax_-xMin_)/gridStep_) + 1;
      height_ = rows; //(int)((yMax_-yMin_)/gridStep_) + 1;
      //float numCells = width_ * height_;
      float cell;

/*      cout << "----"<<endl;
      for(int r=0; r<=yMax_; r++) {
           for(int c=0; c<=xMax_; c++) {
                if (map[r][c] == 1) {
                         cell = -20; //obstacle
                         cout <<'X';
                } else {
                        cell = 0;
                        cout << ' ';
                }

                vObsDistGrid_.push_back(cell);
                vVisitedGrid_.push_back(false);
           } cout <<'|'<<endl;
      }
      cout << "-----"<<endl;*/

      //cout << "----"<<endl;

      for(int c=0; c<=xMax_; c++) {
           for(int r=0; r<=yMax_; r++) {
                if (map[r][c] == 1) {
                         cell = -20; //obstacle
                         //cout <<'X';
                } else {
                        cell = 0;
                        //cout << ' ';
                }

                vObsDistGrid_.push_back(cell);
                vVisitedGrid_.push_back(false);
           } //cout <<'|'<<endl;
      }
      //cout << "-----"<<endl;


    printZGridParameters();

    is_map_loaded_ = true;


  return is_map_loaded_;
}




void MapGrid::configureObstacleDilation(const bool & sq_cl, const unsigned int & rad, 
                                        const bool & lin_quad, const float & quad_factor, 
                                        const int & max_obst)
{
  square_circle_dilation_    = sq_cl;
  linear_quadratic_dilation_ = lin_quad;
  dilation_quadratic_factor_ = quad_factor;
  dilation_radius_           = rad;
  max_obstacle_cost_         = max_obst;
  
  printObstacleDilationParameters();
}

void MapGrid::configureRobotParams(const float & clearance, const float & max_step)
{
  robot_clearance_ = clearance;
  max_robot_step_  = max_step;
}

void MapGrid::createRobotNavigableMap(const float & xFloor, const float & yFloor)
{
#ifdef _DEBUG_MODE_
  std::cout << std::endl << "*************************************************" << std::endl;
  std::cout << " Create Robot Navigable Map" << std::endl;
  std::cout << "*************************************************" << std::endl;
#endif
  if(is_map_loaded_)
  {
    printObstacleDilationParameters();

    int ixFloor, iyFloor;
    coordinates2tiles(xFloor, yFloor, ixFloor, iyFloor);

    //consider maximum step height as obstacle
    createNavigableMapFromMaxStep(ixFloor, iyFloor);

    //dilate obstacle with robot clearance
    dilateNavigableMap();

    //compute obstacle distance map
    createObstacleDistanceNavigableMap();
    
//     //get maximum distance free of obstacles (minimum value of the map)
//     std::vector<float>::iterator it = min_element(vObsDistGrid_.begin(), vObsDistGrid_.end());
//     maxDist2Obs_ = *it;

    std::cout << "Navigation Map is ready to be used." << std::endl;
  }
  else
  {
    std::cout << __LINE__ << std::endl;
    throw CException(_HERE_, "Map could not be loaded, please validate input map file");
  }
}

void MapGrid::printZGridParameters() const
{
 // #ifdef _DEBUG_MODE_
    std::cout << std::endl << "*************************************************" << std::endl;
    std::cout << " Z Grid Map Parameters:" << std::endl;
    std::cout << "   - X(" << xMin_ << ", " << xMax_ << ")" << std::endl;
    std::cout << "   - Y(" << yMin_ << ", " << yMax_ << ")" << std::endl;
    std::cout << "   - width:     " << width_ << std::endl;
    std::cout << "   - height:    " << height_ << std::endl;
    std::cout << "   - map size:  " << vVisitedGrid_.size() << std::endl;
    std::cout << "   - grid step: " << gridStep_ << std::endl;
    std::cout << "*************************************************" << std::endl << std::endl;
 // #endif
}

void MapGrid::printObstacleDilationParameters() const
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "*************************************************" << std::endl;
    std::cout << " Current Obstacle Dilation Parameters:" << std::endl;

    if(square_circle_dilation_)
      std::cout << "   - dilation shape:    square" << std::endl;
    else
      std::cout << "   - dilation shape:    circle    [radius=" << dilation_radius_ << "]" << std::endl;
    
    if(linear_quadratic_dilation_)
      std::cout << "   - dilation type:     linear" << std::endl;
    else
      std::cout << "   - dilation type:     quadratic [factor=" << dilation_quadratic_factor_ << "]" << std::endl;

    std::cout << "   - max obstacle cost: " << max_obstacle_cost_ << std::endl;
    std::cout << "*************************************************" << std::endl << std::endl;
  #endif
}

void MapGrid::printRobotParameters() const
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "*************************************************" << std::endl;
    std::cout << " Current Robot Parameters:" << std::endl;
    std::cout << "   - robot clearance: " << robot_clearance_ << std::endl;
    std::cout << "   - max robot step:  " << max_robot_step_ << std::endl;
    std::cout << "*************************************************" << std::endl << std::endl;
  #endif
}

std::string MapGrid::getConfigName() const
{
  std::stringstream ss, sc, lq;
  
  if( square_circle_dilation_ ) sc << "_sq";
  else                          sc << "_cl_" << dilation_radius_;

  if( square_circle_dilation_ ) lq << "_lin";
  else                          lq << "_quad_" << dilation_quadratic_factor_;
  
  
  ss << "map_grid_gs_" << gridStep_ << sc.str() << lq.str() << "_oc_" << max_obstacle_cost_;
  ss << "_rc_" << robot_clearance_ << "_step_" << max_robot_step_;
  
  return ss.str();
}

void MapGrid::clearVisitedGrid()
{
  for(int ii=0; ii<(int)vVisitedGrid_.size(); ii++)
    vVisitedGrid_[ii]=false;
}

void MapGrid::getMinMaxXY(float & xmin, float & xmax, float & ymin, float & ymax) const
{
  xmin = xMin_;
  xmax = xMax_;
  ymin = yMin_;
  ymax = yMax_;
}

void MapGrid::getDilationProperties(bool & sq_cl, unsigned int & rad, bool & lin_quad, 
                                    float & quad_factor, unsigned int & max_obst) const
{
  sq_cl       = square_circle_dilation_;
  rad         = dilation_radius_;
  lin_quad    = linear_quadratic_dilation_;
  quad_factor = dilation_quadratic_factor_;
  max_obst    = max_obstacle_cost_;
}

void MapGrid::coordinates2tiles(const float & xx, const float & yy, int & ix, int & iy) const
{
  ix = (int)( (xx-xMin_)/gridStep_ );
  iy = (int)( (yy-yMin_)/gridStep_ );
}

void MapGrid::coordinates2tiles(const float & xx, const float & yy, unsigned int & ix, unsigned int & iy) const
{
  ix = (unsigned int)( (xx-xMin_)/gridStep_ );
  iy = (unsigned int)( (yy-yMin_)/gridStep_ );
}

void MapGrid::tiles2coordinates(const unsigned int & ix, const unsigned int & iy, float & xx, float & yy) const
{
  xx = (gridStep_*ix)+xMin_;
  yy = (gridStep_*iy)+yMin_;
}

float MapGrid::getCellValue(const float & xx, const float & yy) const
{
  int ix, iy;
  coordinates2tiles(xx, yy, ix, iy);
  return vObsDistGrid_.at(width_*iy + ix);
}

float MapGrid::getCellValue(const unsigned int & ix, const unsigned int & iy) const
{
  return vObsDistGrid_.at(width_*iy + ix);
}

bool MapGrid::isVisited(const unsigned int & ix, const unsigned int & iy) const
{
  return vVisitedGrid_.at(width_*iy + ix);
}

void MapGrid::setVisited(const unsigned int & ix, const unsigned int & iy)
{
  vVisitedGrid_.at(width_*iy + ix) = true;
}

void MapGrid::setNotVisited(const unsigned int & ix, const unsigned int & iy)
{
  vVisitedGrid_.at(width_*iy + ix) = false;
}

bool MapGrid::isObstacle(const unsigned int & ix, const unsigned int & iy) const
{
  return isObstacle((int)ix, (int)iy);
}

bool MapGrid::isObstacle(const int & ix, const int & iy) const
{
  if( !isInMap(ix, iy) || vObsDistGrid_.at(width_*iy + ix) == OBSTACLE_VALUE )
    return true;
  else 
    return false;
}

bool MapGrid::isObstacle(const float & xx, const float & yy) const
{
  int ix, iy;
  coordinates2tiles(xx, yy, ix, iy);
  return isObstacle(ix, iy);
}

bool MapGrid::isInMap(const unsigned int & ix, const unsigned int & iy) const
{
  return isInMap((int)ix, (int)iy);
}

bool MapGrid::isInMap(const int & ix, const int & iy) const
{
  if( ix >= 0 && ix < (int)width_ && iy >= 0 && iy < (int)height_ )
    return true;
  else 
    return false;
}

bool MapGrid::isInMap(const float & xx, const float & yy) const
{
  int ix, iy;
  coordinates2tiles(xx, yy, ix, iy);
  return isInMap(ix, iy);
}

void MapGrid::printGrid() const
{
  for(int iy=std::min(100,(int)(height_-1)); iy>=0; iy--)
  {
    for(int ix=0; ix<std::min(150,(int)width_); ix++)
    {
//    std::cout << "(" << ix << ", " << iy << ")" << std::endl;
      if( vObsDistGrid_.at(width_*iy + ix) == OBSTACLE_VALUE) std::cout << "+";
        else if( vObsDistGrid_.at(width_*iy + ix) == 0) std::cout << "·";
      else std::cout << (int)vObsDistGrid_.at(width_*iy + ix);
    }
    std::cout << std::endl;
  }
}

void MapGrid::createNavigableMapFromMaxStep(const int & ixFloor, const int & iyFloor)
{
  std::deque<unsigned int> vOpenX, vOpenY;
  std::vector<float> vGrid(vObsDistGrid_.size(), map_grid::UNKNOWN_CELL);

  if( !isInMap(ixFloor, iyFloor) )
    throw CException(_HERE_, "Invalid floor cell, out of the map. Please provide a new cell.");

  //add floor point to open set
  vOpenX.push_back(ixFloor);
  vOpenY.push_back(iyFloor);
  vGrid[width_*iyFloor + ixFloor] = map_grid::NAVIGABLE_CELL;

  //while open set not empty
  while( !vOpenX.empty() )
  {
    //get next cell
    unsigned int cx = vOpenX.front();
    unsigned int cy = vOpenY.front();
    vOpenX.pop_front();
    vOpenY.pop_front();

    //for all cell neightbours
    for(int ux=-1; ux<2; ux++)
      for(int uy=-1; uy<2; uy++)
      {
        //if neightbour is in map & has not been visited yet
        if( isInMap(cx+ux,cy+uy) && vGrid.at(width_*(cy+uy) + (cx+ux)) == map_grid::UNKNOWN_CELL )
        {
          //compute height increment
          float Ah = fabs(getCellValue(cx+ux,cy+uy) - getCellValue(cx,cy));
          
          //if height increment is less than max step
          if( Ah <= max_robot_step_ )
          {
            //add cell to open set
            vOpenX.push_back(cx+ux);
            vOpenY.push_back(cy+uy);

            //mark cell as navigable
            vGrid[width_*(cy+uy) + (cx+ux)] = map_grid::NAVIGABLE_CELL;
          }
          //else (height increment bigger than max step)
          else
          {
            //mark cell as obstacle
            vGrid[width_*(cy+uy) + (cx+ux)] = OBSTACLE_VALUE;
          }
        }
      }
  }

  //for all cells in map
  for(unsigned int ii=0; ii<vGrid.size(); ii++)
  {
    //if cell is unknown
    //mark cell as obstacle
    if( vGrid[ii] == map_grid::UNKNOWN_CELL )
      vGrid[ii] = OBSTACLE_VALUE;
  }

  //update obstacle distance map
  vObsDistGrid_ = vGrid;

  ////PMImageMap imap(width_, height_, vObsDistGrid_);
  ///imap.setLandmark(ixFloor, iyFloor, 225, 0, 0);
  std::stringstream ss;
  ss << "1_max_step_rs" << max_robot_step_ << ".ppm";
  ///imap.saveImage(ss.str());
}

void MapGrid::dilateNavigableMap()
{
  
  int clearance = round(robot_clearance_/gridStep_);
  int xx, yy;

  for(unsigned int iy=0; iy<height_; iy++)
    for(unsigned int ix=0; ix<width_; ix++)
      if( isObstacle(ix,iy) )
        for(int cx=-clearance; cx<=clearance; cx++)
          for(int cy=-clearance; cy<=clearance; cy++)
          {
            xx = ix+cx;
            yy = iy+cy;
            if( isInMap(xx,yy) && !isObstacle((unsigned int)xx,(unsigned int)yy))
            {
              if( !square_circle_dilation_ )
              {
                if( cx*cx+cy*cy <= clearance*clearance )
                  vObsDistGrid_.at(width_*yy + xx) = OBSTACLE_VALUE*2;
              }
              else
                  vObsDistGrid_.at(width_*yy + xx) = OBSTACLE_VALUE*2;
            }
          }
  
  for(unsigned int iy=0; iy<height_; iy++)
    for(unsigned int ix=0; ix<width_; ix++)
      if( getCellValue(ix,iy) == OBSTACLE_VALUE*2)
        vObsDistGrid_.at(width_*iy + ix) = OBSTACLE_VALUE;

  ///PPMImageMap imap(width_, height_, vObsDistGrid_);
  std::stringstream ss;
  ss << "2_dilate_rs" << max_robot_step_ << "_rc" << robot_clearance_ << ".ppm";
  ///imap.saveImage(ss.str());
}

void MapGrid::checkNeighbourhood(const unsigned int & ix, const unsigned int & iy, const float & find, const float & subs)
{
  for(int ux=-1; ux<2; ux++)
    for(int uy=-1; uy<2; uy++)
      if( isInMap(ix+ux,iy+uy) && getCellValue(ix+ux,iy+uy) == find )
      {
        vObsDistGrid_.at(width_*iy + ix) = subs;
        return;
      }
}

void MapGrid::checkCircleNeighbourhood(const unsigned int & ix, const unsigned int & iy, const float & radius, const float & find, const float & subs)
{
  int xx, yy;
  for(int ux=-radius; ux<=radius; ux++)
    for(int uy=-radius; uy<=radius; uy++)
    {
      xx = ix+ux;
      yy = iy+uy;
      if( isInMap(xx,yy) && getCellValue((unsigned int)xx,(unsigned int)yy) == find && (ux*ux+uy*uy<=radius*radius) )
        vObsDistGrid_.at(width_*yy + xx) = subs;
    }
}

void MapGrid::createObstacleDistanceNavigableMap()
{
  int step = 1;
  if( !square_circle_dilation_ )
    step = dilation_radius_;

  for(unsigned int iy=0; iy<height_; iy++)
    for(unsigned int ix=0; ix<width_; ix++)
    {
      //square dilation_
      if( square_circle_dilation_ )
      {
        if( getCellValue(ix,iy) == map_grid::NAVIGABLE_CELL)
          checkNeighbourhood(ix, iy, OBSTACLE_VALUE, max_obstacle_cost_);
      }
      //circle dilation
      else
      {
        if( getCellValue(ix,iy) == OBSTACLE_VALUE)
          checkCircleNeighbourhood(ix, iy, step, map_grid::NAVIGABLE_CELL, max_obstacle_cost_);
      }
    }

  std::vector<float> vDistVals;
  vDistVals.push_back(max_obstacle_cost_);

  //linear dilation
  if(linear_quadratic_dilation_)
  {
    for(int ii=1; ii<(int)max_obstacle_cost_; ii++)
      vDistVals.push_back(max_obstacle_cost_-ii);
  }
  //quadratic dilation
  else 
  {
    int ii=1;
    float val=max_obstacle_cost_-dilation_quadratic_factor_*ii*ii;
    while(val > 0)
    {
      vDistVals.push_back(val);
      ii++;
      val=max_obstacle_cost_-dilation_quadratic_factor_*ii*ii;
    }
  }

  for(unsigned int ii=step; ii<vDistVals.size(); ii+=step)
  {
    for(unsigned int iy=0; iy<height_; iy++)
      for(unsigned int ix=0; ix<width_; ix++)
      {
        if( square_circle_dilation_ )
        {
          if( getCellValue(ix,iy) == map_grid::NAVIGABLE_CELL)
            checkNeighbourhood(ix, iy, vDistVals[ii-step], vDistVals[ii]);
        }
        else
        {
          if( getCellValue(ix,iy) == vDistVals[ii-step])
            checkCircleNeighbourhood(ix, iy, step, map_grid::NAVIGABLE_CELL, vDistVals[ii]);
        }
      }
  }

  ///PPMImageMap imap(width_, height_, vObsDistGrid_);
  std::stringstream ss;
  ss << "3_obs_dist_rs" << max_robot_step_ << "_rc" << robot_clearance_;
  ss << "_sc" << square_circle_dilation_ << "_lq" << linear_quadratic_dilation_ << ".ppm";
  ///imap.saveImage(ss.str());
}

}
