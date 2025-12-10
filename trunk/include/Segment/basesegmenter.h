#ifndef BASESEGMENTER_H
#define BASESEGMENTER_H

#include "Segment/segment.h"

/*! A segmenter which makes regions based on the GMap and a score s per cell.
    First groups are made based on: s<0, s=0, s>0
    Then segmenting is done based on the group and the neighbouring (connection analysis).
  */
class BaseSegmenter : public Segmenter {
public:
    //! Init with GMap and number of neighbours
    BaseSegmenter(GMap* gmap, int neighb);

    //! Init with number of neighbours.
    BaseSegmenter(int neighb);

    /*! segments based on vec value: <0, ==0, >0
    then connectioonAnalsyis is done
    */
    std::vector<int>* segment(std::vector<double>* vec, int &segmentCount);

    virtual bool segmentsCanChangeWhenSeekerHalt();
};

#endif // BASESEGMENTER_H
