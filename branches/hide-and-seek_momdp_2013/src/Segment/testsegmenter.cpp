#include "testsegmenter.h"

TestSegmenter::TestSegmenter(GMap* gmap, int neighb) :
        Segmenter(gmap, neighb) {
}

TestSegmenter::TestSegmenter(int neighb) :
        Segmenter(neighb) {
}


vector<int>* TestSegmenter::segment(vector<double>* vec, int &segmentCount) {
    DEBUG_SEGMENT(cout << "TestSegmenter.segment"<<endl;);

    //init segements: 0
    vector<int>* vecSeg = newRegVector(vec->size(),false);

    DEBUG_SEGMENT(cout << "generating segm vector: "<<flush;);

    //set segments as map node id
    for (unsigned int i=0; i<vec->size(); i++) {
        (*vecSeg)[i] = i;
    }

    //segment count
    segmentCount = vec->size();

    return vecSeg;
}

