#include <cstdio>
#include <cassert>
#include "MActNode.h"
#include "MObsEdge.h"
#include "Model.h"
#include "MBelief.h"
#include "MBounds.h"
#include "RandSource.h"

using namespace std;

ActNode::ActNode(const MAction& action, const Belief& belief, Bounds* bounds):
        action(action), belief(belief), bounds(bounds),
        avgLower(NegInf), avgUpper(NegInf),
        randSeed(bounds->randSource.get())
{}

void ActNode::backup()
{
    bool debug = false;

    if (debug) {
        cout<<"ActNode::backup\n";
    }

    for (map<MObs,ObsEdge>::iterator currIt = obsChildren.begin();
         currIt != obsChildren.end(); ++currIt){
        // backup for each observation
        ObsEdge& obsEdge = currIt->second;
        assert(obsEdge.count == obsEdge.cachedParticles->particles.size());
        obsEdge.backup();
    }

    // compute sum over the observations
    double lower = 0;
    double upper = 0;
    long count = 0;

    for (map<MObs,ObsEdge>::iterator currIt = obsChildren.begin(); currIt != obsChildren.end(); ++currIt) {
        lower += currIt->second.lower;
        upper += currIt->second.upper;
        count += currIt->second.count;
    }

    avgLower = lower / count;
    avgUpper = upper / count;

    if (debug) {
        cout<<"Leaving ActNode::backup\n";
    }
}

void ActNode::clearObsPartitions()
{
    for (map<MObs,ObsEdge>::iterator currIt = obsChildren.begin(); currIt != obsChildren.end(); ++currIt){
        currIt->second.clearParticles();
    }
}

void ActNode::generateObsPartitions()
{
    bool debug = false;

    if (debug) {
        cout<<"ActNode::generateObsPartitions\n";
    }

    // reinit observation related storage

    // Use the randSeed
    RandStream randStream;
    randStream.initseed(randSeed);

    Belief::const_iterator it;
    // Don't use parallel here since we need randStream to be used in
    // a sequential manner??? TODO: Is it true?
    MObs obs(vector<long>(bounds->model.getNumObsVar(),0));
    MState nextState(bounds->model.getNumStateVar(),0);
    for (it = belief.begin(bounds->numRandStreams,randStream);
         it != belief.end(); ++it){
        Particle const& currParticle = *it;
        MState const& currState = currParticle.state;

        double immediateReward = bounds->model.sample(currState, this->action, nextState, obs, randStream);

        map<MObs,ObsEdge>::iterator obsIt = obsChildren.find(obs);

        if (obsIt == obsChildren.end()) {
            // init observations if not seen before
            pair<map<MObs,ObsEdge>::iterator, bool> ret;
            ret = obsChildren.insert(pair<MObs,ObsEdge>(obs,ObsEdge(obs,bounds)));
            obsIt = ret.first;
        }

        double discounted = power(bounds->model.getDiscount(), currParticle.pathLength) * immediateReward;
        obsIt->second.addParticle(nextState, currParticle.pathLength+1, discounted);
    }

    if (debug) {
        cout<<"Leaving ActNode::generateObsPartitions\n";
    }
}
