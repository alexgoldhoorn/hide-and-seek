
#ifndef HS_PARTICLE_FILTER_CHECKER_H
#define HS_PARTICLE_FILTER_CHECKER_H

#include "Filter/particlefilterchecker.h"
#include "Base/seekerhsparams.h"
#include "Base/playerinfo.h"
#include "HSGame/gmap.h"

/**
 * @brief HSParticleWeighter calculates the weights of particles based on the
 * known map.
 */
class HSParticleFilterChecker : public ParticleFilterChecker {
    public:
        //TODO make global tunable params
       static constexpr double WEIGHT_LOW_INCONSISTENT = 0.001;
       static constexpr double WEIGHT_CONSISTENT_HIDDEN = 0.01;

       //AG160504: added player info to get observed autowalkers
       HSParticleFilterChecker(SeekerHSParams* params, GMap* map, PlayerInfo* playerInfo);
       virtual ~HSParticleFilterChecker();

       virtual double calcWeight(const Eigen::Vector3d& particle, const Pos& obsPos, const Pos& seekerPos, const pomcp::Observation& obs);

       virtual bool isValid(const Eigen::Vector3d& particle);


    private:
       GMap* _map;
       SeekerHSParams* _params;
       PlayerInfo* _playerInfo;
};
#endif
