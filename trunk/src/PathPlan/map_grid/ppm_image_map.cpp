#include "PathPlan/map_grid/ppm_image_map.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include "exceptions.h"

namespace map_grid
{

PPMImageMap::PPMImageMap() :
  width_(0),
  height_(0)
{
  image_data_ = NULL;
}

PPMImageMap::PPMImageMap(const float & w, const float & h, const std::vector<float> & vData)
{
  image_data_ = NULL;
  loadData(w, h, vData);
}

PPMImageMap::~PPMImageMap()
{
  if( image_data_ != NULL)
    delete []image_data_;
}

void PPMImageMap::loadData(const float & w, const float & h, const std::vector<float> & vData)
{
  width_  = w;
  height_ = h;

  if( width_*height_ < (int)vData.size() )
    throw CException(_HERE_, "Incorrect input width/height, larger than data vector");
  
  if( image_data_ != NULL )
    delete []image_data_;

  image_data_ = new char[width_*height_*3];
  
  float max = *max_element(vData.begin(), vData.end());
  float min = *min_element(vData.begin(), vData.end());
  float norm = 255.f/(max-min);
  
  for(int iy=0; iy<height_; iy++)
    for(int ix=0; ix<width_; ix++)
    {
      unsigned int bk_id = width_*(height_-iy-1)+ix;
      unsigned int fw_id = 3*(width_*iy+ix);
      float val = vData.at(bk_id);

      image_data_[fw_id+0] = 255-(val-min)*norm;
      image_data_[fw_id+1] = 255-(val-min)*norm;
      image_data_[fw_id+2] = 255-(val-min)*norm;
    }
}

void PPMImageMap::setPixel(const unsigned int & ix, const unsigned int & iy, const unsigned int & r, const unsigned int & g, const unsigned int & b)
{
  if( image_data_ == NULL )
    throw CException(_HERE_, "No data loaded. First call PPMImageMap::loadData.");
    
  if( (int)ix>width_ || (int)iy>height_ || (int)ix<0 || (int)iy<0 )
  {
    std::stringstream ss;
    ss << "Input point out of image: point(" << (int)ix << "," << height_-(int)iy << "), image[" << width_ << "," << height_ << "]";
    throw CException(_HERE_, ss.str());
  }

  unsigned int bk_id = width_*(height_-iy-1)+ix;
  image_data_[3*bk_id+0] = r;
  image_data_[3*bk_id+1] = g;
  image_data_[3*bk_id+2] = b;
}

void PPMImageMap::setLandmark(const unsigned int & ix, const unsigned int & iy, const unsigned int & r, const unsigned int & g, const unsigned int & b)
{
  try
  {
  setPixel(ix, iy, r, g, b);

  if(ix>0)
    setPixel(ix-1, iy,   r, g, b);
  if(iy>0)
    setPixel(ix,   iy-1, r, g, b);

  if((int)ix<width_)
    setPixel(ix+1, iy,   r, g, b);
  if((int)iy<height_)
    setPixel(ix,   iy+1, r, g, b);
  }
  catch(...)
  {
  }
}


//void PPMImageMap::saveImage(const std::string & filename) const
//{ //AG: doenst accept the ofstream params, why??????
  /*if(filename.size() == 0)
    throw CException(_HERE_, "Invalid filename for the image.");
  
  if(image_data_ == NULL)
    throw CException(_HERE_, "No data loaded. First call PPMImageMap::loadData.");

  //create image file
  std::ofstream img_file(filename, std::ios::binary);

  //write header
  img_file << "P6" << std::endl << width_ << " " << height_ << std::endl << 255 << std::endl;

  img_file.write(image_data_, width_*height_*3);

  //close image file
  img_file.close();*/
//}

}
