#ifndef BeliefValuePairPool_H
#define BeliefValuePairPool_H

#include "Bound.h"
#include "Backup.h"
#include "Tuple.h"
#include "IndexedTuple.h"
#include "BeliefValuePair.h"
#include "PruneBeliefValuePair.h"
#include <exception>
#include <list>
#include <vector>
#include <stdexcept>


using namespace std;
using namespace momdp;
namespace momdp {

#define CORNER_EPS (1e-6)
#define MIN_RATIO_EPS (1e-10)

class BeliefValuePairPoolDataTuple :public Tuple {
public:
    int UB_ACTION; /*the max ub action*/
};


class BeliefValuePairPool :	public Bound<BeliefValuePair> {
public:
    BeliefCache *beliefCache;
    void setBeliefCache(BeliefCache *p);
    IndexedTuple<BeliefValuePairPoolDataTuple> *dataTable;
    void setDataTable(IndexedTuple<BeliefValuePairPoolDataTuple> *p);

    SharedPointer<MOMDP> problem;
    void setProblem(SharedPointer<MOMDP> p);
    void setSolver(PointBasedAlgorithm *p) ;

    BeliefValuePairPool(Backup<BeliefValuePair> *_backupEngine) ;
    PruneBeliefValuePair* pruneEngine;

    virtual ~BeliefValuePairPool(void);

    virtual REAL_VALUE getValue(SharedPointer<Belief>& belief);
    double getValue_NoInterpolation(const belief_vector& b);

    virtual SharedPointer<BeliefValuePair> backup(BeliefTreeNode * node);


    unsigned int cornerPointsVersion;
    DenseVector cornerPoints;


    list<SharedPointer<BeliefValuePair> > points;
    // update helper functions
    SharedPointer<BeliefValuePair> addPoint(SharedPointer<belief_vector>&  b, double val);
    int whichCornerPoint(const SharedPointer<belief_vector>&  b) const;

private:
    void printCorners() const;


};


}

#endif

