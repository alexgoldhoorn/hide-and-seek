#include "Segment/robotcenteredsegmentation.h"
#include <cmath>
#include <cassert>
#include <stdlib.h>
#include <iostream>

using namespace std;

RobotCenteredSegmentation::RobotCenteredSegmentation(GMap* gmap, int neighb, double segmentDiam, double segmentAngleDeg, char distType,
                                                     double centerRadius, double highResRadius, bool setBaseState, bool setHiderState ) :
        Segmenter(gmap, neighb) {

    setSegmentDiameter(segmentDiam);
    setSegmentAngleDeg(segmentAngleDeg);
    setDistanceType(distType);

    assert(centerRadius>=0);
    assert(highResRadius>=0);

    _centerRadius = centerRadius;
    _setBaseState = setBaseState;
    _setHiderState = setHiderState;
    _highResRadius = highResRadius;
}

RobotCenteredSegmentation::RobotCenteredSegmentation(int neighb, double segmentDiam, double segmentAngleDeg, char distType,
                                                     double centerRadius, double highResRadius, bool setBaseState, bool setHiderState ) :
        Segmenter(neighb) {

    setSegmentDiameter(segmentDiam);
    setSegmentAngleDeg(segmentAngleDeg);
    setDistanceType(distType);

    assert(centerRadius>=0);
    assert(highResRadius>=0);

    _centerRadius = centerRadius;
    _setBaseState = setBaseState;
    _setHiderState = setHiderState;
    _highResRadius = highResRadius;
}

//! Set the distance from the center; 1d is the first zone, 2d defines the second, etc.
void RobotCenteredSegmentation::setSegmentDiameter(double d) {
    assert(d==-1 || d>0);
    _segmentDiameter = d;
}

//! Get segment distance
double RobotCenteredSegmentation::getSegmentDiameter() {
    return _segmentDiameter;
}

//! Set the angle distance in degrees.
void RobotCenteredSegmentation::setSegmentAngleDeg(double a) {
    if (a==-1) {
        _segmentAngle = -1;
    } else {
        _segmentAngle = M_PI*a/180.0; //AG120829: error found: 360 -> 180
    }
}

//! Get angle distance (rad)
double RobotCenteredSegmentation::getSegmentAngleDeg() {
    return _segmentAngle*180/M_PI; //AG120829: error found: 360 -> 180
}

//! Set the angle distance in radials.
void RobotCenteredSegmentation::setSegmentAngleRad(double a) {
    if (a==-1) {
        _segmentAngle = -1;
    } else {
        _segmentAngle = a;
    }
}

//! Set the angle distance in radials.
double RobotCenteredSegmentation::getSegmentAngleRad() {
    return _segmentAngle;
}

//! Set center point
/*void RobotCenteredSegmentation::setCenterPoint(QPoint p) {
    //_centerPoint = p;
    _centerRow = p.x();
    _centerCol = p.y();
}

void RobotCenteredSegmentation::setCenterPoint(int row, int col) {
    _centerRow = row;
    _centerCol = col;
}*/

//! Get center point
Pos RobotCenteredSegmentation::getCenterPoint() {
    return getPosition();
}

//! Set distance type
void RobotCenteredSegmentation::setDistanceType(char distType) {
    assert(distType==DIST_CHESS || distType==DIST_EUCL || distType==DIST_SHORTEST_PATH);
    _distType = distType;
}

//! Get distance type
char RobotCenteredSegmentation::getDistanceType() {
    return _distType;
}

double RobotCenteredSegmentation::calcDistance(int r, int c) {
    double d = -1;


    switch (_distType) {
    case DIST_CHESS:
        d = max(abs(r-_rowPos),abs(c-_colPos));
        break;
    case DIST_EUCL:
        d = sqrt(pow(r-_rowPos,2)+pow(c-_colPos,2));
        break;
    case DIST_SHORTEST_PATH:
        cout << "Shortest path not supported yet by RobotCenteredSegmentation"<<endl;
        exit(-1);
        break;
    default:
        cout << "Unexpected distance type: " << _distType <<endl;
        exit(-1);
        break;
    }

    return d;
}

double RobotCenteredSegmentation::calcAngleDistance(int r, int c) {
    //http://www.cplusplus.com/reference/clibrary/cmath/atan2/
    //atan2 returns in [-pi,+pi]

    //return modulus<double>(atan2(r-_centerRow, c-_centerCol)+M_PI, (2*M_PI));
    return fmod(atan2(r-_rowPos, c-_colPos)+M_PI, (2*M_PI));
}

void RobotCenteredSegmentation::calcDefaultDist() {
    DEBUG_SEGMENT(if (_segmentDiameter!=-1 || _segmentAngle!=-1)
                  cout << "RobotCenteredSegmentation.calcDefaultDist: overwriting segment diameter ("
                       <<_segmentDiameter<<") and angle dist ("<<_segmentAngle<<")"<<endl;);

    //first estimate the number regions wanted

    //from k calculation
    int freeCells = _gmap->numFreeCells();
    int totalCells = _gmap->width()*_gmap->height();
    int obstCells = totalCells-freeCells;

    //calc scores, later get min
    int s1 = round(KMEANS_K_S1_PERC*freeCells);
    int s2 = round(freeCells / obstCells);
    int k = min(s1,s2);
    if (k<KMEANS_K_MIN) {
        k = KMEANS_K_MIN;
    } else if (k>KMEANS_K_MAX) {
        k = KMEANS_K_MAX;
    }

    DEBUG_SEGMENT(cout << " [k="<<k<<"] ";);


    //now angle based on min width
    if (_gmap->colCount()>=MIN_WIDTH_FOR_SEGM_ANGLE_45 && _gmap->rowCount()>=MIN_WIDTH_FOR_SEGM_ANGLE_45) {
        _segmentAngle = M_PI_4; //45º
    } else {
        _segmentAngle = M_PI_2; //90º
    }

    /* calculate number of squares

      q = _segmentDiameter          -> diameter of circle or side of square
      m = max(#rows,#cols)          -> max of #rows, and #col
      S_R = m / q                   -> number of radius segments (without parting)
      f_A = 360 / _segmentAngle     -> factor for number of segmentations through 'crossing'
      S_T = S_R * f_A               -> total number of segments

      Now knowing S_T, f_A, we want to calculate _segmentRadius
      S_R = S_T / f_A   <->
      m / q = S_T / f_A  <->
      f_A * m = q * S_T <->
      _segmentDiameter = q = (f_A * m) / S_T <->
    */
    _segmentDiameter = (2*M_PI /_segmentAngle) * max(_gmap->rowCount(),_gmap->colCount()) / k;

}



vector<int>* RobotCenteredSegmentation::segment(vector<double>* vec, int &segmentCount) {
    //vec assumed to be belief!

    DEBUG_SEGMENT(cout << "RobotCenteredSegmentation.segment"<<endl;);
    DEBUG_SEGMENT(cout << "centerSRadius="<<_centerRadius<<"; high res radius="<<_highResRadius<<endl;);

    assert(_gmap!=NULL);
    int vecSize = _gmap->numFreeCells();

    if (_segmentDiameter == -1 || _segmentAngle == -1) {
        DEBUG_SEGMENT(cout << "Calculating segmentation distance and angle ..."<<flush;);
        //_segmentRadius = calcDefaultSegmentDist();
        calcDefaultDist();
        DEBUG_SEGMENT(cout << "ok"<<endl;);
    }
    DEBUG_SEGMENT(cout << "Segmentation distance: "<<_segmentDiameter<< "; Angle seg distance: "<<_segmentAngle<<endl;);

    if (_rowPos==-1 || _colPos==-1){
        DEBUG_SEGMENT(cout << "Calculating center point based on size.."<<endl;);
        _rowPos = (int)floor(_gmap->rowCount()/2.0f);
        _colPos = (int)floor(_gmap->colCount()/2.0f);
    }
    DEBUG_SEGMENT(cout << "Center point: row="<<_rowPos<< ", col="<<_colPos<<endl;);

    //init segements: 0
    vector<int>* vecSeg = newRegVector(vecSize,false);

    DEBUG_SEGMENT(cout << "GMap==NULL: "<<(_gmap==NULL)<<endl;);
    Pos base =_gmap->getBase();

    DEBUG_SEGMENT(cout << "generating segm vector: "<<flush;);

    segmentCount = 0;
    DEBUG_SEGMENT(cout <<"robotcentered segmentation:"<<endl;);

    int lastHighResCellID = -5;

    //now loop to give each cell a segment id
    int i = 0;
    for (int r=0; r<_gmap->rowCount(); r++) {
        for (int c=0; c<_gmap->colCount(); c++) {
            //skip obstacles
            if (_gmap->isObstacle(r,c)) {
                DEBUG_SEGMENT(cout << "[X]";);
                continue;
            }

            //state id
            int s = 0;

            if (r==_rowPos && c==_colPos) {
                //for the current location we define a specific state
                s = -2;
            } else if (_setBaseState && base.equals(r,c)) {
                //set a specific state for the base
                s = -3;
            } else if (_setHiderState && (*vec)[i]>0.99) {
                //set a specific state for the hider
                s = -4;
            } else {
                //calculate distance and angle from the point with the center
                double d = calcDistance(r,c);

                if (d<=_centerRadius) {
                    //AG121109: define the size of the center state
                    //state is in center
                    s = -2;
                } else if (d<=_highResRadius) {
                    //AG121221: radius from the center on which the highest resolution is used, of bottom cells
                    s = --lastHighResCellID;
                    //note: we count backwards since the normal segment ids are based on angle and distance
                    //later on renumbering is done
                } else {
                    //base state on distance
                    double a = calcAngleDistance(r,c);

                    // calculate the (sub)index
                    int sr = floor(d / _segmentDiameter);
                    int sa = floor(a / _segmentAngle);

                    //total index
                    s = sr * round(2*M_PI/_segmentAngle) + sa;
                }
            }
            //cout << "(d="<<d<<";a="<<a<<";sr="<<sr<<";sa="<<sa<<")";

            (*vecSeg)[i] = s;

            if (s>=segmentCount) segmentCount=s+1;

            DEBUG_SEGMENT(cout << "["<<s<<"]";);

            i++;
        }
        DEBUG_SEGMENT(cout<<endl;);
    }



/*

    //DEBUG_SEGMENT(cout << endl<< "Connection analysis ..."<<endl;);
    //now do con an.
    vector<int>* vecSegCA = connectionAnalysis(vecSeg);

    //DEBUG_SEGMENT(cout << "Update indices ..."<<endl;);
    */

    //update indices
    vector<int>* vecSegSeg = get0BasedSegments(vecSeg, segmentCount);
    DEBUG_SEGMENT(cout << "Segmenting ok"<<endl;);

   /* cout << "vecseg.sz="<<vecSeg->size()<<"; deleting..."<<endl;
    //delete orig vector
    delete vecSeg;
    //delete CA vector
    delete vecSegCA;
    cout << "delete ok"<<endl;*/

    //delete orig vector
    DEBUG_DELETE(cout<<"basesegmenter.deleting 1st segment:"<<flush;);
    delete vecSeg;
    /*//delete CA vector
    DEBUG_DELETE(cout <<"ok"<<endl<<"  delete CA:"<<flush;);
    delete vecSegCA;*/
    DEBUG_DELETE(cout <<"ok"<<endl;);

    return vecSegSeg;
}


bool RobotCenteredSegmentation::segmentsCanChangeWhenSeekerHalt() {
    return false; //only depends on seeker's location
}
