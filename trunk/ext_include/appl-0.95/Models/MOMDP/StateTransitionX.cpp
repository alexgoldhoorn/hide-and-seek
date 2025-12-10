#include "StateTransitionX.h"
#include "MOMDP.h"

StateTransitionX::StateTransitionX(void) {
}


//ag120229: predefine matrix
StateTransitionX::StateTransitionX(vector<vector<SharedPointer<SparseMatrix> > > matrix, vector<vector<SharedPointer<SparseMatrix> > > matrixTr) {
    this->matrix = matrix;
    this->matrixTr = matrixTr;
}

StateTransitionX::~StateTransitionX(void) {
}

// (unobserved states, observed states)
SharedPointer<SparseMatrix> StateTransitionX::getMatrix(int a, int x) {
    return matrix[a][x];
}
SharedPointer<SparseMatrix> StateTransitionX::getMatrixTr(int a, int x) {
    return matrixTr[a][x];
}
