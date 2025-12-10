
#include "Utils/normalbivardist.h"

#include "Utils/generic.h"

#include <iostream>
using namespace std;

/*
//! This part is in order to generate Normal vectors
namespace Eigen {
namespace internal {
template<typename Scalar>
//! normal distribution generator (with mu=0, std.dev.=1), used by Eigen::MatrixXd::NullaryExpr
struct NormalDist
{
//  EIGEN_EMPTY_STRUCT_CTOR(NormalDist)

    //for randomizer
    std::random_device _randomDevice;
    std::mt19937 _randomGenerator;
    std::normal_distribution<double> _gausDistr;

    NormalDist() : _randomGenerator(_randomDevice()),_gausDistr(0, 1) {}

    NormalDist(const NormalDist&) : _randomGenerator(_randomDevice()),_gausDistr(0, 1) {}

    template<typename Index>
      inline const Scalar operator() (Index, Index = 0)  { return _gausDistr(_randomGenerator); }
    };

    template<typename Scalar>
    struct functor_traits<NormalDist<Scalar> >
    { enum { Cost = 50 * NumTraits<Scalar>::MulCost, PacketAccess = false, IsRepeatable = false }; };

} // end namespace internal
} // end namespace Eigen
*/


NormalBivariateDist::NormalBivariateDist() : _randomGenerator(_randomDevice()),
                        _normalDist(0,1), _mean(2), _covar(2,2), _normTransform(2,2), _sampMat(2,1) {
    //default mean: (0,0)
    _mean << 0,0;
    //default covar: (1,0; 0,1)
    _covar << 1,0,0,1;
}


NormalBivariateDist::~NormalBivariateDist() {
}


void NormalBivariateDist::setMeanCovar(const std::vector<double> &meanVec, const std::vector<double> &covVec) {
    setMean(meanVec);
    setCovar(covVec);
}

void NormalBivariateDist::setMean(const std::vector<double> &meanVec) {
    assert(meanVec.size()==2);
    _mean << meanVec[0], meanVec[1];
}

void NormalBivariateDist::setMean(double x, double y) {
    _mean << x, y;
}

void NormalBivariateDist::setMean(const Pos &pos) {
    _mean << pos.colDouble(), pos.rowDouble();
    //cout << "Mean:"<<endl<<_mean<<endl;
}

void NormalBivariateDist::setCovar(const std::vector<double> &covVec) {
    assert(covVec.size()==4);
    _covar << covVec[0], covVec[1], covVec[2], covVec[3];

    //cout <<"Covar:"<<endl<<_covar<<endl;

    //Now get A in AA'=S    (S=sigma, covar)
    Eigen::LLT<Eigen::MatrixXd> cholSolver(_covar);

    // We can only use the cholesky decomposition if
    // the covariance matrix is symmetric, pos-definite.
    // But a covariance matrix might be pos-semi-definite.
    // In that case, we'll go to an EigenSolver
    if (cholSolver.info()==Eigen::Success) {
        // Use cholesky solver
        _normTransform = cholSolver.matrixL();
        //std::cout<<"CholSolver succeeded"<<std::endl;
    } else {
        // Use eigen solver
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigenSolver(_covar);
        _normTransform = eigenSolver.eigenvectors()
                     * eigenSolver.eigenvalues().cwiseSqrt().asDiagonal();
        //std::cout<<"CholSolver failed"<<std::endl;
    }
}

void NormalBivariateDist::getRandPoint(double &x, double &y) {
    //TODO: check if mean + cov set

    /*std::cout << "generating rand point, with, mean="<<endl<<_mean<<endl<<"Covariance:"<<endl
              <<_covar<<endl<<"Norm Transforms: "<<endl<<_normTransform<<endl;*/

    //Eigen::MatrixXd sampMat(2,4); // = Eigen::MatrixXd::NullaryExpr(size,nn,randN);
    _sampMat << _normalDist(_randomGenerator), _normalDist(_randomGenerator);
    //std::cout <<"sample mat:"<<endl<<_sampMat<<endl;

    Eigen::MatrixXd samples = (_normTransform
                             * _sampMat).colwise()
                             + _mean;
    //cout <<"out: "<<endl<<samples<<endl<<"size="<<samples.size()<<" rowsxcols= "<<samples.rows()<<"x"<<samples.cols()<<endl;

    assert(samples.size()==2);

    x = samples(0,0);
    y = samples(1,0);
}

Pos NormalBivariateDist::getRandPoint() {
    double r,c;
    getRandPoint(c,r);
    return Pos(r,c);
}


void NormalBivariateDist::getRandPoint(const std::vector<double> &meanVec, const std::vector<double> &covVec, double &x, double &y) {
    setMeanCovar(meanVec, covVec);
    getRandPoint(x,y);
}
