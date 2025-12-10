#ifndef SEGMENT_H
#define SEGMENT_H

#include "Base/hsconfig.h"

#include "HSGame/gmap.h"
#include <vector>


/*! The segmenter is the class that segments the 'map', based on its values.
  The Segmenter is the abstract class that already has a connectionAnalysis method, which
  gives different segments a different number based on connectivity.
  Note: the gmap is used to check the dimension, connectivity and obstacle locations.
  The values of the vector should have the order: first row, then col, i.e. r0c0,r0c1,r0c2,...r1c0,r1c1,....

  */
class Segmenter {
public:

    //! Init with GMap and number of neighbours
    Segmenter(GMap* gmap, int neighb);

    //! Init with number of neighbours.
    Segmenter(int neighb);

    virtual ~Segmenter();

    //! Set GMap
    virtual void setGMap(GMap* gmap);

    /*! return vector with segment for each state
     [i]=j  -> state i is in segment j
     (r,c)-> (row,col) to pass at some segmenters
     */
    virtual std::vector<int>* segment(std::vector<double>* vec, int &segmentCount)=0;

    //! segments current _vecSeg using conneciton analysis
    std::vector<int>* connectionAnalysis(std::vector<int>* vec, bool ignoreObstacles=false);

    /*! based on an integer vector, it changes segment ids such that the segment numbering
    starts at 0 and runs from there (not skipping numbers)
    The segmentCount returns the number
    */
    std::vector<int>* get0BasedSegments(std::vector<int>* vec, int &segmentCount);

    //! Show the categorized map
    void showMap(std::vector<int>* segVec, int xState=-1, int yState=-1);


    //! count connected obstacles
    //int countConnectedObstacles();

    //! set position (given by user)
    //! not necessarily used by segmenter
    virtual void setPosition(int row, int col);

    //! get position
    Pos getPosition();

    //AG120904
    //! returns if segments can changes when the seeker's action was 'halt'
    //! this is used to prevent regenerating a POMDP and relearning the policy
    virtual bool segmentsCanChangeWhenSeekerHalt()=0;

protected:
    /*! generate new vector with same size as _vec
     remember to clear the memory! */
    std::vector<int>* newRegVector(int size, bool init0);

    //! #neighbours: 4 / 8
    int _neighb;

    //! the map used
    GMap* _gmap;

    //! pos
    int _rowPos, _colPos;

private:
    //! check neighbour
    void caChangeItem(std::vector<int>* newRegVec, std::vector<int>* vecSeg, int r, int c, int r1, int c1, int i, bool ignoreObstacles);
};


#endif // SEGMENT_H
