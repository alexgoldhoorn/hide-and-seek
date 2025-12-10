#ifndef COMBINECENTEREDSEGMENTATION_H
#define COMBINECENTEREDSEGMENTATION_H


#include "robotcenteredsegmentation.h"

//#include <QPoint>

/*!
 Combines a centered segmentation of the seeker and the base. It uses the RobotCenteredSegmentation class to generate
 the specific segmentations, and then combines it.

  NOTE: Point.x=row, point.y=col

TODO: later add also hider (?), but then requires to get hider pos, but this is available through belief

  */
class CombineCenteredSegmentation : public RobotCenteredSegmentation {
public:
    //! CombineCenteredSegmentation for gmap, #neighbours, segmentation distance (see setSegmentDist) and angle distance (in degrees) (see setAngleDistDeg)
    CombineCenteredSegmentation(GMap* gmap, int neighb, double segmentDiameter = -1, double segmentAngleDeg = -1,
                                char distType = DIST_CHESS, double centerRadius = 0, bool setHiderState = true, double highResRadius = 0  );

    //! CombineCenteredSegmentation for #neighbours, segmentation distance (see setSegmentDist) and angle distance (in degrees) (see setAngleDistDeg)
    CombineCenteredSegmentation(int neighb, double segmentDiameter = -1, double segmentAngleDeg = -1,
                                char distType = DIST_CHESS, double centerRadius = 0,  bool setHiderState = true, double highResRadius = 0  );

    virtual vector<int>* segment(vector<double>* vec, int &segmentCount);


    //! set position
    //CHECK: is this passed really from segment ??
    //TODO: pass to _seekerCSegmenter
    virtual void setPosition(int row, int col);


    //! Set GMap
    void setGMap(GMap* gmap);

protected:
    //! init
    void initBS();

    //! Seeker centered segmenter
    RobotCenteredSegmentation _seekerCSegmenter;

    //! Base centered segmenter
    RobotCenteredSegmentation _baseCSegmenter;

    //! Segments from base perspective
    vector<int>* _baseCSegments;

    //! Num segments from base perspective
    int _baseCSegmentCount;

};


#endif // COMBINECENTEREDSEGMENTATION_H
