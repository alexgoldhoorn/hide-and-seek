#include "Filter/hsparticlefilterchecker.h"
#include <cmath>
#include <cassert>
#include <iostream>

#include "exceptions.h"
#include "Base/hsconfig.h"

using namespace std;

HSParticleFilterChecker::HSParticleFilterChecker(SeekerHSParams* params, GMap* map, PlayerInfo* playerInfo) :
    _map(map), _params(params), _playerInfo(playerInfo)
{
    assert(map!=nullptr);
    assert(params!=nullptr);
    assert(playerInfo!=nullptr);
}

HSParticleFilterChecker::~HSParticleFilterChecker(){
}

double HSParticleFilterChecker::calcWeight(const Eigen::Vector3d& particle, const Pos& obsPos, const Pos& seekerPos, const pomcp::Observation& obs) {
    static constexpr double sigmab2 = 1.0;// 0.9*0.9; //2*10*10; //*0.9*0.9;//TODO this has to be configurable

    //DEBUG_FILTER(cout << "WeightCalc: particle="<<particle<<", obs="<<obs<<flush;);

    //convert to Pos
    Pos partPos(particle);
    //Pos obsPos(obs);
    //Eigen::Vector2d obs = obsPos.toEigenVector<2>();

    // check if they are legal positions before calculating distance
    // when running this function, the observation should be a legal point
    /*if (!obsPos.isSet()) { // !_map->isPosInMap(obsPos) || _map->isObstacle(obsPos))
        //cout << "obs="<<endl<<obs<<endl<<"obspos="<<obsPos.toString()<<endl;
        throw CException(_HERE_,"observation should be a correct position in the map");
    }*/
    assert(_params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES ||
           _params->solverType==SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES_HB ||
           _params->solverType==SeekerHSParams::SOLVER_MULTI_FILTER_ALWAYS_UPDATES_HB_PARTICLE ||
           obsPos.isSet()
    );
    assert(seekerPos.isSet());

    //initialize weight to 0
    double w = 0;

    // to give the particle a weight, it should be a legal map position
    if (partPos.isSet() && _map->isPosInMap(partPos) && !_map->isObstacle(partPos)) {

        if (obsPos.isSet()) {
            //we have an observation, so we can calculate the weight
            double d = _map->distance(obsPos, partPos);
            double err2 = d*d; //pow(d,2);

            w = exp(-err2 / sigmab2);
        } else {
            //no visible observation, calculate probability of being visible
            double pVisib = _map->getVisibilityProb(seekerPos, partPos, false,
                                                      //_params->useDynObstForVisibCheck?&_playerInfo->dynObsVisibleVec:nullptr);
                                                    //AG160602: use the joined list of dyn. obst.
                                                      _params->useDynObstForVisibCheck? &obs.dynObstVec:nullptr);

            if (pVisib==0) {
                //not visible, but so consistent
                //AG160224 todo: we COULD add some false pos here.. by using some random factor
                w = WEIGHT_CONSISTENT_HIDDEN;
            } else {
                //is posibly visible:
                w = (1.0-pVisib) * WEIGHT_LOW_INCONSISTENT;
                        //TODO: fixed, higher ... or based on previous?;
            }


#ifdef OLD_CODE
            if (_map->isVisible(seekerPos, partPos, false)) {
                w = WEIGHT_LOW_INCONSISTENT;
            } else {
                w = WEIGHT_CONSISTENT_HIDDEN;
                        //TODO: fixed, higher ... or based on previous?;
            }
#endif

        }

        //DEBUG_FILTER(cout <<",d="<<d<<",err2="<<err2<<", w="<<w<<endl;);
    } //else
        // particle weight = 0, but maybe to be changed
        // TODO: change this? at least take into account possibility of all particles
        // having weight 0

    return w;
}


bool HSParticleFilterChecker::isValid(const Eigen::Vector3d &particle) {
    Pos pos(particle);
    return _map->isPosInMap(pos) && !_map->isObstacle(pos);
}
