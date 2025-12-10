#include "astar.h"

#include <sstream>
#include <math.h>
#include <sys/time.h>
#include <algorithm>

#include "../exceptions.h"

namespace astar
{

Node::Node(void) :
  xx_(0),
  yy_(0),
  cost2go_(0),
  cost2come_(0),
  cost_(0),
  state_(0)
{
  NUM_NODES++;
  parent_ = NULL;
}
    
Node::Node(const unsigned int & ix, const unsigned int & iy) :
  cost2go_(0),
  cost2come_(0),
  cost_(0),
  state_(0)
{
  NUM_NODES++;
  xx_ = ix;
  yy_ = iy;
  parent_ = NULL;
}

Node::~Node()
{
  NUM_NODES--;
}

void Node::setNodeCell(const unsigned int & ix, const unsigned int & iy)
{
  xx_ = ix;
  yy_ = iy;
}
    
void Node::updateCost()
{
  cost_ = cost2go_ + cost2come_;
}
    
std::string Node::printCell() const
{
  std::stringstream ss;
  ss << "(" << xx_ << "," << yy_ << ")";
  return ss.str();
}

void Node::print() const
{
  std::cout << "Cell" << printCell() << std::endl;
  std::cout << "  cost2go_=" << cost2go_ << std::endl;
  std::cout << "  cost2come_=" << cost2come_ << std::endl;
  std::cout << "  cost=" << cost_ << std::endl;
  if(parent_ != NULL)
    std::cout << "  parent_=" << parent_->printCell() << std::endl;
  else
    std::cout << "  parent_=NULL" << std::endl;
}


/**
ASTAR
*/

const unsigned int astar::AStar::STRAIGHT_BREAK;
const unsigned int astar::AStar::CHEBSHEV_2DIAG_HEURISTIC;

AStar::AStar(const map_grid::MapGrid & map, const float & maxCd, 
             const unsigned int & dH, const unsigned int & tB) :
    grid_(map),
    vVisitedMap_(grid_.getWidth()*grid_.getHeight(), NULL),
    gX_(0),
    gY_(0),
    iX_(0),
    iY_(0),
    distHeuristic_(dH),
    tieBreaking_(tB),
    maxCircleDist_(maxCd/grid_.getGridStep()),
    image_folder_("../exec/"),
    image_filename_(grid_.getConfigName()),
    exec_folder_(""),
    iterations_(0),
    requests_(0),

    cancel_execution_(false)
{
}

AStar::~AStar(void)
{
  clearMaps();
  std::cout << "astar::Node::NUM_NODES=" << astar::Node::NUM_NODES << std::endl;
}

void AStar::clearMaps(void)
{
  for(unsigned int ii=0; ii<vVisitedMap_.size(); ii++)
  {
    if(vVisitedMap_[ii] != NULL)
    {
      delete vVisitedMap_[ii];
      vVisitedMap_[ii] = NULL;
    }
  }
 
  grid_.clearVisitedGrid();
  vOpenSet_.clear();

  for(std::list<Node*>::iterator it=vPath_.begin(); it!=vPath_.end(); it++)
    delete *it;
  vPath_.clear();
}

double AStar::timeStamp()
{
  timeval timeStamp;
  gettimeofday(&timeStamp, NULL); 
  return (double)(timeStamp.tv_sec-SECONDSJANUARY09 + timeStamp.tv_usec/1e6); 
}

void AStar::saveInitMapFrame()
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "        Save Init Map Frame" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    debug_image_.loadData(grid_.getWidth(), grid_.getHeight(), grid_.getVectorObsDistGrid());
    debug_image_.saveImage(image_folder_ + image_filename_ + "/init.ppm");
  #endif
}

void AStar::createExecutionFolder(void)
{
   #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "       Create Execution folder" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    if( access(std::string(image_folder_).c_str(), 0) == -1 )
      system(std::string("mkdir "+ image_folder_).c_str());
    if( access(std::string(image_folder_ + image_filename_).c_str(), 0) == -1 )
      system(std::string("mkdir " + image_folder_ + image_filename_).c_str());

    std::stringstream ss;
    ss << "/req_" << requests_ << "_he_" << distHeuristic_ << "_tb_" << tieBreaking_;
    exec_folder_ = ss.str();

    if( access(std::string(image_folder_ + image_filename_ + exec_folder_).c_str(), 0) == -1 )
      system(std::string("mkdir "+ image_folder_ + image_filename_ + exec_folder_).c_str());
    
    debug_image_.loadData(grid_.getWidth(), grid_.getHeight(), grid_.getVectorObsDistGrid());
    std::stringstream ss2;
    ss2 << image_folder_ << image_filename_ << exec_folder_ << "/map_" << iterations_ << ".ppm";
    debug_image_.saveImage(ss2.str());
    
    std::cout << "execution file name: " << ss2.str() << std::endl;
   #endif
}

void AStar::saveMapFrame(void)
{
   #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "        Save Map Frame" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    #endif

    if(!debug_image_.isMapLoaded())
      debug_image_.loadData(grid_.getWidth(), grid_.getHeight(), grid_.getVectorObsDistGrid());

    unsigned int xx, yy;
    for(unsigned int ii=0; ii<grid_.getWidth()*grid_.getHeight(); ii++)
    {
      if( vVisitedMap_.at(ii) != NULL )
      {
        xx = vVisitedMap_.at(ii)->xx();
        yy = vVisitedMap_.at(ii)->yy();

        switch(vVisitedMap_.at(ii)->state())
        {
          case Node::OPEN:
            debug_image_.setLandmark(xx, yy, 0, 255, 0);
            break;

          case Node::CLOSE:
            debug_image_.setLandmark(xx, yy, 0, 0, 255);
            break;
        }
      }
    }

    debug_image_.setLandmark(iX_, iY_, 255, 0, 0);
    debug_image_.setLandmark(gX_, gY_, 255, 0, 255);

    #ifdef _DEBUG_MODE_
    std::stringstream ss;
    ss << image_folder_ << image_filename_ << exec_folder_ << "/map_" << iterations_ << ".ppm";
    debug_image_.saveImage(ss.str());
    #endif
}

void AStar::saveImagePath(std::list<Node*> path, const std::string & filename) const
{
   #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "        Save Image Path" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
//     if(!debug_image_.isMapLoaded())
//       debug_image_.loadData(grid_.getWidth(), grid_.getHeight(), grid_.getVectorObsDistGrid());
    map_grid::PPMImageMap path_image(grid_.getWidth(), grid_.getHeight(), grid_.getVectorObsDistGrid());

    unsigned int xx, yy;
    for(unsigned int ii=0; ii<grid_.getWidth()*grid_.getHeight(); ii++)
    {
	  //std::cout << ii << std::fflush;
      if( vVisitedMap_.at(ii) != NULL )
      {
        xx = vVisitedMap_.at(ii)->xx();
        yy = vVisitedMap_.at(ii)->yy();
		
        switch(vVisitedMap_.at(ii)->state())
        {
          case Node::OPEN:
            path_image.setLandmark(xx, yy, 0, 255, 0);
            break;

          case Node::CLOSE:
            path_image.setLandmark(xx, yy, 0, 0, 255);
            break;
        }
      }
    }

    path_image.setLandmark(iX_, iY_, 255, 0, 0);
    path_image.setLandmark(gX_, gY_, 255, 0, 255);
    
    for(std::list<Node*>::iterator it=path.begin(); it!=path.end(); it++)
        path_image.setLandmark((*it)->xx(), (*it)->yy(), 255, 0, 255);
    
    std::stringstream ss;
    ss << image_folder_ << image_filename_ << "/" << filename << requests_ << "_final_path_" << iterations_ << ".ppm";
    path_image.saveImage(ss.str());
   #endif
}

void AStar::getCurrentConfiguration(void) const
{
   #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=================================================" << std::endl;
    std::cout << "||           A-Star Path Planning              ||" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    std::cout << " from: (" << iX_ << ", " << iY_ << ")" << std::endl;
    std::cout << "   to: (" << gX_ << ", " << gY_ << ")" << std::endl;

    std::string heuristic;
    switch(distHeuristic_)
    {
      case CHEBSHEV_HEURISTIC:
        heuristic = "Chebshev Heuristic";
        break;
      case CHEBSHEV_2DIAG_HEURISTIC:
        heuristic = "Chebshev 2 Diagonal Heuristic";
        break;
      case EUCLIDEAN_HEURISTIC:
      default:
        heuristic = "Euclidean Heuristic";
        break;
    }
    
    std::string tie_break;
    switch(tieBreaking_)
    {
      case YOUNG_BREAK:
        tie_break = "Young Break";
        break;
      case STRAIGHT_BREAK:
        tie_break = "Straight Break";
        break;
      case COST2COME_BREAK:
      default:
        tie_break = "Cost 2 Come Break";
        break;
    }
    
    std::cout << " Heuristic:    " << heuristic << std::endl;
    std::cout << " Tie Breaking: " << tie_break << std::endl;
    std::cout << " Max Distance Between Way Points: " << maxCircleDist_ << std::endl;
    std::cout << "=================================================" << std::endl << std::endl;
   #endif
}

void AStar::printCurrentIteration(Node *N) const
{
  #ifdef _DEBUG_MODE_
    std::cout << "=======================================" << std::endl;
    std::cout << "            iteration " << iterations_ << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    N->print();
    std::cout << "OpenSet.size    = " << vOpenSet_.size() << std::endl;
    std::cout << "=======================================" << std::endl << std::endl;
  #endif
}

void AStar::printPath(std::list<Node*> path) const
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=================================================" << std::endl;
    std::cout << "                  Final Path" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    for(std::list<Node*>::reverse_iterator it=path.rbegin(); it!=path.rend(); it++)
      std::cout << "\tCell " << (*it)->printCell() << std::endl;
    std::cout << "=================================================" << std::endl;
  #endif
}

void AStar::createRobotNavigableMap(const float & robotClearance, const float & maxRobotStep,
                                    const float & xFloor, const float & yFloor)
{
  grid_.configureRobotParams(robotClearance, maxRobotStep);

  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "        Create Navigable Path" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    grid_.printObstacleDilationParameters();
    grid_.printZGridParameters();
    grid_.printRobotParameters();
    std::cout << std::endl << "=======================================" << std::endl;
  #endif

  try
  {
    if(grid_.isMapLoaded())
    {
      grid_.createRobotNavigableMap(xFloor, yFloor);
      saveInitMapFrame();
    }
    else
      throw CException(_HERE_, "No map loaded, first load a map file");
  }
  catch(CException e)
  {
    std::cout << __LINE__ << std::endl;
    throw e;
  }
}

float AStar::distance(Node* & node, const bool & toGoal) const
{
  unsigned int gx, gy;
  float cellCost;

  if( toGoal )
  {
    gx = gX_;
    gy = gY_;
    cellCost = (grid_.getMaxObstacleCost()+grid_.getMinMapValue())/2;
  }
  else
  {
    gx = node->parent()->xx();
    gy = node->parent()->yy();
    cellCost = grid_.getCellValue(node->xx(), node->yy());
  }
//   cout << "\t cellCost=" << cellCost << endl;
  
  float dist;
  unsigned int xdif = abs((int)node->xx()-gx);
  unsigned int ydif = abs((int)node->yy()-gy);

  switch(distHeuristic_)
  {
    case CHEBSHEV_HEURISTIC:
      // cost2GoChebyshev
      // h(n) = D * max(abs(n.x-goal.x), abs(n.y-goal.y));
      dist = cellCost*std::max(abs(xdif), abs(ydif));
      break;

    case CHEBSHEV_2DIAG_HEURISTIC:
      // cost2GoChebyshev2Diag
      // D2 = sqrt(2)*D;
      // h_diagonal(n) = min(abs(n.x-goal.x), abs(n.y-goal.y));
      // h_straight(n) = (abs(n.x-goal.x) + abs(n.y-goal.y));
      // h(n) = D2 * h_diagonal(n) + D * (h_straight(n) - 2*h_diagonal(n)));
      dist = sqrt(2) * cellCost* std::min(abs(xdif), abs(ydif)) 
             + cellCost*(abs(xdif) + abs(ydif) 
             - 2*std::min(abs(xdif), abs(ydif)));
      break;

    case EUCLIDEAN_HEURISTIC:
    default:
      // cost2GoEuclidean
      // h(n) = D * sqrt((n.x-goal.x)^2 + (n.y-goal.y)^2);
      dist = cellCost*sqrt( xdif*xdif + ydif*ydif );
      break;
  }

  return dist;
}

float AStar::Cost2Come(Node* & node) const
{
  return distance(node, false) + node->parent()->cost2come();
}

float AStar::Cost2Go(Node* & node) const
{
  float heuristic = distance(node, true);

  float dx1, dy1, dx2, dy2, cross;

  switch(tieBreaking_)
  {
    case YOUNG_BREAK:
      //The factor p should be chosen so that:
      //p <(minimum cost of taking one step)/(expected maximum path length). 
      //Assuming that you don’t expect the paths to be more than 1000 steps long,
      //you can choose p = 1/1000.
      heuristic *= (1.0 + 1/10000);
      break;

    case STRAIGHT_BREAK:
      //A different way to break ties is to prefer paths that are along the 
      //straight line from the starting point to the goal.
      //This code computes the vector cross-product between the start to goal 
      //vector and the current point to goal vector. When these vectors don’t 
      //line up, the cross product will be larger. The result is that this code
      //will give some slight preference to a path that lies along the straight 
      //line path from the start to the goal.
      dx1 = (int)node->xx() - (int)gX_;
      dy1 = (int)node->yy() - (int)gY_;
      dx2 = (int)iX_ - (int)gX_;
      dy2 = (int)iY_ - (int)gY_;
      cross = abs(dx1*dy2 - dx2*dy1);
      heuristic += cross*0.1;//0.001;
      break;

    case COST2COME_BREAK:
    default:
      //A more straightforward way to do this would to pass h to the comparison
      //function. When the f values are equal, the comparison function would break
      //the tie by looking at h.
      break;
  }

  return heuristic;
}

void AStar::computeNodeCosts(Node* & node)
{
  node->cost2come(Cost2Come(node));
  node->cost2go(Cost2Go(node));
  node->updateCost();
//   cout << "\t cost2go=" << node->cost2go << " cost2come=" << node->cost2come << " cost=" << node->cost << endl;
}

bool AStar::isInOpenSet(const unsigned int & ix, const unsigned int & iy, unsigned int & index) const
{
  index = getVectorIndex(ix, iy);
  if( vVisitedMap_.at(index) != NULL )
    return vVisitedMap_.at(index)->state() == Node::OPEN;
  else 
    return false;
}

bool AStar::isInCloseSet(const unsigned int & ix, const unsigned int & iy, unsigned int & index) const
{
  index = getVectorIndex(ix, iy);
  if( vVisitedMap_.at(index) != NULL )
    return vVisitedMap_.at(index)->state() == Node::CLOSE;
  else 
    return false;
}

Node* AStar::getBestNode()
{
  make_heap(vOpenSet_.begin(), vOpenSet_.end());
  sort_heap(vOpenSet_.begin(), vOpenSet_.end(), HeapCompareFunction());
  
  //get node from open set
  Node* node = vOpenSet_.front();

  pop_heap(vOpenSet_.begin(), vOpenSet_.end());
  vOpenSet_.pop_back();

  //mark node as close
  node->markAsClose();

  return node;
}

inline unsigned int AStar::getVectorIndex(const unsigned int & ix, const unsigned int & iy) const
{
  return grid_.getWidth()*iy + ix;
}

void AStar::addNode(Node* & node)
{
  //add node to open set and sort open set
  vOpenSet_.push_back(node);
  
  //mark node as open and point map to node
  node->markAsOpen();
  vVisitedMap_.at(getVectorIndex(node->xx(), node->yy())) = node;
}

bool AStar::findClosestFreeCell(unsigned int & ix, unsigned int & iy)
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "          Find Closest Free Point           " << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    std::cout << " initial cell:  (" << ix << "," << iy << ")" << std::endl;
  #endif
  
  const int maxNumCells = 3;
  
  //until reach maximum distance of cells
  for(int ii=1; ii<=maxNumCells; ii++)
  {
    //above and under neighbors
    for(int ux=-ii; ux<=ii; ux++)
    {
      int uy;
      for(int by=-1; by<2; by+=2)
      {
        uy = by*ii;

        //if cell is free of obstacles
        if( !grid_.isObstacle(ix+ux, iy+uy) )
        {
          ix = ix+ux;
          iy = iy+uy;
          
          #ifdef _DEBUG_MODE_
            std::cout << " new free cell: (" << ix << "," << iy << ")" << std::endl << std::endl;
          #endif
          return true;
        }
      }
    }

    //left and right neighbors
    for(int uy=-(ii-1); uy<ii; uy++)
    {
      int ux;
      for(int bx=-1; bx<2; bx+=2)
      {
        ux = bx*ii;

        //if cell is free of obstacles
        if( !grid_.isObstacle(ix+ux, iy+uy) )
        {
          ix = ix+ux;
          iy = iy+uy;

          #ifdef _DEBUG_MODE_
            std::cout << " new free cell: (" << ix << "," << iy << ")" << std::endl << std::endl;
          #endif
          return true;
        }
      }
    }
  }

  return false;
}


bool AStar::computePath(const float & ix, const float & iy, 
                        const float & gx, const float & gy,
                        const unsigned int & h,  const unsigned int & tb)
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "              Compute Path" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
  #endif

  grid_.coordinates2tiles(ix, iy, iX_, iY_);
  grid_.coordinates2tiles(gx, gy, gX_, gY_);

  distHeuristic_ = h;
  tieBreaking_   = tb;

  getCurrentConfiguration();

  //double t0 = timeStamp();
  requests_++;

  //init Init Node
  goal_node_ = NULL;
  Node *N = new Node(iX_, iY_);
  N->parent(N);
  computeNodeCosts(N);

  //check if goal point is free of obstacles
  if( grid_.isObstacle(gX_, gY_) )
  {
    delete N;
    throw CException(_HERE_, "ERROR! goalPoint is not free of obstacles");
  }

  //check if init point is free of obstacles
  if( grid_.isObstacle(iX_, iY_) )
  {
    //probably localization is not accurate
    if( !findClosestFreeCell(iX_, iY_) )
    {
      delete N;
      throw CException(_HERE_, "ERROR! initPoint is not free of obstacles");
    }
  }

  //free memory from previous paths
  clearMaps();

  //add Init Node to OpenSet
  addNode(N);
  make_heap(vOpenSet_.begin(), vOpenSet_.end());

  //init loop vars
  iterations_       = 0;
  bool pathFound    = false;
  cancel_execution_ = false;

  createExecutionFolder();
  saveInitMapFrame();
  
  //while open set is not empty
  while( !vOpenSet_.empty() && !cancel_execution_)
  {
    //get lowest cost node and remove if from OpenSet
    N = getBestNode();

    printCurrentIteration(N);

    //check if it is the goal
    if( N->xx() == gX_ && N->yy() == gY_)
    {
      pathFound = true;
      goal_node_ = N;
      break;
    }

    checkNeighbourhood(N);

//     if(iterations_%1000 == 0)
//       saveMapFrame();

    iterations_++;
  }


  return pathFound;
}

bool AStar::computeDensePath(const float & ix, const float & iy, 
                             const float & gx, const float & gy,
                             const unsigned int & h,  const unsigned int & tb)
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "          Compute Dense Path" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
  #endif

  //double t0 = timeStamp();

  bool found = computePath(ix, iy, gx, gy, h, tb);

  if( found )
  {
    recoverPathFromList();

    //compute process time
    //double t1 = timeStamp();
    //int    dT = (int)(t1-t0);

    #ifdef _DEBUG_MODE_
    std::cout << "time elapsed = " << dT << " seconds" << std::endl;
    std::cout << "iterations   = " << iterations_ << std::endl;
    std::cout << "iters/sec    = " << iterations_/std::max(1,dT) << std::endl;
    std::cout << std::endl << std::endl;
    #endif
    
    printPath(vPath_);
  }
  else
  {
    std::cout << "ERROR!! No path could be found!" << std::endl;
  }
  
  return found;
}

bool AStar::computeAndRefinePath(const float & ix, const float & iy, 
                                 const float & gx, const float & gy,
                                 const unsigned int & h,  const unsigned int & tb)
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "       Compute And Refine Path" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
  #endif

  double t0 = timeStamp();

  bool found = computePath(ix, iy, gx, gy, h, tb);

  if( found )
  {
    createPath();

    //compute process time
    double t1 = timeStamp();
    int    dT = (int)(t1-t0);

    std::cout << "time elapsed = " << dT << " seconds" << std::endl;
    std::cout << "iterations   = " << iterations_ << std::endl;
    std::cout << "iters/sec    = " << iterations_/std::max(1,dT) << std::endl;
    std::cout << std::endl << std::endl;
  }
  else
  {
    std::cout << "ERROR!! No path could be found!" << std::endl;
  }
  
  return found;
}

void AStar::checkNeighbourhood(Node* & node)
{
  try
  {
    //for all neighbors of current node
    for(int ux=-1; ux<2; ux++)
      for(int uy=-1; uy<2; uy++)
      {
        int cx = (int)node->xx()+ux;
        int cy = (int)node->yy()+uy;

        //skip negative cells
        if(cx<0 || cy<0)
          continue;

        //check neighbor availability
        if( !grid_.isObstacle(cx, cy) )
        {
          //create new node
          Node * neigNode = new Node(cx, cy);
          neigNode->parent(node);
          neigNode->cost2come(Cost2Come(neigNode));
          neigNode->cost2go(Cost2Go(neigNode));
          neigNode->updateCost();

          //get neighbor cost 2 come
          unsigned int iiOpenSet;

          //if neighbor in open set
          if( isInOpenSet(neigNode->xx(), neigNode->yy(), iiOpenSet) )
          {
            if( neigNode->cost2come() < vVisitedMap_.at(iiOpenSet)->cost2come())
            {
              //update node in open set
              vVisitedMap_.at(iiOpenSet)->parent(node);
              vVisitedMap_.at(iiOpenSet)->cost2come(neigNode->cost2come());
              vVisitedMap_.at(iiOpenSet)->cost2go(neigNode->cost2go());
              vVisitedMap_.at(iiOpenSet)->updateCost();
            }
          }
          else
          {
            if( !isInCloseSet(neigNode->xx(), neigNode->yy(), iiOpenSet) )
            {
              //add neighbor to open set
              addNode(neigNode);
              continue;
            }
          }

          delete neigNode;
        }
      }
  }
  catch(...)
  {
    std::cout << std::endl << __LINE__ << ": Not enough memory" << std::endl;
    std::cout << "iterations_=" << iterations_ << std::endl;
    std::cout << "vOpenSet_.size=" << vOpenSet_.size() << std::endl;
    saveMapFrame();
    cancel_execution_ = true;
  }
}

void AStar::recoverPathFromList()
{
  //insert last node (goal)
  Node *N = goal_node_;
  vPath_.push_back(new Node(N->xx(), N->yy()));

  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "       Recover Path From List" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    unsigned int ii=0;
    std::cout << " Cell " << ii << ": " << N->printCell() << std::endl;
  #endif
  
  //define current and last increments
  //int cAX, cAY;
  //int lAX = (int)N->parent()->xx() - (int)N->xx();
  //int lAY = (int)N->parent()->yy() - (int)N->yy();

  //for each cell in the original path
  while(N->parent() != N)
  {
    vPath_.push_back(new Node(N->xx(), N->yy()));
    #ifdef _DEBUG_MODE_
      std::cout << " Cell " << ++ii << ": " << N->printCell() << std::endl;
    #endif

    //check next cell in path
    N = N->parent();
  }

  //add first node (path origen)
  vPath_.push_back(new Node(N->xx(), N->yy()));

  #ifdef _DEBUG_MODE_
    std::cout << " Cell " << ++ii << ": " << N->printCell() << std::endl;
    std::cout << "=======================================" << std::endl << std::endl;
    saveImagePath(vPath_, "1_redundant_");
  #endif
}

void AStar::removeRedundantPointsFromPath(Node* & node)
{
  //insert last node (goal)
  Node *N = goal_node_;
  vPath_.push_back(new Node(N->xx(), N->yy()));

  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "   Remove Redundant Points from Path" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    unsigned int ii=0;
    std::cout << " Cell " << ii << ": " << N->printCell() << std::endl;
  #endif
  
  //define current and last increments
  int cAX, cAY;
  int lAX = (int)N->parent()->xx() - (int)N->xx();
  int lAY = (int)N->parent()->yy() - (int)N->yy();

  //for each cell in the original path
  while(N->parent() != N)
  {
    //get current action
    cAX = (int)N->parent()->xx() - (int)N->xx();
    cAY = (int)N->parent()->yy() - (int)N->yy();

    //if next action is different from previous, add cell to smooth path
    if( cAX!=lAX || cAY!=lAY )
    {
      vPath_.push_back(new Node(N->xx(), N->yy()));
      #ifdef _DEBUG_MODE_
        std::cout << " Cell " << ++ii << ": " << N->printCell() << std::endl;
      #endif
    }

    //update last action vars
    lAX=cAX;
    lAY=cAY;

    //check next cell in path
    N = N->parent();
  }

  //add first node (path origen)
  vPath_.push_back(new Node(N->xx(), N->yy()));

  #ifdef _DEBUG_MODE_
    std::cout << " Cell " << ++ii << ": " << N->printCell() << std::endl;
    std::cout << "=======================================" << std::endl << std::endl;
    saveImagePath(vPath_, "1_redundant_");
  #endif
}

void AStar::addWayPoints(const float & maxCircleDist)
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "        Add Way Points to Path" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
  #endif

  float ux, uy;
  float dist, ndiv;
  std::list<Node*>::iterator it=vPath_.begin();
  std::list<Node*>::iterator next;

  //insert points each maxCircleDist_ distance
  for(; it!=vPath_.end(); it++)
  {
    next=it;
    next++;

    if(next == vPath_.end())
      break;

    ux = (int)(*next)->xx() - (int)(*it)->xx();
    uy = (int)(*next)->yy() - (int)(*it)->yy();
    dist = sqrt(ux*ux + uy*uy);

    #ifdef _DEBUG_MODE_
      std::cout << " Iter Cell:     " << (*it)->printCell() << std::endl;
      std::cout << " Next Cell:     " << (*next)->printCell() << std::endl;
      std::cout << " Distance:      " << dist << " (" << maxCircleDist << ")" << std::endl;
    #endif

    //if segment is bigger than max distance
    if( dist > maxCircleDist )
    {
      ux   = ux/dist;
      uy   = uy/dist;
      ndiv = floor(dist/maxCircleDist);

      //insert point along segment each maxd
      for(int nn=1; nn<=ndiv; nn++)
      {
        int xx = ux*maxCircleDist*nn + (*it)->xx();
        int yy = uy*maxCircleDist*nn + (*it)->yy();

        vPath_.insert(next, new Node(xx, yy));
       
        #ifdef _DEBUG_MODE_
          std::cout << " Iter Cell:     " << (*it)->printCell() << std::endl;
          std::cout << " Next Cell:     " << (*next)->printCell() << std::endl;
          std::cout << " Distance:      " << dist << " (" << maxCircleDist << ")" << std::endl;
          std::cout << " Adding Cell:   (" << xx << ", " << yy << ")" << std::endl;

          for(std::list<Node*>::iterator it2=vPath_.begin(); it2!=vPath_.end(); it2++)
            std::cout << "    Cell " << ": " << (*it2)->printCell() << std::endl;
        #endif
      }
    }
    #ifdef _DEBUG_MODE_
      std::cout << std::endl;
    #endif
  }
  
  #ifdef _DEBUG_MODE_
    std::cout << "=======================================" << std::endl << std::endl;
    saveImagePath(vPath_, "2_waypoints_");
  #endif
}

void AStar::removeClosePointsFromPath(const float & minDist)
{
  #ifdef _DEBUG_MODE_
    std::cout << std::endl << "=======================================" << std::endl;
    std::cout << "     Remove Close Points from Path" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
  #endif

  //aux variables
  int ux, uy;
  float dist;
  std::list<Node*>::iterator it = vPath_.begin();
  std::list<Node*>::iterator offset, eraser;

  //remove consecutive points with less than minimum distance
  while( it != vPath_.end() )
  {
    offset = it;
    offset++;

    if(offset == vPath_.end())
      break;

    ux = (int)(*offset)->xx() - (int)(*it)->xx();
    uy = (int)(*offset)->yy() - (int)(*it)->yy();
    dist = sqrt(ux*ux + uy*uy);

    #ifdef _DEBUG_MODE_
      std::cout << " Iter Cell:     " << (*it)->printCell() << std::endl;
      std::cout << " Next Cell:     " << (*offset)->printCell() << std::endl;
      std::cout << " Distance:      " << dist << " (" << minDist << ")" << std::endl;
    #endif
    
    while(dist < minDist)
    {
      eraser=offset;
      offset++;
      
      if(offset == vPath_.end())
        break;

      #ifdef _DEBUG_MODE_
        std::cout << " Erase Cell:    " << (*eraser)->printCell() << std::endl;
      #endif

      vPath_.erase(eraser);
      delete *eraser;

      #ifdef _DEBUG_MODE_
        for(std::list<Node*>::iterator it2=vPath_.begin(); it2!=vPath_.end(); it2++)
          std::cout << "    Cell " << ": " << (*it2)->printCell() << std::endl;
      #endif
        
      ux = (int)(*offset)->xx() - (int)(*it)->xx();
      uy = (int)(*offset)->yy() - (int)(*it)->yy();
      dist = sqrt(ux*ux + uy*uy);
      
      #ifdef _DEBUG_MODE_
        std::cout << " Iter Cell:     " << (*it)->printCell() << std::endl;
        std::cout << " Next Cell:     " << (*offset)->printCell() << std::endl;
        std::cout << " Distance:      " << dist << " (" << minDist << ")" << std::endl;
        std::cout << std::endl;
      #endif
    }

    it = offset;
  }

  #ifdef _DEBUG_MODE_
    std::cout << "=======================================" << std::endl << std::endl;
    saveImagePath(vPath_, "3_closepoints_");
  #endif
}

void AStar::createPath()
{
#ifdef _DEBUG_MODE_
  std::cout << std::endl << "=================================================" << std::endl;
  std::cout << "||                Smooth Path                  ||" << std::endl;
  std::cout << "=================================================" << std::endl;
#endif

  //remove redundant points (points in same direction)
  removeRedundantPointsFromPath(goal_node_);

  //add way points each maxCircleDist_ distance
  addWayPoints(maxCircleDist_);

  //define minimum distance
  const float minDist = 1; //in [m]
  
  //remove points closer than minimum distance
  removeClosePointsFromPath(minDist/grid_.getGridStep());

  printPath(vPath_);
  saveImagePath(vPath_, "4_final_");
}

}
