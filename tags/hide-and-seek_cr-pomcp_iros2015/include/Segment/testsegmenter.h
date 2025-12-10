#ifndef TESTSEGMENTER_H
#define TESTSEGMENTER_H

#include "Segment/segment.h"
#include <vector>

/*! Segmenter for testing segmentation.
 * It generates segmentation like original map
  */
class TestSegmenter : public Segmenter {
public:
    //! Init with GMap and number of neighbours
    TestSegmenter(GMap* gmap, int neighb);

    //! Init with number of neighbours.
    TestSegmenter(int neighb);

    /*! segments based on original map
    */
    std::vector<int>* segment(std::vector<double>* vec, int &segmentCount);

    virtual bool segmentsCanChangeWhenSeekerHalt();
};


#endif // TESTSEGMENTER_H
