#include "HideSeekModel.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>

#include "MInclude.h"
#include "ParticlesBelief.h"
//#include "hsexception.h"

#include "hsglobaldata.h"

#include <string>
#include <vector>
#include <sstream>

using namespace std;



string stateToString(MState& s) {
    stringstream sstr;
    sstr <<"State: (";
    for(vector<double>::iterator it = s.begin(); it!=s.end(); it++) {
        sstr << *it <<",";
    }
    sstr<<")";
    return sstr.str();
}


/*
 * TODO:
 * - observations not 1/0, can be noisy in real world
 * -
 *
 **/


//> code of MCVI DiscreteCorridor example
double uniform_deviate(int seed) {
    return seed * (1.0 / (RAND_MAX + 1.0) );
}

int rand_range(int low, int high) {
    return low + uniform_deviate(rand()) * (high-low);
}

#define NOISY(v) ((v) ^ (randStream.getf() < Noise))
//< Discrete corridor



HideSeekModel::HideSeekModel(GMap* map):
        Model(NUM_STATE_VARS, NUM_OBS_VARS, NUM_ACTS, NUM_MACRO_ACTS, NUM_INIT_POLICIES, Discount),
        _seekerPlayer(map), _hiderPlayer(map)
{
    _gmap = map;
    _base = map->getBase();
}

bool HideSeekModel::allowableAct(const Belief& belief, const MAction& action) {
    if (action.type == Macro) return false;

    return true;
}

//AG NOTE: could also use indices instead of (x,y) as states!!!
double HideSeekModel::sample(const MState& currState, const MAction& action, MState& nextState, MObs& obs, RandStream& randStream)  {
    bool debug = false;

    //first do deterministic movement of seeker
    _seekerPlayer.setCurPos(currState[0],currState[1]);
    bool moved = _seekerPlayer.move(action.getActNumUser()); //TODO: check if this indeed is the index!

    /*if (!moved) { //ag130304: seeker could not move
        //throw HSException("seeker could not move");
        cout << "WARNING: seeker not moved"<<endl;
    }*/
    //cout << (moved?'.':'!')<<flush;

    //next state
    Pos nextSPos = _seekerPlayer.getCurPos();

    //now decide random movement of hider
    _hiderPlayer.setCurPos(currState[2],currState[3]);
    // ag130227: getf()<1.0 -> see RandStream.getf
    // ag130305: try actions until one that works (others might be out of bounds)
    moved = false;
    while (!moved) {
        int hiderAct = (int)floor( randStream.getf() * NUM_ACTS );
        moved = _hiderPlayer.move(hiderAct);
    }

    /*if (!moved) { //ag130304: hider could not move
        //throw HSException("hider could not move");
        cout << "WARNING: hider not moved"<<endl;
    }*/

    //next hider state
    Pos nextHPos = _hiderPlayer.getCurPos();

    //set next state
    nextState[0] = nextSPos.row;
    nextState[1] = nextSPos.col;
    nextState[2] = nextHPos.row;
    nextState[3] = nextHPos.col;

    //is visible
    bool isVis = _gmap->isVisible(nextSPos,nextHPos);

    //set observation
    if (isVis) {
        //visible -> sees correct coordinates
        obs.obs[0] = nextHPos.row;
        obs.obs[1] = nextHPos.col;
    } else {
        obs.obs[0] = obs.obs[1] = -1;
    }


    //now calculate reward
    double reward = REWARD_MOVEMENT;
    if (nextHPos.equals(nextSPos)) {
        //seeker wins
        reward = REWARD_WIN;
    } else if (nextHPos.equals(_gmap->getBase())) {
        reward = REWARD_LOSE;
    } //else tie ..


       // if NOISY(moveDir) nxtIndex = 1;

    if (debug) {
        cout<<"Model::sample\n";
        cout<<currState[1]<<" "<<action.getActNumUser()<<" "<<nextState[1]<<" "<<obs.obs[0]<<"\n";
        cout<<"Leaving Model::sample\n";
    }

    return reward;
}

double HideSeekModel::sample(const MState& currState, const MAction& macroAct, long controllerState, MState& nextState, long& nextControllerState, MObs& obs, RandStream& randStream) {
    //Macro actions not supported by solver (yet)
    assert(false);
    return 0;
}

double HideSeekModel::initPolicy(const MState& currState, const MAction& initAction, long controllerState, MState& nextState, long& nextControllerState, MObs& dummy, RandStream& randStream) {
    MObs obs(vector<long>(getNumObsVar(), 0));
    /*if (currState[0] > 0) {
        nextState = currState;
        return 0;
    }

    // move left
    return sample(currState, Action(Act,ActLeft), nextState, obs, randStream);*/

    //first halt, later on use mabye like triangle rule of automated hider
    //TODO: more intelligent action
    return sample(currState, MAction(Act,HSGlobalData::ACT_H), nextState, obs, randStream);
}


//AG130304: disabled since used by initbelief, different than our
MState HideSeekModel::sampleInitState() {
    //double p = rand_range(0,4);
//    cout << p << endl;
    int rows = _gmap->rowCount();
    int cols = _gmap->colCount();
    int numCells = _gmap->numFreeCells();

    MState st(getNumStateVar(), 0);
    //st[1] = p;       

    //ag130311: instead of random x,y, get random free cell
    int sCell = rand_range(0,numCells);
    int hCell = rand_range(0,numCells);

    Pos sPos = _gmap->getCoordFromIndex(sCell);
    Pos hPos = _gmap->getCoordFromIndex(hCell);

    st[0] = sPos.row; //rand_range(0,rows);
    st[1] = sPos.col; //rand_range(0,cols);
    st[2] = hPos.row; //rand_range(0,rows);
    st[3] = hPos.col; //rand_range(0,cols);

    assert(st[0]>=0 && st[0]<rows && st[1]>=0 && st[1]<cols);
    assert(st[2]>=0 && st[2]<rows && st[3]>=0 && st[3]<cols);

    return st;
}


// not used directly bug model, but for getting a init belief

/// TODO!!!!!
//// INIT BELIEF DEPENDS ON MAP AND HIDDEN POS
ParticlesBelief* HideSeekModel::getInitBelief(int num) {
    MObs obs(vector<long>(getNumObsVar(), 0));
    obs.obs[0] = obs.obs[1] = -1; //ObsNothing;
    ParticlesBelief* pb = new ParticlesBelief(new BeliefNode(obs));
    double w = 1.0/num;

    cout << "Creating init belief, w="<<w<<endl;

    for (long i = 0; i < num; i++) {
        MState st = sampleInitState();
        Particle temp(st,0,w);
        pb->belief.push_back(temp);

        cout << " "<<i<<") "<<stateToString(st)<<endl;
    }
    return pb;
}

double HideSeekModel::upperBound(const MState& state) {

    /*double minSteps = fabs(state[1] - 1);
    double reward = 0, coef = 1;
    for (int i=1; i<minSteps; i++) {
        reward += coef * REWARD_MOVEMENT;
        coef *= discount;
    }
    reward += coef * EnterReward;
    return reward;*/

    /* upper bound: best possible reward to be get:
     *  shortest route to hider's location [Note: assuming hider is not moving]
     */

    float d = _gmap->distance(state[0],state[1],state[2],state[3]);
    double reward = d * REWARD_MOVEMENT + REWARD_WIN;

    return reward;
}

double HideSeekModel::getObsProb(const MAction& action, const MState& nextState, const MObs& obs) {

    double isVis = _gmap->isVisible(nextState[0],nextState[1],nextState[2],nextState[3]);

    if (isVis) {
        //is visible so should return right observation
        //TODO: note that here the probability may be slightly different if we work in the real world

        if (nextState[2]==obs.obs[0] && nextState[3]==obs.obs[1]) {
            return 1.0;
        } else {
            return 0;
        }
    } else {
        //not visible, so obs should be -1
        if (obs.obs[0] == -1) {
            return 1.0;
        } else {
            return 0;
        }
    }

    return -1;

    /*    bool debug = false;

    double nxtPos = nextState[1];

    if (debug) {
        cout<<"ActNum "<<action.getActNumUser()<<"\n";
        cout<<"State "<<nextState[1]<<"\n";
        cout<<"Obs "<<obs.obs[0]<<"\n";
    }

    if (obs.obs[0] == TermObs) {
        if (nextState[0] == TermState) return 1.0;
        else return 0.0;
    } else if (obs.obs[0] == ObsLeftEnd) {
        if (nxtPos == 0) return 1.0;
        else return 0.0;
    } else if (obs.obs[0] == ObsRightEnd) {
        if (nxtPos == 3) return 1.0;
        else return 0.0;
    } else if (obs.obs[0] == ObsWrongDoor) {
        if (nxtPos != 1) return 1.0;
        else return 0.0;
    } else {
        //ObsNothing
        if (nxtPos == 0 || nxtPos == 3) return 0.0;
        return 1.0;
    }*/
}
