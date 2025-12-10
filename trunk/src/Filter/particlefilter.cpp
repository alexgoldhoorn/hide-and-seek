#include "Filter/particlefilter.h"
#include "Base/hsconfig.h"

#include <cmath>
#include <cassert>
#include <iostream>
#include <cassert>
#include <limits>

#include "exceptions.h"

using namespace std;

ParticleFilter::ParticleFilter(SeekerHSParams* params, ParticleFilterChecker* pfWeighter) :
    _params(params),
    _pfChecker(pfWeighter),
    _randomGenerator(_randomDevice()),
    _normDist(0,1),
    _uniformProbDistr(0,1)
     {
        assert(_params!=nullptr);
        assert(_params->numBeliefStates>1);
        assert(_params->contNextHiderStateStdDev>=0);
        assert(_params->contHiderHeadStdDev>=0);
        assert(_params->hiderStepDistance>=0);
}

void ParticleFilter::initState1(const Eigen::Vector3d& x) {
    assert(_pfChecker!=nullptr);
    //the passed vector should be a legal position
    assert(_pfChecker->isValid(x));

    //init size
    X.resize(NUM_ROWS, _params->numBeliefStates);
    _weight.resize(_params->numBeliefStates,1);

    //set weights to 0
    _weight.setZero();

    //try count
    unsigned int numTries = 0;

    //add noise
    for(unsigned int i = 0; i<_params->numBeliefStates; ++i) {
        numTries = 0;
        //try to find a legal noisy location
        do {
            //set location
            X(0,i) = randomNormValue(x(0), _params->contNextHiderStateStdDev);
            X(1,i) = randomNormValue(x(1), _params->contNextHiderStateStdDev);
            //increase counter
            ++numTries;
        } while (!_pfChecker->isValid(X.col(i)) && numTries<_params->MAX_NUM_TRIES_AFTER_FAILURE);

        if(numTries>=_params->MAX_NUM_TRIES_AFTER_FAILURE) {
            cout << "ParticleFilter::initState1 - WARNING: tried "<<numTries<<" times to add noise to "<<x.transpose()<<endl;
            //set to observation pos
            X(0,i) = x(0);
            X(1,i) = x(1);
        }

        X(2,i) = randomUnValue(2*M_PI,0);
    }
}

void ParticleFilter::initState(Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic> X) {
    assert(X.cols()==_params->numBeliefStates); //TODO: should not be allowed to change, maybe later
    assert(X.rows()==NUM_ROWS);
    this->X = X;

    _weight.resize(_params->numBeliefStates,1);

    //set weights to 0
    _weight.setZero();
}

void ParticleFilter::update(const Eigen::Vector2d& obs,
                    const Pos& seekerPos,
                    double* degeneracyRet,
                    double* weightMeanRet,
                    double* weightStdRet) {

    throw CException(_HERE_,"TODO: this is not taking into account multiple observations.");

#ifdef OLD_CODE
    assert(_pfChecker!=nullptr);
    assert(_weight.rows() == _params->numBeliefStates);
    assert(X.cols() == _params->numBeliefStates);
    //assert correct observation
    assert(_params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES ||
           _params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES_HB ||
           _params->solverType==SeekerHSParams::SOLVER_MULTI_FILTER_ALWAYS_UPDATES_HB_PARTICLE ||
           (obs(0)>=0 && obs(1)>=0)
    );
    //seekerpos has to be set
    assert(seekerPos.isSet());

    double weightSum = 0;
    double weight2Sum = 0;

    uint num0=0;//test

    //calculate weights
    for(unsigned int c = 0; c < X.cols(); ++c) {
        //weighter should decide the weight based on the observation
        _weight(c) = _pfChecker->calcWeight(X.col(c), obs, seekerPos);
        weightSum += _weight(c);
        weight2Sum += pow(_weight(c),2);

        //test
        if (/*std::isinf(_weight(c)) ||*/ _weight(c)==0) {
            DEBUG_FILTER(cout << "0 col "<<c<<" obs: "<<obs.transpose()<<" part: "<<X.col(c).transpose()<<"; w="<<_weight(c)<<endl;);
            ++num0;
        }
    }

    DEBUG_WARN(
        if (num0>0)
            cout << " --> "<<num0<<" 0-weight of "<<X.cols()<<" ("<<(100.0*num0/X.cols())<<"%)"<<endl;
    );

    //AG16011: calculate mean and std.dev.
    if (weightMeanRet!=nullptr) {
        *weightMeanRet = weightSum / _params->numBeliefStates;
        if (weightStdRet!=nullptr) {
            *weightStdRet = (_weight.array() - *weightMeanRet).array().square().sum() / _params->numBeliefStates;
        }
    }

    //degeneracy value:
    double degeneracy = 1.0 / weight2Sum;

    //set return value
    if (degeneracyRet!=nullptr)
        *degeneracyRet = degeneracy;

    //here particles (mean) were used...

    //AG160110: TODO handle case of low weights (see article)
    DEBUG_WARN(
        if (degeneracy > 1000) {
            cout << "Warning! degeneracy = "<<degeneracy<<endl;
        }
    );

    //AG160110: added to prevent the particle weight to reach nan or inf
    /*if (weightSum==0) {
        cout << "WARNING: weight^2 sum is 0"<<endl;
        weightSum=1;
    }*/ // TODO SOMETHING HERE

    //normalize
    _weight /= weightSum;

    //now create accumulation distribution
    vector<double> weightAcc(_params->numBeliefStates);
    weightAcc[0] = _weight(0);
    for(int i = 1; i < _weight.rows(); ++i) {
        weightAcc[i] = weightAcc[i-1]+_weight(i);
    }

    /*DEBUG_FILTER(
        cout << "weights:"<<endl;
        for(int i = 0; i < _weight.rows(); ++i)
            cout<<_weight(i)<<",";
        cout <<endl<< "accumulated:"<<endl;
        for(double w : weightAcc)
            cout << w <<",";
        cout<<endl;
    );*/

    //new X
    Eigen::Matrix<double, NUM_ROWS, Eigen::Dynamic> Xnew;
    Xnew.resize(NUM_ROWS, _params->numBeliefStates);

    //todo: this is resample, maybe different function
    //random number
    for(unsigned int i=0; i<_params->numBeliefStates; ++i) {
        //get random p
        double p = _uniformProbDistr(_randomGenerator);
        //get index
        auto lower = lower_bound(weightAcc.begin(), weightAcc.end(), p);
        assert(lower!=weightAcc.end());
        int index = (int)( (lower - weightAcc.begin()) );
        //DEBUG_FILTER(cout <<"p="<<p<<", index:"<<index<<endl;);

        //add particle
        Xnew.col(i) = X.col(index);
    }

    X = Xnew;
#endif
}

void ParticleFilter::updateMulti(const pomcp::HSObservation& obs,
                    double* degeneracyRet,
                    double* weightMeanRet,
                    double* weightStdRet) {


    assert(_pfChecker!=nullptr);
    assert(_weight.rows() == _params->numBeliefStates);
    assert(X.cols() == _params->numBeliefStates);
    //assert correct observation
    assert(_params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES ||
           _params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES_HB ||
           _params->solverType==SeekerHSParams::SOLVER_MULTI_FILTER_ALWAYS_UPDATES_HB_PARTICLE ||
           obs.hasVisibleHider()
    );
    //seekerpos has to be set, done in HSObservation generation
    //assert(seekerPos.isSet());

    double weightSum = 0;
    double weight2Sum = 0;

    uint num0=0;//test


    //calculate weights
    for(unsigned int c = 0; c < X.cols(); ++c) {
        //weighter should decide the weight based on the observation

        //AG160215: now loop all observations and choose highest weight
        //AG160223: changed from max to min ->
        //AG161007: made option to aggregate in different ways
        double aggW = 0;
        switch (_params->multiSeekerObsCombType) {
            case SeekerHSParams::MULTI_SEEKER_OBS_COMB_MIN:
                aggW =  std::numeric_limits<double>::max();
                break;
            case SeekerHSParams::MULTI_SEEKER_OBS_COMB_MAX:
                aggW = -1.0;
                break;
            case SeekerHSParams::MULTI_SEEKER_OBS_COMB_AVG:
                aggW = 0;
                break;
            default:
                throw CException(_HERE_,"Unknown obs. combo type");
                break;
        }

        /*double minW =  std::numeric_limits<double>::max(); //-1.0;
        double maxW =  -1.0;
        double avgW = 0;*/

        for(size_t i=0; i<obs.size(); i++) {
            //get the state
            const pomcp::HSState* state = obs.getObservationState(i);
            //calculate the weight
            double w = _pfChecker->calcWeight(X.col(c), state->hiderPos, state->seekerPos, obs);
            /*//check max
            if (w<minW)
                minW = w;
            if (w>maxW)
                maxW = w;
            avgW += w;*/
            switch (_params->multiSeekerObsCombType) {
                case SeekerHSParams::MULTI_SEEKER_OBS_COMB_MIN:
                    if (w<aggW)
                        aggW = w;
                    break;
                case SeekerHSParams::MULTI_SEEKER_OBS_COMB_MAX:
                    if (w>aggW)
                        aggW = w;
                    break;
                case SeekerHSParams::MULTI_SEEKER_OBS_COMB_AVG:
                    aggW += w;
                    break;
            }
        }

        if (_params->multiSeekerObsCombType==SeekerHSParams::MULTI_SEEKER_OBS_COMB_AVG)
            aggW /= obs.size();

        _weight(c) = aggW;
        weightSum += aggW;
        weight2Sum += pow(aggW,2);

        //test
        if (/*std::isinf(_weight(c)) ||*/ _weight(c)==0) {
            DEBUG_FILTER(cout << "0 col "<<c<<" obs: "<<obs.transpose()<<" part: "<<X.col(c).transpose()<<"; w="<<_weight(c)<<endl;);
            ++num0;
        }
    }

    DEBUG_WARN(
        if (num0>0)
            cout << "PF "<<num0<<" 0-weight of "<<X.cols()<<" ("<<(100.0*num0/X.cols())<<"%)"<<endl;
    );

    //AG16011: calculate mean and std.dev.
    if (weightMeanRet!=nullptr) {
        *weightMeanRet = weightSum / _params->numBeliefStates;
        if (weightStdRet!=nullptr) {
            *weightStdRet = (_weight.array() - *weightMeanRet).array().square().sum() / _params->numBeliefStates;
        }
    }

    //degeneracy value:
    double degeneracy = 1.0 / weight2Sum;

    //set return value
    if (degeneracyRet!=nullptr)
        *degeneracyRet = degeneracy;

    //here particles (mean) were used...

    //AG160110: TODO handle case of low weights (see article)
    DEBUG_WARN(
        if (degeneracy > 1000) {
            cout << "Warning! degeneracy = "<<degeneracy<<endl;
        }
    );

    //AG160110: added to prevent the particle weight to reach nan or inf
    /*if (weightSum==0) {
        cout << "WARNING: weight^2 sum is 0"<<endl;
        weightSum=1;
    }*/ // TODO SOMETHING HERE

    //normalize
    _weight /= weightSum;

    //now create accumulation distribution
    vector<double> weightAcc(_params->numBeliefStates);
    weightAcc[0] = _weight(0);
    for(int i = 1; i < _weight.rows(); ++i) {
        weightAcc[i] = weightAcc[i-1]+_weight(i);
    }

    /*DEBUG_FILTER(
        cout << "weights:"<<endl;
        for(int i = 0; i < _weight.rows(); ++i)
            cout<<_weight(i)<<",";
        cout <<endl<< "accumulated:"<<endl;
        for(double w : weightAcc)
            cout << w <<",";
        cout<<endl;
    );*/

    //new X
    Eigen::Matrix<double, NUM_ROWS, Eigen::Dynamic> Xnew;
    Xnew.resize(NUM_ROWS, _params->numBeliefStates);

    //todo: this is resample, maybe different function
    //random number
    for(unsigned int i=0; i<_params->numBeliefStates; ++i) {
        //get random p
        double p = _uniformProbDistr(_randomGenerator);
        //get index
        auto lower = lower_bound(weightAcc.begin(), weightAcc.end(), p);
        assert(lower!=weightAcc.end());
        int index = (int)( (lower - weightAcc.begin()) );
        //DEBUG_FILTER(cout <<"p="<<p<<", index:"<<index<<endl;);

        //add particle
        Xnew.col(i) = X.col(index);
    }

    X = Xnew;
}

void ParticleFilter::predict() {
    assert(_pfChecker!=nullptr);
    // here only move
    // TODO: maybe incorporate with sampling, i.e. before sampling
    // ALSO check mont carlo localization

    //AG16022: count number of failures
    unsigned int retryCount = 0;

    for(unsigned int c = 0; c < X.cols(); ++c) {

        double newHead, l, oldX, oldY;
        //try count
        unsigned int numTries = 0;

        //set old x,y
        oldX = X(0,c);
        oldY = X(1,c);

        do {
            // new heading
            if (_params->useHeading)
                newHead = randomNormValue(X(2,c), _params->contHiderHeadStdDev);
            else //use completly random heading
                newHead = randomUnValue(2*M_PI,0);

            // noise for step
            l = randomNormValue(_params->hiderStepDistance, _params->contNextHiderStateStdDev);
            // now change value h
            X(0,c) = oldX + l*cos(newHead);
            X(1,c) = oldY + l*sin(newHead);
            //ncrease number of tries
            ++numTries;
        } while (!_pfChecker->isValid(X.col(c)) && numTries < _params->MAX_NUM_TRIES_AFTER_FAILURE);

        if(numTries>=_params->MAX_NUM_TRIES_AFTER_FAILURE) {
            DEBUG_FILTER(cout << "ParticleFilter::predict - WARNING: tried "<<numTries<<" times to move "<<oldX<<" "<<oldY<<endl;);
            //set to observation pos
            X(0,c) = oldX;
            X(1,c) = oldY;

            ++retryCount;
        }

        X(2,c) = newHead;
    }

    if (retryCount>0) {
            DEBUG_WARN(cout << "ParticleFilter::predict - WARNING: "<<retryCount<<" times the old particle location was used instead of a propogation (which was tried "<<
                       _params->MAX_NUM_TRIES_AFTER_FAILURE<<" times)"<<endl;);
    }
}

const Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic>* ParticleFilter::getParticles() const {
    //cout <<"get particles, printing before:"<<endl<<X<<endl<<"..returning.."<<endl;
    //return
    const Eigen::Matrix<double, NUM_ROWS, Eigen::Dynamic>* retx = const_cast<const Eigen:: Matrix<double, NUM_ROWS, Eigen::Dynamic>*>(&X);
    //cout<<"before returning, from ref const:"<<endl<<*retx<<endl;
    return retx;
}

Eigen::Vector3d ParticleFilter::getMeanParticle() const {
    return X.rowwise().mean();
}

Eigen::Matrix3d ParticleFilter::getCovParticle() const {
    auto ones = Eigen::MatrixXd::Constant(_params->numBeliefStates, _params->numBeliefStates, 1);
    auto Xt = X.transpose();
    Eigen::MatrixXd d = Xt - ones * Xt / (1.0*_params->numBeliefStates);

    //covariance
    Eigen::Matrix3d cov = d.transpose() * d / (1.0*_params->numBeliefStates-1);

    return cov;
}

Eigen::Matrix<double, Eigen::Dynamic, 1> ParticleFilter::getWeights() const {
    return _weight;
}

void ParticleFilter::setParticleFilterChecker(ParticleFilterChecker *pfChecker) {
    assert(pfChecker!=nullptr);
    _pfChecker = pfChecker;
}
