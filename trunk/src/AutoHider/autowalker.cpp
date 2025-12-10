#include "AutoHider/autowalker.h"

#include <cassert>
#include <iostream>

#include "Utils/generic.h"

#include "exceptions.h"

using namespace std;

AutoWalker::AutoWalker(SeekerHSParams* params, size_t n) : AutoHider(params), _autoWalkerVec(n) {
}

AutoWalker::~AutoWalker() {
}

bool AutoWalker::initBeliefRun() {
    //AG160129: ist start pos set, use it

    //TODO: not sure if we should use it for all
    //use given start pos
    for(IDPos& pos : _autoWalkerVec) {
        pos = _startPos;
    }

    return true;
}


bool AutoWalker::checkMovement(size_t persI, Pos& newPos, Pos& seekerPos, Pos& hiderPos) {
    assert(newPos.isSet());

#ifdef DYN_OBST_COLL
    //max dist to persons
    double minDist = /*getParams()*/ _params->minDistToObstacle;

    bool moveOk = true;

    //check if too close to hider/seeker
    if (seekerPos.isSet() && newPos.distanceEuc(seekerPos)<minDist || hiderPos.isSet() && newPos.distanceEuc(hiderPos)<minDist) {
        DEBUG_AUTOHIDER(cout << "AutoWalker::checkMovement: filtered person "<<persI<<" to "<<newPos.toString()<<" due to seeker/hider pos"<<endl;);
        moveOk = false;
    } else {
        //check if too close to other walkers
        for(auto i=0; i<_autoWalkerVec.size(); i++) {
            if (i==persI) continue; //and ignore, since this is the prev pos of the walker we are checking

            if (newPos.distanceEuc(_autoWalkerVec[i])<minDist) {
                DEBUG_AUTOHIDER(cout << "AutoWalker::checkMovement: filtered person "<<persI<<" to "<<newPos.toString()<<" due to person "
                                <<i<<" pos: "<<_autoWalkerVec[i].toString()<<endl;);
                moveOk = false;
                break;
            }
        }
    }

    return moveOk;
#else
    return true;
#endif
}

bool AutoWalker::checkSumObsProbIs1() {
    return true; //AG150816: overwrite since we do not need to check this here
}

void AutoWalker::setStartPosRelativeTo(const std::vector<IDPos>& relativePoses, bool startVisib, double dist) {
    assert(!relativePoses.empty());
    assert(_map != nullptr);
    //assert(startVisib || _map->numObstacles()>0); // otherwise cannot hide
    if (!startVisib && _map->numObstacles()==0)
        throw CException(_HERE_,"Cannot hid if there are no obstacles.");

    DEBUG_CLIENT(
        cout << "RandomHider::setStartPosRelativeTo: startVisib="<<startVisib<<"; dist="<<dist<<"; Relative poses: "<<endl;
        for(const IDPos& pos : relativePoses)
            cout << " "<<pos.toString();
        cout << endl;
    );

    if (_startPos.isSet()) {
        DEBUG_WARN(cout << "AutoWalker::setStartPosRelativeTo - WARNING: startPos was already set: "<<_startPos.toString()<<endl;);
        _startPos.clear();
    }

    if (dist < 0) {
        //find random place that is either visible / not
        //DEBUG_CLIENT(cout<<"RandomHider::setStartPosRelativeTo: "<< <<endl;);

        //TODO: we could use GMap.getInvisblePoses
        unsigned int tries = 0;
        bool ok = false;
        while (tries < MAX_TRIES_FIND_START_POS && !ok) {
            //generate random pos
            _startPos = _map->genRandomPos();
            //check if it is visible and if we want it to be visibile
            bool isVisib;
            //if it should start visib, any of them being visible is enough,
            //if it should start hidden, then all should be hidden
            ok = !startVisib;
            //check if the positions are ok
            for(const IDPos& pos : relativePoses) {
                //check visibility
                isVisib = _map->isVisible(_startPos, pos, false, false); //always in hidden area

                if (startVisib) {
                    if (isVisib) {
                        //any being visible is ok
                        ok = true;
                        break;
                    }
                } else {
                    if (isVisib) {
                        //any being visible is NOT ok
                        ok = false;
                        break;
                    }
                }
            }

            ++tries;
        }

        DEBUG_CLIENT(cout<<"AutoWalker::setStartPosRelativeTo: tried "<<tries<<" times, ok="<<ok <<endl;);

        if (!ok) {
            cout << "RandomFixedHider::setStartPosRelativeTo - ERROR: failed to find a "<<(startVisib?"visible":"not visible")
                 <<" position" <<endl;
            throw CException(_HERE_, "could not find a correct hidden/not starting location");
        }

    } else {
        // based on distance

        assert(startVisib); //should be visible if distance is required
        //choose a position randomly
        _startPos = relativePoses[hsutils::random(relativePoses.size()-1)];

        DEBUG_CLIENT(cout<<"AutoWalker::setStartPosRelativeTo: chosen random pos "<<_startPos.toString()<<endl;);

        if (dist>0) {
            //move distance
            double distLeft = hsutils::random( dist>2.0 ? 1.0 : 0.0, dist);
            //try in 1 step first
            double distStep = distLeft;

            //TODO global const?
            static const double STEP_REDUCTION = 0.5;

            DEBUG_CLIENT(uint step = 0;);

            //do some random steps
            do {
                //random dir
                double dir = hsutils::randomDouble(0, 2*M_PI);
                //try to move
                Pos pos = _map->tryMoveDir(dir, _startPos, distStep, true);
                //check if ok
                if (pos.isSet()) {
                    //succeeded
                    distLeft -= distStep;
                    _startPos = pos;

                    DEBUG_CLIENT(cout<<" "<<step++<<" -> "<<pos.toString()<<"; dist left: "<<distLeft<<", dist step: "<<distStep<<endl;);
                } else {
                    if (distStep > 1.1*STEP_REDUCTION)
                        distStep -= STEP_REDUCTION;
                }

            } while (distLeft>0);

            DEBUG_CLIENT(cout<<" randomly chosen, distLeft="<<distLeft<<", distStep="<<distStep <<endl;);
        }
    }

    DEBUG_CLIENT(cout << " Chosen: "<<_startPos.toString()<<endl;);
}
