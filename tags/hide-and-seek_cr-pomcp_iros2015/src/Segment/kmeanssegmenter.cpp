#include "Segment/kmeanssegmenter.h"

#include <iostream>
#include "opencv2/core/core.hpp"

#include "Utils/generic.h"

using namespace cv;
using namespace std;

KMeansSegmenter::KMeansSegmenter(GMap* gmap, int neighb, int k, bool useRowColInKMeans) :
        Segmenter(gmap, neighb) { //maybe more params??
    _k = k;
    _useRowColInKMeans = useRowColInKMeans;
}

KMeansSegmenter::KMeansSegmenter(int neighb, int k, bool useRowColInKMeans) :
        Segmenter(neighb) { //maybe more params??
    _k = k;
    _useRowColInKMeans = useRowColInKMeans;
}

KMeansSegmenter::KMeansSegmenter(int neighb, bool useRowColInKMeans) :
        Segmenter(neighb) { //maybe more params??
    _k = -1;
    _useRowColInKMeans = useRowColInKMeans;
}

void KMeansSegmenter::setUseRowColInKMeans(bool useRowColInKMeans) {
    _useRowColInKMeans = useRowColInKMeans;
}

bool KMeansSegmenter::getUseRowColInKMeans() {
    return _useRowColInKMeans;
}

//! set k
void KMeansSegmenter::setK(int k) {
    _k = k;
}

//! get k (-1 means not set)
int KMeansSegmenter::getK() {
    return _k;
}


vector<int>* KMeansSegmenter::segment(vector<double> *vec, int &segmentCount) {
    DEBUG_HS(cout << "KMeansSegmenter.segment"<<endl;);

    //calculate K if not set
    if (_k == -1) {
        /* An estimate for K is done by:
         *      s1 = a * #free_cells
         *      s2 = #free / #obst
         *      k = min(s1,s2)
         *      k = max(min(k,MAX_K), MIN_K) //have it between MIN_K and MAX_K
        */
        DEBUG_SEGMENT(cout << "k-means calculating K: "<<flush;);
        int freeCells = _gmap->numFreeCells();
        int totalCells = _gmap->width()*_gmap->height();
        int obstCells = totalCells-freeCells;

        //calc scores, later get min
        int s1 = round(KMEANS_K_S1_PERC*freeCells);
        int s2 = round(freeCells / obstCells);        
        _k = min(s1,s2);
        if (_k<KMEANS_K_MIN) {
            _k = KMEANS_K_MIN;
        } else if (_k>KMEANS_K_MAX) {
            _k = KMEANS_K_MAX;
        }

        DEBUG_SEGMENT(cout << "#free="<<freeCells<<";#obst="<<obstCells<<"->#total="<<totalCells<<",s1="<<s1<<",s2="<<s2<<",k="<<_k<<endl;);
    }

    DEBUG_HS(cout << " k-means: k="<<_k<<endl;);

    //init segements vector
    vector<int>* vecSeg = newRegVector(vec->size(),false);


    /* CV Matrix type:
    CV_<bit_depth>(S|U|F)C<number_of_channels>

    S = Signed integer
    U = Unsigned integer
    F = Float

    E.g.: CV_8UC1 means an 8-bit unsigned single-channel matrix,
          CV_32FC2 means a 32-bit float matrix with two channels.

    Using: http://opencv.willowgarage.com/documentation/cpp/clustering_and_search_in_multi-dimensional_spaces.html#cv-kmeans
      samples: Floating-point matrix of input samples, one row per sample
        -> first use only the 'score' (reward or reward*belief) then maybe also the row and col !! -> that should already give geographical localization

      double kmeans(const Mat& samples, int clusterCount, Mat& labels, TermCriteria termcrit, int attempts, int flags, Mat* centers)

      //try different flags -> for init
      */
            //_gmap->countFreeCells()


    //convert data for OpenCV Matrix
    DEBUG_SEGMENT(cout<<"Preparing for OpenCV.kmeans.. "<<flush;);
    //adding the location of the cells in the vectors of for the k-means such that also physical
    //distance is taken into account, BUT might have to be tuned.
    float* data = new float[vec->size()*3];
    int i = 0;
    for (int r=0; r<_gmap->rowCount(); r++) {
        for (int c=0; c<_gmap->colCount(); c++) {
            if (!_gmap->isObstacle(r,c)) {
                //2D data in matrix
                data[i*3] = VALUE_WEIGHT * (*vec)[i];
                data[i*3 + 1] = r;
                data[i*3 + 2] = c;

                i++;
            }
        }
    }

    //generate the CV matrix
    //ag note: CV_32F -> float, in double is not accepted by OpenCV.kmeans
    Mat mat(vec->size(),3, CV_32F, data);

    //generate output matrices
    Mat matLabel, matCentroids;
    //set criteria for kmeans
    TermCriteria termCriteria;
    termCriteria.type = TermCriteria::MAX_ITER + TermCriteria::EPS;
    termCriteria.epsilon = OPENCV_KMEANS_EPSILON;
    termCriteria.maxCount = OPENCV_KMEANS_MAX_COUNT;

    DEBUG_SEGMENT(cout << "ok"<<endl<<"Data:"<<endl<< mat<<endl;);

    DEBUG_SEGMENT(cout << "Running k-means ..."<<endl;);

    double compactness = kmeans(mat, _k, matLabel, termCriteria, OPENCV_KMEANS_ATTEMPTS, KMEANS_RANDOM_CENTERS, matCentroids);

#ifdef DEBUG_SEGMENT_ON
    cout << "done, with compactness: "<<compactness<<endl;

    cout <<"Categories:"<<endl<<matLabel<<endl
        <<"Centroids:"<<endl<< matCentroids<<endl;

    cout << "Category and items:"<<endl;
    for(int c=0;c<_k;c++) {
        cout << " * "<<c<<") ";
        float avg=0;
        unsigned int n = 0;
        for (int i=0;i<matLabel.rows;i++) {
            if (matLabel.at<int>(i,0)==c){
                float f = mat.at<float>(i,0);
                cout << f<<", ";
                avg +=f;
                n++;
            }
        }
        cout <<" -> "<< (avg/n)<< endl;
    }
#endif

    delete[] data;

    //set function output, the labels of the segments
    segmentCount = _k;
    for (int i=0;i<matLabel.rows;i++) {
        (*vecSeg)[i] = matLabel.at<int>(i,0);
    }


    DEBUG_SEGMENT(cout << endl<< "Connection analysis ..."<<endl;);
    //now do con an.
    vector<int>* vecSegCA = connectionAnalysis(vecSeg);

    DEBUG_SEGMENT(cout << "Update indices ..."<<endl;);
    //update indices
    vector<int>* vecSegSeg = get0BasedSegments(vecSegCA, segmentCount);

    DEBUG_SEGMENT(cout << "Segmenting ok"<<endl;);


    //delete orig vector
    DEBUG_DELETE(cout<<"basesegmenter.deleting 1st segment:"<<flush;);
    delete vecSeg;
    //delete CA vector
    DEBUG_DELETE(cout <<"ok"<<endl<<"  delete CA:"<<flush;);
    delete vecSegCA;
    DEBUG_DELETE(cout <<"ok"<<endl;);

    return vecSegSeg;
}

bool KMeansSegmenter::segmentsCanChangeWhenSeekerHalt() {
    return true; //because it uses belief
}
