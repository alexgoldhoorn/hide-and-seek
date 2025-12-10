
#include "PeoplePrediction/personpathpredconsumer.h"

#include "exceptions.h"
#include "HSGame/gmap.h"

#include <cmath>

#include <iostream>

using namespace std;

PersonPathPredConsumer::PersonPathPredConsumer() : _lastStepUsedPred(false) {
}

PersonPathPredConsumer::~PersonPathPredConsumer() {
}


void PersonPathPredConsumer::setPredPath(const std::vector<std::vector<double>>& pointMeanVec, const std::vector<std::vector<double>>& pointCovVec) {
    if (pointMeanVec.size()!=pointCovVec.size()) {
        throw CException(_HERE_, "The point mean vector and cov. vector must have the same size!");
    }
    //copy to global vars
    _pointMeanVec = pointMeanVec;
    _pointCovVec = pointCovVec;
}


/*void PersonPathPredConsumer::getPathPoint(unsigned int t, Eigen::VectorXd& mean, Eigen::MatrixXd& covar) {
    if (t >= _pointMeanVec.size()) {
        throw CException(_HERE_, "The index (t) is exceeding the vector size.");
    }

    //mean
    vector<double> meanVec = _pointMeanVec[t];
    assert(meanVec.size()==2);
    mean << meanVec[0], meanVec[1];
    //cov
    vector<double> covVec = _pointCovVec[t];
    assert(covVec.size()==4);
    mean << covVec[0], covVec[1], covVec[2], covVec[3];
}*/

void PersonPathPredConsumer::getRandPredPoint(long t, double &r, double &c) {
    if (t==0) {
        throw CException(_HERE_, "Cannot predict for t=0, this is 'now'.");
    }
    if (t-1 > (int)_pointMeanVec.size()) {
        throw CException(_HERE_, "The index (t-1) is exceeding the vector size.");
    }

    //TODO: NOT SURE IF THIS IS CORRECT!!! -> NOW IT is possible that the next step is much further than the expected step..
    //get rand pointon predicted locatin
    _normalBivarDist.getRandPoint(_pointMeanVec[t-1], _pointCovVec[t-1], c, r);
    _lastStepUsedPred = true;
}

bool PersonPathPredConsumer::predictionAvailable(long t, const double &r, const double &c) {
    if (t==0) {
        throw CException(_HERE_, "Cannot predict for t=0, this is 'now'.");
    }

    //check if available for that time step
    bool predAvail = (t -1 < (int)_pointMeanVec.size());

    if ( predAvail && !_lastStepUsedPred && t>1 ) {
        //check if current location (r,c) is close to previously predicted location (t-2)
        vector<double> meanVec = _pointMeanVec[t-2];
        vector<double> covVec = _pointCovVec[t-2];
        //now check max distance
        //TODO 'tune' which we accept
        double d = GMap::distanceEuc(meanVec[1],meanVec[0],r,c);

        //now should use covariance to check if inside the expected area, but should be based on x and y,
        //now easiest: biggest of sigma_xx and sigma_yy
        double sigma = 0;

        //TODO: NOT SURE IF THIS IS CORRECT!!! -> NOW IT is possible that the next step is much further than the expected step..
        if (covVec[0]<covVec[3]) {
            sigma = sqrt(covVec[3]);
        } else {
            sigma = sqrt(covVec[0]);
        }

        if (d > (sigma)*2) {
            //outside of std.dev.*2, so ignore
            predAvail = false;
        }

        //cout<<(predAvail?"!":"-")<<"d="<<d<<";s="<<sigma<<(predAvail?"!":"-")<<flush;
    } //else cout <<"[-]"<<flush;

    return predAvail;
}

/*void PersonPathPredConsumer::resetTime(unsigned int t) {
    _t = t;
    _lastStepUsedPred = false;
}

void PersonPathPredConsumer::increaseTime() {
    _t++;
}
*/
