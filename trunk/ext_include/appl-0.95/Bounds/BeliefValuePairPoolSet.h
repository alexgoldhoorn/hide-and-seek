#ifndef BeliefValuePairPoolSet_H
#define BeliefValuePairPoolSet_H

#include "Bound.h"
#include "Backup.h"
#include "MOMDP.h"
#include "BeliefValuePair.h"
#include "BeliefValuePairPool.h"
#include "PruneBeliefValuePair.h"
#include <exception>
#include <vector>
using namespace std;
using namespace momdp;
namespace momdp {


class BeliefValuePairPoolSet :	public Bound<BeliefValuePair> {
private:

public:
    vector<BeliefValuePairPool*> set;
    vector<BeliefCache *> beliefCacheSet;
    void setBeliefCache(vector<BeliefCache *> p);

    vector<IndexedTuple<BeliefValuePairPoolDataTuple> *> dataTableSet;
    void setDataTable(vector<IndexedTuple<BeliefValuePairPoolDataTuple> *> p);

    SharedPointer<MOMDP> problem;
    void setProblem(SharedPointer<MOMDP> p);
    void setSolver(PointBasedAlgorithm *p);

    BeliefValuePairPoolSet(Backup<BeliefValuePair> *_backupEngine);
    void initialize() ;

    SharedPointer<BeliefValuePair> addPoint(SharedPointer<BeliefWithState> beliefandState, double val);

    virtual void appendOnBackupHandler(BackupCallback _onBackup);

    virtual void removeOnBackupHandler(BackupCallback _onBackup) ;

    virtual ~BeliefValuePairPoolSet(void) ;

    virtual REAL_VALUE getValue(SharedPointer<BeliefWithState>& belief);

    virtual SharedPointer<BeliefValuePair> backup(BeliefTreeNode * node);

};


}

#endif

