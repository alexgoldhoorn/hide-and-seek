#ifndef ROBOTCENTEREDSEGMENTATION_H
#define ROBOTCENTEREDSEGMENTATION_H

#include "segment.h"

//#include <QPoint>

/*! This segmenter segments around a certain point (for example the position of the robot).
  Division is done based on the distance to that point (d = _segmentDist) and angle with that point
  (a = _segmentAngle). The segmentation will be dartboard like.

  The algorithm loops through all cells and calculates the distance and angle (with the centerpoint).


  NOTE: Point.x=row, point.y=col


  */
class RobotCenteredSegmentation: public Segmenter {
public:
    //! Chess distance
    static const char DIST_CHESS = 1;
    //! eucledian distance
    static const char DIST_EUCL = 2;
    //! shortest path distance
    static const char DIST_SHORTEST_PATH = 3;

    //AG120507: from K-means segmenter
    //! K means percentage of cells
    static const float KMEANS_K_S1_PERC = 0.07f;
    //! K means min K
    static const int KMEANS_K_MIN = 2;
    //! K means max K
    static const int KMEANS_K_MAX = 30;
    //! Min rows/cols to use 45º segm
    static const int MIN_WIDTH_FOR_SEGM_ANGLE_45 = 30;


    /*!
     * \brief RobotCenteredSegmentation     complete constructur
     * \param gmap                          map
     * \param neighb                        # neighbours: 4 / 8
     * \param segmentDiameter               diameter of each segment (ie distance of center when a new segment starts)
     * \param segmentAngleDeg               angles between segments, seen from center
     * \param distType                      distance type (chess/euclidean/shortest path)
     * \param centerRadius                  radius of center state (0 is default, 1 cell)
     * \param highResRadius                 radius from center on which no segmentation is done, but bottom cells are taken
     * \param setBaseState                  make a special state of base
     * \param setHiderState                 make special state of hider (if visible)
     */
    RobotCenteredSegmentation(GMap* gmap, int neighb, double segmentDiameter = -1, double segmentAngleDeg = -1, char distType = DIST_CHESS,
                              double centerRadius = 0, double highResRadius = 0, bool setBaseState = true, bool setHiderState = true );

    /*!
     * \brief RobotCenteredSegmentation     constructor without gmap
     * \param neighb                        # neighbours: 4 / 8
     * \param segmentDiameter               diameter of each segment (ie distance of center when a new segment starts)
     * \param segmentAngleDeg               angles between segments, seen from center
     * \param distType                      distance type (chess/euclidean/shortest path)
     * \param centerRadius                  radius of center state (0 is default, 1 cell)
     * \param highResRadius                 radius from center on which no segmentation is done, but bottom cells are taken
     * \param setBaseState                  make a special state of base
     * \param setHiderState                 make special state of hider (if visible)
     */
    RobotCenteredSegmentation(int neighb, double segmentDiameter = -1, double segmentAngleDeg = -1, char distType = DIST_CHESS,
                              double centerRadius = 0, double highResRadius = 0, bool setBaseState = true, bool setHiderState = true );

    virtual vector<int>* segment(vector<double>* vec, int &segmentCount);

    //! Set the distance from the center; 1d is the first zone, 2d defines the second, etc.
    //! When -1, it is calculated automatically.
    void setSegmentDiameter(double d);

    //! Get segment distance
    double getSegmentDiameter();

    //! Set the angle distance in degrees.
    //! When -1, it is calculated automatically.
    void setSegmentAngleDeg(double a);

    //! Get angle distance (rad)
    //! When -1, it is calculated automatically.
    double getSegmentAngleDeg();

    //! Set the angle distance in radials.
    void setSegmentAngleRad(double a);

    //! Get angle distance (rad)
    double getSegmentAngleRad();

    // Set center point
    //void setCenterPoint(QPoint p);

    // Set center point
    //void setCenterPoint(int row, int col);

    //! Get center point
    Pos getCenterPoint();

    //! Set distance type
    void setDistanceType(char distType);

    //! Get distance type
    char getDistanceType();

    virtual bool segmentsCanChangeWhenSeekerHalt() {
        return false; //only depends on seeker's location
    }

protected:
    //! Calculate distance depending on the type
    double calcDistance(int r, int c);

    //! Calculate angle distance depending on the type
    double calcAngleDistance(int r, int c);


    /*//! Calculate distance depending on the type
    double calcDistance(int r, int c);

    //! Calculate angle distance depending on the type
    double calcAngleDistance(int r, int c);*/


    //! Default segment dist and angle
    void calcDefaultDist(); //dist+angle

    // Default angle dist
    //double calcDefaultAngleDist();

    //! distance from 'center' to define one 'circle' (/square)
    double _segmentDiameter;

    //! angle with the point in rad
    double _segmentAngle;

    // center point which is the base of the segmentation
    //QPoint _centerPoint;
    //int _centerRow, _centerCol;

    //! Distance type
    char _distType;

    //! radius of the center state, by default 0, i.e. only the center (robot's place) is in the super state
    double _centerRadius;

    //! Make the base a seperate state on top level (if not occupied by seeker)
    bool _setBaseState;

    //! Make the hider state a seperate state on top level (if visible, i.e. there is a state y_B with b(y_B)>=0.99)
    bool _setHiderState;

    //AG121221
    //! radius from the center in which no segmentation is done and the bottom resolution is taken
    double _highResRadius;
};

#endif // ROBOTCENTEREDSEGMENTATION_H
