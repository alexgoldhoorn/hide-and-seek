#ifndef KMEANSSEGMENTER_H
#define KMEANSSEGMENTER_H

#include "segment.h"


/*! The K-means segmenter first segments the values in k groups and then based on neighbouring (connection analysis) in more.
  The implementation is done using OpenCV:
    http://opencv.willowgarage.com/documentation/cpp/clustering_and_search_in_multi-dimensional_spaces.html#cv-kmeans



  */
class KMeansSegmenter: public Segmenter {
public:
    //! K means percentage of cells
    static const float KMEANS_K_S1_PERC = 0.07f;
    //! K means min K
    static const int KMEANS_K_MIN = 2;
    //! K means max K
    static const int KMEANS_K_MAX = 30;
    //! Weight for the value when it is combined with the row and column
    static const float VALUE_WEIGHT = 2.0f;
    //! OpenCV TermCriteria.epsilon: required epsilon
    static const double OPENCV_KMEANS_EPSILON = 0.1;
    //! OpenCV TermCriteria.maxCount: maximum number of iterations
    static const int OPENCV_KMEANS_MAX_COUNT = 20;
    //! OpenCV attempts
    static const int OPENCV_KMEANS_ATTEMPTS = 3;

    /*! KMeanSegmenter for gmap, #neighbours (4 or 8), k, and useRowColInKMeans (default=true, see setUseRowColInKMeans).
      */
    KMeansSegmenter(GMap* gmap, int neighb, int k, bool useRowColInKMeans = true);

    /*! KMeanSegmenter for #neighbours (4 or 8), k, and useRowColInKMeans (default=true, see setUseRowColInKMeans).
      */
    KMeansSegmenter(int neighb, int k, bool useRowColInKMeans = true);

    /*! KMeanSegmenter for #neighbours (4 or 8), and useRowColInKMeans (default=true, see setUseRowColInKMeans).
      */
    KMeansSegmenter(int neighb, bool useRowColInKMeans = true);

    /*! Define to use row and column in the K-means segmentation. Using this will generate regions that already are clustered based on
      their location.
      */
    void setUseRowColInKMeans(bool useRowColInKMeans);

    //! Return if rows and columns are used in K-means segmentation
    bool getUseRowColInKMeans();


    vector<int>* segment(vector<double>* vec, int &segmentCount);

    //! set k
    void setK(int k);

    //! get k (-1 means not set)
    int getK();

    virtual bool segmentsCanChangeWhenSeekerHalt() {
        return true; //because it uses belief
    }


private:
    //! k centroids for k-means
    int _k;

    //! use row and col in k-means
    bool _useRowColInKMeans;
};


#endif // KMEANSSEGMENTER_H
