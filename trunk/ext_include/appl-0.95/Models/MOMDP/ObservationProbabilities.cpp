#include "ObservationProbabilities.h"

ObservationProbabilities::ObservationProbabilities(void) {
}

ObservationProbabilities::~ObservationProbabilities(void) {
}


//ag120229: predefine matrix
ObservationProbabilities::ObservationProbabilities(vector<vector<SharedPointer<SparseMatrix> > > matrix, vector<vector<SharedPointer<SparseMatrix> > > matrixTr) {
    this->matrix = matrix;
    this->matrixTr = matrixTr;
}



SharedPointer<SparseMatrix> ObservationProbabilities::getMatrix(int a, int x) {
    return matrix[a][x];
}
SharedPointer<SparseMatrix> ObservationProbabilities::getMatrixTr(int a, int x) {
    return matrixTr[a][x];
}

