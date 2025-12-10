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

#ifndef _PPM_IMAGE_MAP_H_
#define _PPM_IMAGE_MAP_H_

#include <iostream>
#include <vector>

namespace map_grid
{
  
class PPMImageMap
{
  protected:
    char *image_data_;
    int width_, height_;

  public:
    PPMImageMap();
    PPMImageMap(const float & w, const float & h, const std::vector<float> & vData);
    ~PPMImageMap();

    void loadData(const float & w, const float & h, const std::vector<float> & vData);
    void setPixel(const unsigned int & ix, const unsigned int & iy, const unsigned int & r, const unsigned int & g, const unsigned int & b);
    void setLandmark(const unsigned int & ix, const unsigned int & iy, const unsigned int & r, const unsigned int & g, const unsigned int & b);
    void saveImage(const std::string & filename) const;
    bool isMapLoaded(void) const{ return image_data_!=NULL; };
};

};

#endif