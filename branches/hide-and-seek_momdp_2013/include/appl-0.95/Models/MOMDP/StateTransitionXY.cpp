#include "StateTransitionXY.h"

StateTransitionXY::StateTransitionXY(void) {
}

//ag120229: predefine matrix
StateTransitionXY::StateTransitionXY(vector<vector<SharedPointer<SparseMatrix> > > matrix, vector<vector<SharedPointer<SparseMatrix> > > matrixTr) {
    this->matrix = matrix;
    this->matrixTr = matrixTr;
}


StateTransitionXY::~StateTransitionXY(void) {
}

SharedPointer<SparseMatrix> StateTransitionXY::getMatrix(int a, int x, int xp) {
    return matrix[a][x];
}
SharedPointer<SparseMatrix> StateTransitionXY::getMatrixTr(int a, int x, int xp) {
    return matrixTr[a][x];
}
