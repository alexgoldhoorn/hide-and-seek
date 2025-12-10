#include "POMCP/hsobservation.h"

#include <sstream>
#include <cassert>
#include <cmath>
#include <iostream>

#include "exceptions.h"

using namespace pomcp;
using namespace std;

HSObservation::HSObservation() : Observation() {
}

HSObservation::HSObservation(const PlayerInfo *thisSeeker, const std::vector<PlayerInfo*>& allPlayersVec) : Observation() {
    bool ok = readFromPlayerInfo(thisSeeker, allPlayersVec);
    if(!ok)
        throw CException(_HERE_, "Problems while loading the HSObservation");
}

HSObservation::~HSObservation(){
}

State* HSObservation::copy() const {
    HSObservation* hsObs = new HSObservation();
    hsObs->ownSeekerObs = ownSeekerObs;
    hsObs->otherSeekersObsVec = otherSeekersObsVec;
    return hsObs;
}


string HSObservation::toString() const {
    stringstream sstr;
    //print observation
    sstr <<"<O:my:"<<ownSeekerObs.toString()<<";others[#"<<otherSeekersObsVec.size()<<"]:";
    //at max 3 observations
    size_t s = (size_t)min(3,(int)otherSeekersObsVec.size());
    for(size_t i=0; i<s; i++) {
        sstr<<otherSeekersObsVec[i].toString();
        if (i!=s-1) sstr<<";";
    }

    if (s<otherSeekersObsVec.size())
        sstr<<"...";
    sstr<<">";

    return sstr.str();
}


bool HSObservation::operator <(const State& other) const { //TODO improve/remove
    if (other.getStateType()==State::HSObservation_Type) {
        const HSObservation& hsobs = static_cast<const HSObservation&>(other);

        return ownSeekerObs<hsobs.ownSeekerObs;
    } else {
        throw CException(_HERE_,"HSObservation::operator <: requires a HSObservation as parameter");
    }

}

bool HSObservation::operator >(const State& other) const {//TODO improve/remove
    if (other.getStateType()==State::HSObservation_Type) {
        const HSObservation& hsobs = static_cast<const HSObservation&>(other);

        return ownSeekerObs>hsobs.ownSeekerObs;
    } else {
        throw CException(_HERE_,"HSObservation::operator >: requires a HSObservation as parameter");
    }
}

bool HSObservation::operator ==(const State& other) const {//TODO improve/remove
    if (other.getStateType()==State::HSObservation_Type) {
        const HSObservation& hsobs = static_cast<const HSObservation&>(other);

        return ownSeekerObs==hsobs.ownSeekerObs;
    } else {
        return false;
    }
}

bool HSObservation::operator !=(const State& other) const {//TODO improve/remove
    if (other.getStateType()==State::HSObservation_Type) {
        const HSObservation& hsobs = static_cast<const HSObservation&>(other);

        return ownSeekerObs!=hsobs.ownSeekerObs;
    } else {
        return true;
    }
}


State::StateType HSObservation::getStateType() const {
    return State::HSObservation_Type;
}

string HSObservation::getHash() const {
    //TODO SHOULD return a hash number that represents exactly one State!!!
    throw CException(_HERE_, "TODO IMPLEMENT - not sure if necessary");

    return ownSeekerObs.getHash();  //TODO should only be based on the seeker (?)
}

bool HSObservation::readFromPlayerInfo(const PlayerInfo* thisSeekerPlayer,
                                       const std::vector<PlayerInfo*>& allPlayersVec) {

    assert(allPlayersVec.size()>=2);
    //assert there are #players-2 other seekers
    //return in the vector observation of other seekers,
    //which are #players - 1 hider - this seeker
    bool ok = true;
    //AG150709: we want to normalize this again, since some seekers might not have been seen
    double sumObsProb = 0;
    //count of seekers with data read
    uint numSeekers = 0;

    //now loop all players to get their obs
    for(const PlayerInfo* pInfoIt : allPlayersVec) {
        if (pInfoIt->isSeeker() && *pInfoIt!=*thisSeekerPlayer) {
            //add obs only of other seekers

            HSState obs;
            bool readOk = obs.readFromPlayer(*pInfoIt);

            if (readOk) {
                //another seeker, so stet a the player's obs
                otherSeekersObsVec.push_back(obs);
                //add obs prob to sum
                sumObsProb += obs.prob;
                ++numSeekers;
                //AG160215: seeker should be set
                assert(obs.seekerPos.isSet());
            } else {
                //AG150709: we should allow this, since it could happen that another seeker's pos has not been received
                //ok = false;
                DEBUG_WARN(cout <<"HSObservation::readFromPlayerInfo: WARNING could not read "<<pInfoIt->toString()<<endl;);
            }
        }
    }

    //assert(otherSeekersObsVec.size()<=allPlayersVec.size()-1); //-2); AG150701: we allow the hiderPlayer not to be passed

    //now return own obs
    bool readOk = ownSeekerObs.readFromPlayer(*thisSeekerPlayer);
    if (!readOk) {
        ok = false; //we require the own seeker to be read at least
        cout << "HSObservation::readFromPlayerInfo: WARNING could not read own player info: "<<thisSeekerPlayer->toString()<<endl;
    } else {
        //add obs prob to sum
        sumObsProb += ownSeekerObs.prob;
        ++numSeekers;

        if (sumObsProb == 0) {
            cout << "HSObservation::readFromPlayerInfo: WARNING: the sum of obs. prob is 0, might be due to connection failure, setting uniform prob."<<endl;
            double p = 1.0/numSeekers;
            ownSeekerObs.prob = p;
            for(HSState& state : otherSeekersObsVec) {
                state.prob = p;
            }
        } else {
            //now normalize all
            ownSeekerObs.prob /= sumObsProb;
            for(HSState& state : otherSeekersObsVec) {
                state.prob /= sumObsProb;
            }
        }
    }

    //AG160504: add dyn. obst. from all
    dynObstVec.clear();
    //create hash vector to check for duplicates
    vector<string> dynObsHashVec;
    //create vector

    for(IDPos pos : thisSeekerPlayer->dynObsVisibleVec) {
       dynObstVec.push_back(pos);
       dynObsHashVec.push_back(pos.getHash(false));
    }
    //now check for other players
    for(const PlayerInfo* pInfoIt : allPlayersVec) {
        if (pInfoIt->isSeeker() && *pInfoIt!=*thisSeekerPlayer) {
            //loop over its dyn. obst.
            for(IDPos pos : pInfoIt->dynObsVisibleVec) {
                //check if the dyn. obst. is already listed
                if ( find(dynObsHashVec.begin(), dynObsHashVec.end(), pos.getHash(false)) == dynObsHashVec.end()){
                    //not listed, so add
                    dynObstVec.push_back(pos);
                    dynObsHashVec.push_back(pos.getHash(false));
                }
            }
        }
    }

    return ok;
}

HSState* HSObservation::getRandomState(double i) const {
    assert(i>=0 && i<=1);

    State* newState = NULL;
    if (i<=ownSeekerObs.prob) {
        //choosing the own obs
        newState = ownSeekerObs.copy();
    } else {
        assert(!otherSeekersObsVec.empty());
        //choose another prob
        double pSum = ownSeekerObs.prob;

        size_t j = 0;
        for(;j<otherSeekersObsVec.size() && i>pSum; j++) {
            pSum += otherSeekersObsVec[j].prob;
        }

        //we should have a legal index
        assert(j-1<=otherSeekersObsVec.size());

        //return the state
        newState = otherSeekersObsVec[j-1].copy();
    }

    return HSState::castFromState(newState);
}

const HSState* HSObservation::getObservationState(size_t i) const {
    //TODO: should we return a copy??

    if (i==0) {
        //return own seeker obs
        return &ownSeekerObs;
    } else {
        if (i-1>=otherSeekersObsVec.size()) {
            throw CException(_HERE_,"outofbounds: requesting index "+to_string(i)
                             +" but only has "+to_string(size())+" items");
        }

        return &otherSeekersObsVec[i-1];
    }
}

size_t HSObservation::size() const {
    return otherSeekersObsVec.size()+1;
}


const HSObservation* HSObservation::castFromObservation(const Observation *obs) {
    if (obs==NULL) {
        return NULL;
    } else {
        return static_cast<const HSObservation*>(obs);
    }
}

HSObservation* HSObservation::castFromObservation(Observation *obs) {
    if (obs==NULL) {
        return NULL;
    } else {
        return static_cast<HSObservation*>(obs);
    }
}

const State* HSObservation::getUpdateObservationState() const {
    return &ownSeekerObs;
}

void HSObservation::convertToObservation() {
    throw CException(_HERE_,"HSObservation does not support 'convertToObservation");
}

vector<Pos> HSObservation::getSeekerPoses() const {
    vector<Pos> seekerPosesVec;
    seekerPosesVec.push_back(ownSeekerObs.seekerPos);
    //add other seeker poses
    for(const HSState& hsState : otherSeekersObsVec) {
        seekerPosesVec.push_back(hsState.seekerPos);
    }

    return seekerPosesVec;
}

bool HSObservation::hasVisibleHider() const {
    for(const HSState& hsState : otherSeekersObsVec) {
        if (hsState.hiderVisible())
            return true;
    }

    return false;
}
