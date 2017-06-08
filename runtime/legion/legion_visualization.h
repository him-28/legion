/* Copyright 2017 Stanford University, NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef legion_visualization_h
#define legion_visualization_h

#include "legion.h"

using namespace LegionRuntime::Arrays;

namespace Legion {
    namespace Visualization {
        
        static const int image_region_dimensions = 3;//(width x height) x layerID
        
        typedef struct {
            int width;
            int height;
            int numImageLayers;
            int numFragmentsPerLayer;
            
            int pixelsPerLayer() const{ return width * height; }
          
            Point<image_region_dimensions> origin() const{ return Point<image_region_dimensions>::ZEROES(); }
            Point<image_region_dimensions> upperBound() const{
                Point<image_region_dimensions> result;
                result.x[0] = width;
                result.x[1] = height;
                result.x[2] = numImageLayers;
                return result;
            }
            
            // launch by depth plane, each depth point is one image
            Point<image_region_dimensions> layerSize() const{
                Point<image_region_dimensions> result;
                result.x[0] = width;
                result.x[1] = height;
                result.x[2] = 1;
                return result;
            }
            Point<image_region_dimensions> numLayers() const{
                Point<image_region_dimensions> result;
                result.x[0] = 1;
                result.x[1] = 1;
                result.x[2] = numImageLayers;
                return result;
            }
            
            // launch by composite fragment,
            Point<image_region_dimensions> fragmentSize() const{
                Point<image_region_dimensions> result;
                if(numFragmentsPerLayer > height) {
                    assert((width * height) % numFragmentsPerLayer == 0);
                    result.x[0] = (width * height) / numFragmentsPerLayer;
                    result.x[1] = 1;
                    result.x[2] = 1;
                } else {
                    result.x[0] = width;
                    assert(height % numFragmentsPerLayer == 0);
                    result.x[1] = height / numFragmentsPerLayer;
                    result.x[2] = 1;
                }
                return result;
            }
            Point<image_region_dimensions> numFragments() const{
                Point<image_region_dimensions> result;
                Point<image_region_dimensions> size = fragmentSize();
                result.x[0] = width / size.x[0];
                result.x[1] = height / size.x[1];
                result.x[2] = numImageLayers;
                return result;
            }
                        
            Point<image_region_dimensions> incrementFragment(Point<image_region_dimensions> point) const {
                point.x[0] += 1;
                if(point.x[0] >= numFragments().x[0]) {
                    point.x[0] = 0;
                    point.x[1] += 1;
                    if(point.x[1] >= numFragments().x[1]) {
                        point.x[1] = 0;
                        point.x[2] += 1;
                        if(point.x[2] >= numFragments().x[2]) {
                            point.x[2] = 0;
                        }
                    }
                }
                return point;
            }
            
            int numPixelsPerFragment() const {
                Point<image_region_dimensions> size = fragmentSize();
                int result = 1;
                for(int i = 0; i < image_region_dimensions; ++i) {
                    result *= size.x[i];
                }
                return result;
            }
          
          std::string toString() const {
            char buffer[512];
            sprintf(buffer, "(%dx%d) x %d layers, %d fragments per layer (%lldx%lldx%lld)",
                    width, height, numImageLayers, numFragmentsPerLayer,
                    fragmentSize().x[0], fragmentSize().x[1], fragmentSize().x[2]);
            return std::string(buffer);
          }
          
        } ImageSize;
      
    }
}

#include "image_reduction.h"


#endif /* legion_visualization_h */
