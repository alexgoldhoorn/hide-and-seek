#include "Filter/particlefilterplayer.h"

#include <cmath>
#include <cassert>
#include <iostream>

#ifdef DEBUG_FILTER_WRITE_PARTICLES_ON
#include <fstream>
#endif

#include <eigen3/Eigen/Core>

#include "exceptions.h"
#include "Base/hsconfig.h"
#include "Utils/generic.h"

#include "POMCP/hsobservation.h"

using namespace std;
using namespace hsutils;
using namespace pomcp;

ParticleFilterPlayer::ParticleFilterPlayer(SeekerHSParams* params) :
    AutoPlayer(params), _pf(params), _pfChecker(nullptr),
    _beliefProbMatrix(nullptr), _beliefProbMatrixUptodate(false),
    _hsSimulator(nullptr)
{
    DEBUG_FILTER(cout<<"ParticleFilter"<<endl;);
}

ParticleFilterPlayer::~ParticleFilterPlayer() {
    delete _pfChecker;
    delete _hsSimulator;

    //delete hist
    if (_beliefProbMatrix!=nullptr) {
        FreeDynamicArray<double>(_beliefProbMatrix, _map->rowCount());
    }
}

bool ParticleFilterPlayer::initBeliefRun() {
    assert(playerInfo.currentPos.isSet());
    //checker should not have been set yet
    assert(_pfChecker==nullptr);

    if (_params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_REQ_VISIB
            && !playerInfo.hiderObsPosWNoise.isSet())
        throw CException(_HERE_,"ParticleFilterPlayer::initBeliefRun: observation required (since type is SOLVER_FILTER_PARTICLE_REQ_VISIB)");

    DEBUG_FILTER(cout<<"Initializing ParticleFilter:"<<endl);

    /*
    //set motion noise based on configuration
    _pf.setStepNoiseStd( _params->contNextHiderStateStdDev );
    DEBUG_FILTER(cout << "Motion noise (step noise): "<<_params->contNextHiderStateStdDev<<endl;);

    //set obs noise
    //TODO set obs noise ?? (_params->contHiderObsStdDev);
    //DEBUG_FILTER(cout << "Obs noise (R): "<<endl<<R<<endl;);

    //set heading noise
    _pf.setHeadNoiseStd( _params->contHiderHeadStdDev );
    DEBUG_FILTER(cout << "Heading noise: "<<_params->contHiderHeadStdDev<<endl;);

    //set step size
    _pf.setStepSize( _params->hiderStepDistance );
    DEBUG_FILTER(cout << "Step Distance: "<<_params->hiderStepDistance<<endl;);

    //set number of particles
    //TODO: maybe later on use specific variable name (instead of belief states)
    _pf.setNumParticles( _params->numBeliefStates );
    DEBUG_FILTER(cout << "Num. particles: "<<_params->numBeliefStates<<endl;);
    */

    //set checker
    _pfChecker = new HSParticleFilterChecker(_params, _map, &playerInfo);
    _pf.setParticleFilterChecker(_pfChecker);

    //now initialize the state
#ifdef OLD_CODE
    if (playerInfo.hiderObsPosWNoise.isSet()) {
        //set the observation, internally PF will generate particles
        Eigen::Vector3d obsVec = playerInfo.hiderObsPosWNoise.toEigenVector<3>();
        _pf.initState1(obsVec);
    } else {
        //matrix X
        Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic> X;
        //generate initial states
        X = genAllInitStates(playerInfo.currentPos, _params->numBeliefStates);
        //set init states
        _pf.initState(X);
    }
#endif

    //AG160211: use the POMCP observation object to store and obtain the observations to initialize the belief
    //create HSSimulator
    _hsSimulator = new HSSimulatorCont(_map, _params);
    //result X
    Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic> X = genAllInitStates();
    //set init states
    _pf.initState(X);

    //belief matrix should be updated
    _beliefProbMatrixUptodate = false;

    return true;
}

Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic> ParticleFilterPlayer::genAllInitStates() {
    assert(_hsSimulator!=nullptr);
    assert(_params->numBeliefStates>0);

    //create observation
    HSObservation obs(&playerInfo, _playerInfoVec);
    //generate states
    vector<State*> allInitStates = _hsSimulator->genAllInitStates(&obs, _params->numBeliefStates);

    //create output matrix
    Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic> resMatrix;
    resMatrix.resize(ParticleFilter::NUM_ROWS, _params->numBeliefStates);

    //generate list
    for(unsigned int i=0; i<_params->numBeliefStates; i++) {
        //get state
        HSState* hsState = HSState::castFromState(allInitStates[i]);

        //set, and add random offset
        resMatrix(0,i) = hsState->hiderPos.rowDouble();
        resMatrix(1,i) = hsState->hiderPos.colDouble();

        //set heading
        resMatrix(2,i) = randomDouble(0, 2*M_PI);

        //AG160217: the state should be deleted
        delete hsState;
    }

    return resMatrix;
}

#ifdef OLD_CODE
Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic> ParticleFilterPlayer::genAllInitStates(const Pos& robotPos, int n) {
    DEBUG_FILTER(cout<<"ParticleFilterPlayer::genAllInitStates ("<<n <<") "<<endl;);
    assert(robotPos.isSet());
    assert(n>0);

    //AG160211: changed to use probability of visibility (also using max distance)

    //check invisible poses
    //vector<Pos> invisPosVec = _map->getInvisiblePoints(robotPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);

    //create output
    Eigen::Matrix<double, ParticleFilter::NUM_ROWS, Eigen::Dynamic> resMatrix;
    resMatrix.resize(ParticleFilter::NUM_ROWS, n);

    //generate list
    for(int i=0; i<n; i++) {
        //get random state
        Pos pPos = invisPosVec[random(invisPosVec.size()-1)];

        //set, and add random offset
        resMatrix(0,i) = pPos.rowDouble() + hsutils::randomDouble(0,1);
        resMatrix(1,i) = pPos.colDouble() + hsutils::randomDouble(0,1);

        //set heading
        resMatrix(2,i) = randomDouble(0, 2*M_PI);
    }

    return resMatrix;
}
#endif

Pos ParticleFilterPlayer::getNextPosRun(int actionDone, int *newAction) {

#ifdef OLD_CODE
    if (!playerInfo.hiderObsPosWNoise.isSet()) {
        DEBUG_FILTER(cout << "ParticleFilterPlayer::getNextPosRun: WARNING: the hider position is not set, only doing prediction."<<endl;);

        if (_params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_REQ_VISIB)
            throw CException(_HERE_,"ParticleFilterPlayer::getNextPosRun: observation required (since type is SOLVER_FILTER_PARTICLE_REQ_VISIB)");
    }
#endif


    //AG160215: use observation
    //create observation and use multiple
    HSObservation obs(&playerInfo, _playerInfoVec);


    DEBUG_CLIENT(
    cout<<"DYNOBST ParticleFilterPlayer::getNextPosRun: (#dynobst of obs: "<<obs.dynObstVec.size()<<")"<<endl;
    for(PlayerInfo* pi : _playerInfoVec ) {
        if (pi->isSeeker()) {
            cout <<"  DYNOBST ["<<pi->id<<"]: #"<<pi->dynObsVisibleVec.size()<<endl;
        }
    });

    /*cout << "pI DynOB: "<<playerInfo.dynObsVisibleVec.size()<<" obs dyn obs: "<<obs.dynObstVec.size()<<endl;
    assert(obs.otherSeekersObsVec.size()>0);
    int ndo=0;
    for(const PlayerInfo* pi : _playerInfoVec){
       if (pi->isSeeker()) {
           ndo+=pi->dynObsVisibleVec.size();
       }
    }
    cout<<"TOTAL dyn obst: "<<ndo<<endl;*/
    //assert(obs.dynObstVec.size()==ndo);*/

    assert(!_params->multiSeekerNoCommunication || obs.otherSeekersObsVec.empty());

    DEBUG_FILTER_WRITE_PARTICLES(
        ofstream writer;
        writer.open("particles_bef.csv");
        writer << _pf.getParticles()->transpose(); //TODO test
        writer.close();
    );

    //predict
    DEBUG_FILTER(cout<< "PF predict: "<<flush;);
    _pf.predict();
    DEBUG_FILTER(cout<< "ok"<<endl;);

    DEBUG_FILTER_WRITE_PARTICLES(
        writer.open("particles_pred.csv");
        writer << _pf.getParticles()->transpose(); //TODO test
        writer.close();
    );

    if (playerInfo.hiderObsPosWNoise.isSet() ||
            _params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES ||
            _params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES_HB ||
            _params->solverType==SeekerHSParams::SOLVER_MULTI_FILTER_ALWAYS_UPDATES_HB_PARTICLE
        ) {
        //if observation, or if always updating, do update

        DEBUG_FILTER(cout<< "PF update: "<<flush;);
        double degeneracyRet, weightMeanRet, weightStdRet;

        //update
        //_pf.update(playerInfo.hiderObsPosWNoise.toEigenVector<2>(), playerInfo.currentPos,
        //           &degeneracyRet, &weightMeanRet, &weightStdRet);
        //AG160215: use multiple
        _pf.updateMulti(obs, // playerInfo.currentPos,
                           &degeneracyRet, &weightMeanRet, &weightStdRet);
        DEBUG_FILTER(cout<< "ok"<<endl<<"  Degeneracy:"<<degeneracyRet<<"; w_mean:"<<weightMeanRet<<"+/-"<<weightStdRet<<endl;);

        DEBUG_FILTER_WRITE_PARTICLES(
            writer.open("particles_upd.csv");
            writer << _pf.getParticles()->transpose(); //TODO test
            writer.close();
        );
    }

    //return pos is center of Particle filter's state
    auto meanPF = _pf.getMeanParticle();

    DEBUG_FILTER(cout << "Mean Particle: "<<endl<<meanPF<<endl;);
    DEBUG_FILTER(cout << "Particle Cov.: "<<endl<<_pf.getCovParticle()<<endl;);

    //set return pos
    Pos returnPos(meanPF);

    //check if return pos is a legal position
    if (!_map->isPosInMap(returnPos) || _map->isObstacle(returnPos)) {
        returnPos = getClosestSeekerObs(returnPos);
    }
    DEBUG_FILTER(cout << "return pos: "<<endl<<returnPos.toString()<<endl;);


    //TODO!! THIS PART IS REPEATED; ALSO IN FOLLOWER

    //if a position is set
    double d = returnPos.distanceEuc(playerInfo.currentPos); //seekerPos);

    if ( (_params->onlySendStepGoals && d > 1) ||
         (!_params->onlySendStepGoals && d < 2*_params->winDist && playerInfo.hiderObsPosWNoise.isSet()) )  {

        //stop
        bool stopBeforePos = (playerInfo.hiderObsPosWNoise.isSet() && !_params->onlySendStepGoals);
        returnPos = getNextPosAsStep(playerInfo.currentPos, returnPos, 1, stopBeforePos);
    }

    //belief matrix should be updated
    _beliefProbMatrixUptodate = false;

    return returnPos;
}

bool ParticleFilterPlayer::isSeeker() const {
    return true;
}

std::string ParticleFilterPlayer::getName() const {
    return "ParticleFilter";
}

bool ParticleFilterPlayer::useGetAction() {
    return false;
}

bool ParticleFilterPlayer::tracksBelief() const {
    // if PF checker is set, then the PF is not yet initialized
    return _pfChecker!=nullptr;
}

bool ParticleFilterPlayer::handles2Obs() const {
    return true;
}

double ParticleFilterPlayer::getBelief(int r, int c) {
    assert(_pfChecker!=nullptr);

    if (!_beliefProbMatrixUptodate)
        updateMapHist(_map->rowCount(), _map->colCount());

    return _beliefProbMatrix[r][c];
}

//AG150119: token from Belief::updateMapHist
void ParticleFilterPlayer::updateMapHist(int rowCount, int colCount) {
    assert(rowCount>0 && colCount>0);

    //init matrix
    if (_beliefProbMatrix==nullptr) {
        _beliefProbMatrix = AllocateDynamicArray<double>(rowCount, colCount);
        assert(_beliefProbMatrix!=nullptr);
    }

    //first fill all with 0
    FOR(r,rowCount) {
        FOR(c,colCount) {
            _beliefProbMatrix[r][c] = 0;
        }
    }

    //get particles
    auto X = _pf.getParticles();
    assert(X!=nullptr && X->cols()>0);

    //now fill matrix, summing the values in the weights for each row,col
    for (int i = 0; i<X->cols(); i++) {
        int r = floor( (*X)(0,i) );
        int c = floor( (*X)(1,i) );
        _beliefProbMatrix[r][c] += 1; //it.second.second;
    }

    //normalize the values to get the probabilities
    double total = X->cols();
    FOR(r,rowCount) {
        FOR(c,colCount) {
            _beliefProbMatrix[r][c] /= total;
        }
    }

    //to know if the values of the matrix are consistent
    _beliefProbMatrixUptodate = true;
}
