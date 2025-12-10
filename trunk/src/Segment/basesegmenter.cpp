#include "Segment/basesegmenter.h"

using namespace std;


BaseSegmenter::BaseSegmenter(GMap* gmap, int neighb) :
        Segmenter(gmap, neighb) {
}

BaseSegmenter::BaseSegmenter(int neighb) :
        Segmenter(neighb) {
}


vector<int>* BaseSegmenter::segment(vector<double>* vec, int &segmentCount) {
    DEBUG_SEGMENT(cout << "BaseSegmenter.segment"<<endl;);

    //init segements: 0
    vector<int>* vecSeg = newRegVector(vec->size(),false);

    DEBUG_SEGMENT(cout << "generating segm vector: "<<flush;);
    /*segmentation:
      value     region
      <0        1
      =0        2
      >0        3
    */
    for (unsigned int i=0; i<vec->size(); i++) {
        double v = (*vec)[i];
        int r=0;
        if (v==0) {
            r = 2;
        } else if (v<0) {
            r = 1;
        } else {
            r = 3;
        }
        (*vecSeg)[i] = r;
        DEBUG_SEGMENT(cout <<r<<", "<<flush;);
    }

    //DEBUG_SEGMENT(cout << endl<< "Connection analysis ..."<<endl;);
    //now do con an.
    vector<int>* vecSegCA = connectionAnalysis(vecSeg);

    //DEBUG_SEGMENT(cout << "Update indices ..."<<endl;);
    //update indices
    vector<int>* vecSegSeg = get0BasedSegments(vecSegCA, segmentCount);
    DEBUG_SEGMENT(cout << "Segmenting ok"<<endl;);

   // cout << "vecseg.sz="<<vecSeg->size()<<"; deleting..."<<endl;
    //delete orig vector
    DEBUG_DELETE(cout<<"basesegmenter.deleting 1st segment:"<<flush;);
    delete vecSeg;
    //delete CA vector
    DEBUG_DELETE(cout <<"ok"<<endl<<"  delete CA:"<<flush;);
    delete vecSegCA;
    DEBUG_DELETE(cout <<"ok"<<endl;);

    return vecSegSeg;
}


bool BaseSegmenter::segmentsCanChangeWhenSeekerHalt() {
    return true; //because base segmenter depends on belief
}
