#ifndef TESTSEGMENTER_H
#define TESTSEGMENTER_H

#include "segment.h"

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
    vector<int>* segment(vector<double>* vec, int &segmentCount);

    virtual bool segmentsCanChangeWhenSeekerHalt() {
        return false;
    }
};


#endif // TESTSEGMENTER_H
