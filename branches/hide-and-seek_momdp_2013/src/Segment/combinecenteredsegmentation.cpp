#include "combinecenteredsegmentation.h"
#include <cassert>

//! RobotCenteredSegmentation for gmap, #neighbours, segmentation distance (see setSegmentDist) and angle distance (in degrees) (see setAngleDistDeg)
CombineCenteredSegmentation::CombineCenteredSegmentation(GMap* gmap, int neighb, double segmentDiameter,
                                                         double segmentAngleDeg, char distType, double centerRadius, bool setHiderState, double highResRadius )
    : RobotCenteredSegmentation(gmap,neighb,segmentDiameter,segmentAngleDeg,distType,centerRadius, false, setHiderState, highResRadius)
    , _seekerCSegmenter(gmap,neighb,segmentDiameter,segmentAngleDeg,distType,centerRadius, false, setHiderState, highResRadius)
    , _baseCSegmenter(gmap,neighb,segmentDiameter,segmentAngleDeg,distType,centerRadius, false, false, highResRadius)
{
    initBS();
}

//! RobotCenteredSegmentation for #neighbours, segmentation distance (see setSegmentDist) and angle distance (in degrees) (see setAngleDistDeg)
CombineCenteredSegmentation::CombineCenteredSegmentation(int neighb, double segmentDiameter, double segmentAngleDeg,
                                                         char distType, double centerRadius, bool setHiderState, double highResRadius )
    : RobotCenteredSegmentation(neighb, segmentDiameter, segmentAngleDeg, distType, centerRadius, false, setHiderState, highResRadius)
    , _seekerCSegmenter(neighb, segmentDiameter, segmentAngleDeg, distType, centerRadius, false, setHiderState, highResRadius)
    , _baseCSegmenter(neighb, segmentDiameter, segmentAngleDeg, distType, centerRadius, false, false, highResRadius)
{

}

void CombineCenteredSegmentation::initBS() {
    assert(_gmap!=NULL);

    DEBUG_SEGMENT(cout << "CombineCenteredSegmentation.initBS"<<endl;);

    //segment base segmenter (since this is always fixed)
    Pos base = _gmap->getBase();
    _baseCSegmenter.setGMap(_gmap);
    _baseCSegmenter.setPosition(base.row,base.col);
    _baseCSegments = _baseCSegmenter.segment(NULL,_baseCSegmentCount);

    //set map
    _seekerCSegmenter.setGMap(_gmap);

    DEBUG_SEGMENT(cout << "CombineCenteredSegmentation.initBS DONE"<<endl;);
}



//! set position
//CHECK: is this passed really from segment ??
//TODO: pass to _seekerCSegmenter
void CombineCenteredSegmentation::setPosition(int row, int col) {
    Segmenter::setPosition(row,col);
    //pass seeker pos
    _seekerCSegmenter.setPosition(row,col);
}


//! Set GMap
void CombineCenteredSegmentation::setGMap(GMap* gmap) {
    //set map
    Segmenter::setGMap(gmap);
    initBS();

}

vector<int>* CombineCenteredSegmentation::segment(vector<double>* vec, int &segmentCount)  {

    //if seeker at base, then return base segmentation
    if (_gmap->isBase(_rowPos,_colPos)) {
        DEBUG_SEGMENT(cout << "CombineCenteredSegmentation.segment: returning base centered segmentation since seeker is at base" <<endl; );
        segmentCount = _baseCSegmentCount;
        //copy baseCSegments since it will be deleted afterwards
        vector<int>* baseCSVec = new vector<int>(*_baseCSegments);
        return baseCSVec;
    }

    DEBUG_SEGMENT(cout << "CombineCenteredSegmentation.segment: base segments: "<<_baseCSegmentCount<<"; items="<<_baseCSegments->size() <<endl; );

    DEBUG_SEGMENT(cout << "CombineCenteredSegmentation.segment: segmenting centered from seeker"<<endl; );

    //first segment from seeker's pos
    int sSegCount = 0;
    vector<int>* sSeg = _seekerCSegmenter.segment(vec, sSegCount);

    DEBUG_SEGMENT(cout << "Seeker centered segments: " << sSegCount << "; # items: "<< sSeg->size()<<endl;);

    /* since the segmenters both generate numbers from  ( 0, [n-1] ), we can recalculate the new

    states as s_c = s_1 + s_2 * n_1
        with s_c: combined state
             s_1: state 1
             s_2: state 2
             n_1: number of states of 1

    however since some combinations might not occur the segments have to be renumbered
    */

    DEBUG_SEGMENT(cout << "CombineCenteredSegmentation.segment: combining seeker and base centered top states"<<endl; );

    //combined segment
    vector<int> combSeg;

    //assuming sSeg.count==_baseCSegments.count

    for (int i=0; i<sSeg->size(); i++) {
        DEBUG_SEGMENT(cout << "\t- "<<i<<") "<<flush;);
        //seeker centered state
        int sS = (*sSeg)[i];
        DEBUG_SEGMENT(cout << "s_S=" << sS <<flush;);
        //base centered state
        int sB = (*_baseCSegments)[i];
        DEBUG_SEGMENT(cout << "; s_B=" << sB <<flush;);
        //combined state
        int sC = sS + sB * sSegCount;
        DEBUG_SEGMENT(cout << "; s_C=" << sC <<flush;);
        //add to segments
        combSeg.push_back(sC);
        DEBUG_SEGMENT(cout << "." << endl;);
    }

    DEBUG_SEGMENT(cout << "CombineCenteredSegmentation.segment: make segments 0 based"<<endl; );

    //get 0-index based segments
    vector<int>* combSeg0 =  get0BasedSegments(&combSeg, segmentCount);

    //remove segments from seeker segmenter
    delete sSeg;


    return combSeg0;
}


