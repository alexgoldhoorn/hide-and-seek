#include "BeliefValuePairPoolSet.h"
#include <exception>
using namespace std;


void BeliefValuePairPoolSet::setBeliefCache(vector<BeliefCache *> p) {
    beliefCacheSet = p;
    for(size_t i = 0 ; i < set.size(); i ++) {
        set[i]->setBeliefCache(p[i]);
    }
}

void BeliefValuePairPoolSet::setDataTable(vector<IndexedTuple<BeliefValuePairPoolDataTuple> *> p) {
    dataTableSet = p;
    for(size_t i = 0 ; i < set.size(); i ++) {
        set[i]->setDataTable(p[i]);
    }
}

void BeliefValuePairPoolSet::setProblem(SharedPointer<MOMDP> p) {
    problem = p;
}
void BeliefValuePairPoolSet::setSolver(PointBasedAlgorithm *p) {
    solver = p;
}

BeliefValuePairPoolSet::BeliefValuePairPoolSet(Backup<BeliefValuePair> *_backupEngine) {
    this->setBackupEngine(_backupEngine);
}

void BeliefValuePairPoolSet::initialize() {
    set.resize(problem->XStates->size());
    for(States::iterator iter = problem->XStates->begin(); iter != problem->XStates->end(); iter ++ ) {
        // Should not use BeliefValuePairPool's backup
        BeliefValuePairPool *bound = new BeliefValuePairPool(NULL);
        bound->setProblem(problem);
        bound->setSolver(solver);
        set[iter.index()] = bound;
        bound->pruneEngine = new PruneBeliefValuePair();
        bound->pruneEngine->setup(bound);
    }
}

SharedPointer<BeliefValuePair> BeliefValuePairPoolSet::addPoint(SharedPointer<BeliefWithState> beliefandState, double val) {
    state_val sval = beliefandState->sval;
    SharedPointer<belief_vector>  b = beliefandState->bvec;
    return set[sval]->addPoint(b, val);
}

void BeliefValuePairPoolSet::appendOnBackupHandler(BackupCallback _onBackup) {
    for(size_t i = 0 ; i < set.size(); i ++) {
        set[i]->appendOnBackupHandler(_onBackup);
    }
}
void BeliefValuePairPoolSet::removeOnBackupHandler(BackupCallback _onBackup) {
    for(size_t i = 0 ; i < set.size(); i ++) {
        set[i]->removeOnBackupHandler(_onBackup);
    }
}
BeliefValuePairPoolSet::~BeliefValuePairPoolSet(void) {
}


SharedPointer<BeliefValuePair> BeliefValuePairPoolSet::backup(BeliefTreeNode * node) {
    SharedPointer<BeliefValuePair> result = backupEngine->backup(node);
    for(size_t i = 0 ; i < onBackup.size(); i++) {
        (*onBackup[i])(solver, node, result);
    }
    return result;
}

REAL_VALUE BeliefValuePairPoolSet::getValue(SharedPointer<BeliefWithState>& bns) {
    state_val sval = bns->sval;
    double bestVal;

    bestVal = set[sval]->getValue(bns->bvec);

    return bestVal;

}
