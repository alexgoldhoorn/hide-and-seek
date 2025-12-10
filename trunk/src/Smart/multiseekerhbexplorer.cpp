#include "Smart/multiseekerhbexplorer.h"

#include <iostream>
#include <limits>
#include <algorithm>
#include <cassert>

#include "exceptions.h"


using namespace std;

MultiSeekerHBExplorer::MultiSeekerHBExplorer(SeekerHSParams* params, AutoPlayer* autoPlayerWBelief) :
    TwoSeekerHBExplorer(params,autoPlayerWBelief)
{
    if (_params->minDistBetweenRobots>0 && _params->numPlayersReq>3) {
        throw CException(_HERE_, "for now the minimimum distance between the robots can only be assured for 2 robots at maximum, "
                         "either reduce the number of players (numPlayersReq) or set the min. distance (minDistBetweenRobots) to 0");
    }
}

MultiSeekerHBExplorer::~MultiSeekerHBExplorer()
{
}

bool MultiSeekerHBExplorer::initBeliefRun() {
    TwoSeekerHBExplorer::initBeliefRun();

    //there should be at least 1 seeker, 1 hider
    assert(_playerInfoVec.size()>=2);
    assert(_hiderPlayer!=NULL);

    //create player info wrapper
    for(PlayerInfo* pInfo : _playerInfoVec) {
        _piWrapperVec.push_back(PlayerInfoWrapper(pInfo, _params));
    }

    return true;
}

bool MultiSeekerHBExplorer::calcNextHBList(int actionDone) {

    //assert(seekerPos.isSet());
    DEBUG_CLIENT(
        cout<<"MultiSeekerHBExplorer::calcNextHBList: first updating belief"<<endl;//: [s1:"<<seekerPos.toString()<<",h:"<<hiderPos.toString()<<";s2:"<<flush;
    );

    //TODO!!! JOIN DYN OBST FROM ALL!!!!


    assert(playerInfo.initPosSet);
    assert(playerInfo.posRead);
    assert(!playerInfo.multiHasHBPoses);

    playerInfo.multiHBPosVec.clear();

    _autoPlayerWBelief->playerInfo.prepareNextStep(); //for testing

    //AG150608: copy values
    _autoPlayerWBelief->playerInfo.copyValuesFrom(playerInfo, true);

    assert(playerInfo.currentPos == _autoPlayerWBelief->playerInfo.currentPos);
    assert(playerInfo.hiderObsPosWNoise == _autoPlayerWBelief->playerInfo.hiderObsPosWNoise);

    //AG150608: simplified, run getNextPos of the child auto player to update its belief
    // the selection of observations should be done in that class using hiderObsPosWNoise and first
    // should be check if the value has been read using the posRead flag (otherwise there might have been a communication problem)
    Pos aPlayerPos = _autoPlayerWBelief->getNextPos(actionDone);

    //AG150710: assure that the last action is set
    playerInfo.lastAction = _autoPlayerWBelief->playerInfo.lastAction;
    //AG150728: copy the reward from the POMCP
    playerInfo.seekerReward = _autoPlayerWBelief->playerInfo.seekerReward;

    //now choose the hider player
    _chosenHiderPosConsist = chooseHiderObsFromMulti(_chosenHiderPos);

    assert(_params->multiSeekerExplorerCheckNPoints>0);
    //find highest belief
    playerInfo.multiHBPosVec = findHighestBelief(playerInfo.currentPos,_params->multiSeekerExplorerCheckNPoints,0);

    assert(playerInfo.multiHBPosVec.size()>0);
    //assert(playerInfo.multiHBPosVec.size()==playerInfo.multiHBBeliefVec);

    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::calcNextHBList: found "<<playerInfo.multiHBPosVec.size()<<" highest belief points"<<endl;);

    playerInfo.multiHasHBPoses = true;

    return true;
}


Pos MultiSeekerHBExplorer::selectRobotPosMulti() {
    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::selectRobotPosMulti: "<<endl;);

#ifdef DEBUG_ASSUMEPERFECTCONNECTION
    for(PlayerInfo* pi : _playerInfoVec) {      //TODO: remove this
        DEBUG_CLIENT(cout<<" * "<<pi->toString()<<flush;);
        assert(pi->id>=0);
        if (pi->isSeeker() && !_params->multiSeekerNoCommunication) {
            assert(pi->posRead);
            assert(pi->multiHasHBPoses);
            assert(!pi->multiHBPosVec.empty());
            DEBUG_CLIENT(cout<<"   ->"<<flush;);
            for(BPos& bpos : pi->multiHBPosVec) {
                DEBUG_CLIENT(cout<<bpos.toString()<<" "<<flush;);
                assert(bpos.isSet());
                assert(bpos.b>=0);
            }
        }
        DEBUG_CLIENT(cout<<endl;);
    }
#endif

    //here we should have all highest beliefs
    assert(playerInfo.multiHasHBPoses);
    assert(!playerInfo.chosenGoalPos.isSet());

    if ( _chosenHiderPos.isSet() || //AG15022: if hider pos known, always update goal
        (_params->highBeliefFollowerUpdateGoalNumSteps==0 && _params->highBeliefFollowerUpdateGoalTime_ms==0)  || //AG150205: if no update
        (_timerID<0 && _params->highBeliefFollowerUpdateGoalTime_ms>0) ||      //OR timer not yet started
            (_params->highBeliefFollowerUpdateGoalNumSteps>0 && _stepsToLastUpdate<=0)
            ||             //OR: a number of steps to update is set and it is passed
            (       _params->highBeliefFollowerUpdateGoalTime_ms>0
                    && _timer.getTime_ms(_timerID)>=_params->highBeliefFollowerUpdateGoalTime_ms
            )              //OR: there is update time set, and the time is passed
            || playerInfo.currentPos.distanceEuc(_lastGoalPos)<=_params->followPersonDist
                           //OR: the robot almost reached its goal
         )
        {
        //now decide the order in which we run exploration algo
        //NOTE 1: the order is really important, since the same goal position can be chosen
        //      for different seekers, e.g. p1 can be assigned to s1 if s1 starts, and p2 to s2,
        //      but it can be revers (p1 to s2, and p2 to s1) if we start with s2.
        //      To assure that each agent calculates the same locations for each of the agents we have
        //      to calculate in the same order.
        //NOTE 2: the order (by ID or sum of highest belief points) is defined by SeekerHSParams.multiSeekerProcessOrder
        //AG150717: recalc belief
        for (PlayerInfoWrapper& piw : _piWrapperVec) {
            piw.recalcIfReq();
        }
        //AG150910: reverse search
        std::sort(_piWrapperVec.begin(), _piWrapperVec.end(), std::greater<PlayerInfoWrapper>());

        /*cout << "SORTED PLAYER INFO:"<<endl;
        for(size_t i=0; i<_piWrapperVec.size(); i++)
            cout << "  "<<i<<") "<<_piWrapperVec[i].playerInfo->toString()<<" bs="<<_piWrapperVec[i]._beliefSum<<endl;
        cout<<endl;*/

        //2. now choose which method to use
        if (!_chosenHiderPosConsist || !_chosenHiderPos.isSet()) {
            DEBUG_CLIENT(cout<<"using explorer"<<endl;);
            useExplorer();
        } else {
            //hider visible and consistent observation
            DEBUG_CLIENT(cout<<"using follower"<<endl;);
            useFollower(_chosenHiderPos);
        }

        //(re)start timer
        if (_params->highBeliefFollowerUpdateGoalTime_ms>0) {
            if (_timerID<0) {
                _timerID = _timer.startTimer();
            } else {
                _timer.restartTimer(_timerID);
            }
        }
        //restart step timer
        _stepsToLastUpdate = _params->highBeliefFollowerUpdateGoalNumSteps;

        //goal should now be set
        assert(playerInfo.chosenGoalPos.isSet());

        //last goal
        _lastGoalPos = playerInfo.chosenGoalPos; //returnPos;

    } else if (_lastGoalPos.isSet()) {     // timer not passed / or hider not seen
        DEBUG_CLIENT(cout<<"no update"<<endl;);
        //return last position
        playerInfo.chosenGoalPos = _lastGoalPos;
    } else {
        //SHOULD not occur
        cout <<"MultiSeekerHBExplorer::selectRobotPosMulti: WARNING: time passed and no last pos found"<<endl;
        playerInfo.chosenGoalPos = playerInfo.currentPos;
    }

    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::selectRobotPosMulti: --> chosen goal pos: "<<playerInfo.chosenGoalPos.toString(););

    if (_params->onlySendStepGoals) {
        playerInfo.nextPos = getNextPosAsStep(playerInfo.currentPos, playerInfo.chosenGoalPos, 1, false);

        DEBUG_CLIENT(cout<<", in steps, next step: "<<playerInfo.nextPos.toString(););
    } else {
        //AG150610: no steps so the next pos is the pos we've chosen here
        playerInfo.nextPos = playerInfo.chosenGoalPos;
    }
    DEBUG_CLIENT(cout<<endl;);

    //update steps counter
    if (_stepsToLastUpdate>0)
        --_stepsToLastUpdate;

    return playerInfo.nextPos;

}

bool MultiSeekerHBExplorer::chooseHiderObsFromMulti(Pos &chosenHider) {    //TODO: use trust probability ?? -> weighted avg of Pos to get real pos
    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::chooseHiderObsFromMulti: ";);

    //AG150608: choos hider pos
    bool hiderPosConsist = true;
    chosenHider.clear(); // = playerInfo.hiderObsPosWNoise;

    assert(playerInfo.posRead);

    //now loop all players to check for consistency and/or an observation (if not yet already found)
    for(PlayerInfo* pInfo : _playerInfoVec) {
        if (pInfo->isSeeker() && pInfo->posRead && pInfo->hiderObsPosWNoise.isSet()) {
            if (chosenHider.isSet()) {
                //check for consistency
                if (chosenHider.distanceEuc(pInfo->hiderObsPosWNoise) > _params->obsEqualsThreshDist) {
                    //DEBUG_CLIENT(cout<<" not consistent";);
                    chosenHider.clear();
                    hiderPosConsist = false;
                    break;
                }
            } else {
                //set the new chosen hider
                chosenHider =  pInfo->hiderObsPosWNoise;
                //DEBUG_CLIENT(cout<<"Found "<<chosenHider.toString(););
            }
        }
    }

    DEBUG_CLIENT(cout<<" -> "<<chosenHider.toString()<<" and "<<(hiderPosConsist?"consistent":"NOT consistent")<<endl;);

    return hiderPosConsist;
}

void MultiSeekerHBExplorer::combineHBPoses(std::vector<BPos> &hbPosesVec) {
    //TODO: maybe sort at same time?
    assert(!playerInfo.multiHBPosVec.empty());

    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::combineHBPoses: Players:"<<endl;);
    /* //AG160217: disabled, since does only check, and also done in next loop
    for(PlayerInfo* pInfo : _playerInfoVec) {
        DEBUG_CLIENT(cout <<" - "<<pInfo->toString(););
        if (pInfo->isSeeker() && pInfo->multiHasHBPoses ) {

            for(size_t i=0;i<pInfo->multiHBPosVec.size(); i++) {
                BPos bpos = pInfo->multiHBPosVec[i];
                DEBUG_CLIENT(cout <<" "<< bpos.toString(););
                assert(bpos.isSet());
            }
        }
        DEBUG_CLIENT(cout<<endl;);
    }*/

    //clear
    hbPosesVec.clear();
    //DEBUG_CLIENT(cout<<endl<<"combine hb poses:"<<endl;);

    //sum for normalization
    double pSum=0;

    //now check all hb points from other seekers
    for(PlayerInfo* pInfo : _playerInfoVec) {
        DEBUG_CLIENT(cout << " - "<<pInfo->toString()<<" - "<<(pInfo->isSeeker()?"seeker":"hider")<<" - hashb:"<<pInfo->multiHasHBPoses<<flush;);
        if (pInfo->isSeeker() /*&&  *pInfo!=playerInfo*/ && pInfo->multiHasHBPoses) {
            DEBUG_CLIENT(cout<<" (#hb="<<pInfo->multiHBPosVec.size()<<"): ";); //<<endl;
            assert(!pInfo->multiHBPosVec.empty());
            //only if not own and if it has the multi hb poses (could be that it didn't receive)
            for(BPos bpos : pInfo->multiHBPosVec) {
                assert(bpos.isSet());
                //find if the pos already exists
                DEBUG_CLIENT(cout<<bpos.toString()<<"-";);
                vector<BPos>::iterator it = find(hbPosesVec.begin(), hbPosesVec.end(), bpos);
                DEBUG_CLIENT(cout<<bpos.toString()<<"~";);
                if (it==hbPosesVec.end()) {
                    //doesn't exist, add it
                    hbPosesVec.push_back(bpos);
                    DEBUG_CLIENT(cout << "new; ";);
                } else {
                    //exists, add belief
                    it->b += bpos.b;
                    DEBUG_CLIENT(cout << "added->"<<it->toString()<<"; ";);
                }
                //sum
                pSum += bpos.b;
            }

        }
        DEBUG_CLIENT(cout <<endl;);
    } //for


    /*for(BPos& bpos : hbPosesVec) {
        pSum += bpos.b;
    }*/

    //normalize
    for(BPos& bpos : hbPosesVec) {
        bpos.b /= pSum;
    }
}


void MultiSeekerHBExplorer::useExplorer() {
    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::useExplorer: ";);

    assert(playerInfo.multiHasHBPoses);
    assert(!playerInfo.multiHBPosVec.empty());

    //combine all highest belief poses
    DEBUG_CLIENT(cout<<"combine the HB poses");
    vector<BPos> hbPosesVec;
    combineHBPoses(hbPosesVec);

    DEBUG_CLIENT(cout<<" --> "<< hbPosesVec.size()<<endl;);
    assert(!hbPosesVec.empty());

    //AG150715: initialize U vector
    vector<double> uVec(hbPosesVec.size(), 1.0);
    //list of all poses
    vector<Pos> goalPosesVec;
    //DEBUG_CLIENT(cout<<"Goals:"<<endl;);
    //now use explorer to find goals, using the order of the piWrapperVec
    for(size_t i=0; i<_piWrapperVec.size(); ++i) {
        PlayerInfo* pInfo = _piWrapperVec[i].playerInfo;
        //DEBUG_CLIENT(cout<<" - "<<pInfo->toString(););

        if (pInfo->isSeeker() && pInfo->posRead) {
            assert(pInfo->currentPos.isSet());
            //we have all the information, so we can calculate its goal
            //TODO: can be more efficient if the max distance and max belief are calculated beforehand once
            int goalI = calcExplorationEvaluation(pInfo->currentPos, hbPosesVec, uVec, i+1<_piWrapperVec.size());
            //set goal
            goalPosesVec.push_back(hbPosesVec[goalI]);
            DEBUG_CLIENT(cout<<" "<<goalPosesVec.back().toString()<<endl;);

            //AG150917: set chosengoal here
            //AG160725: set chosen goal in next iteration of distance check
            /*if (*pInfo==playerInfo || _params->numPlayersInClient==1) {
                assert(!pInfo->chosenGoalPos.isSet());
                pInfo->chosenGoalPos = hbPosesVec[goalI];
            }*/
        }
        //DEBUG_CLIENT(cout<<endl;);
    }

    //note: although the previous poses are chosen based on the distance to a specific seeker,
    // it might happen that another seeker is closer to a certain goal, therefore we try to
    // assign the highest goal to each.
    //AG150917: there still can be situations with another agent having to go much further
    //  because the first agent was assigne its closest:
    //
    //     b    1 a  2
    //                  -> here goal a is coser to 1, so if 1 is choosing first: 1->a, and 2->b,
    //                      but it would have been more efficient to go 1->b, 2->a, |1-b|+|2-a|<|1-a|+|2-b|
    // -> herefore disabled
    //AG160725: re-enabled becauseit is a good approximation
//#ifdef USE_MIN_DIST_CHECK
    DEBUG_CLIENT(cout<<"final goal:"<<endl;);
    for(size_t i=0; i<_piWrapperVec.size(); i++) {
        PlayerInfo* pInfo = _piWrapperVec[i].playerInfo;
        DEBUG_CLIENT(cout<<" - "<<pInfo->toString(););

        if (pInfo->isSeeker() && pInfo->posRead /*&& pInfo->multiHasHBPoses*/) {
            //first find closest
            double minD = numeric_limits<double>::max();
            int minI = -1;
            for(size_t i=0; i<goalPosesVec.size(); i++) {
                //calc distance
                double d = _map->distance(pInfo->currentPos, goalPosesVec[i]);
                if (d<minD) {
                    minD = d;
                    minI = i;
                }
            }
            assert(minI>=0);

            //assign goal
            if (*pInfo==playerInfo || _params->numPlayersInClient==1) {
                //AG150807: only set chosen goal of this player if we have more than 1 players in this client,
                // since the playerinfo objects are referred to the same
                assert(!pInfo->chosenGoalPos.isSet());
                pInfo->chosenGoalPos = goalPosesVec[minI];
            }

            DEBUG_CLIENT(cout<<" -> "<</*pInfo->chosenGoalPos*/ goalPosesVec[minI].toString()<<endl;);

            //delete from list
            goalPosesVec.erase(goalPosesVec.begin()+minI);
        }
        DEBUG_CLIENT(cout<<endl;);
    }

    //now goal list should be empty
    assert(goalPosesVec.empty());

//#endif //USE_MIN_DIST_CHECK

    //AG160725: NOTE this should be removed, only keep fixSeekerToBeClose !
    if (_params->minDistBetweenRobots>0 && _otherSeekerPlayer!=NULL && _otherSeekerPlayer->posRead) {
        //now assure the distance between the seekers
        DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::useExplorer: assure min. distance of "<<_params->minDistBetweenRobots<<" between seekers"<<endl;);
        assert(_params->numPlayersReq<=3); //for now assert we have at max 2 seekers
        assert(_params->numPlayersInClient==1); //only  in this client
        assert(playerInfo.chosenGoalPos.isSet());
        assert(_otherSeekerPlayer->chosenGoalPos.isSet());

        DEBUG_CLIENT(cout << "Checking distance to goals: ";);
        if (        _map->distance(playerInfo.currentPos,playerInfo.chosenGoalPos) + _map->distance(_otherSeekerPlayer->currentPos,_otherSeekerPlayer->chosenGoalPos)
                <=
                    _map->distance(playerInfo.currentPos,_otherSeekerPlayer->chosenGoalPos) + _map->distance(_otherSeekerPlayer->currentPos,playerInfo.chosenGoalPos)
           ) {
            DEBUG_CLIENT(cout <<"ok as is"<<endl;);
        } else {
            DEBUG_CLIENT(cout <<"switching"<<endl;);
            Pos p = playerInfo.chosenGoalPos;
            playerInfo.chosenGoalPos = _otherSeekerPlayer->chosenGoalPos;
            _otherSeekerPlayer->chosenGoalPos = p;
        }

        Pos fixNextPos1, fixNextPos2;

        fixSeekersNotToBeClose(playerInfo.chosenGoalPos, _otherSeekerPlayer->chosenGoalPos,fixNextPos1, fixNextPos2);

        //set new goals
        assert(fixNextPos1.isSet());
        assert(fixNextPos2.isSet());
        playerInfo.chosenGoalPos = fixNextPos1;
        _otherSeekerPlayer->chosenGoalPos = fixNextPos2;

    }
}

void MultiSeekerHBExplorer::useFollower(const Pos& hiderPos) {
    assert(hiderPos.isSet());

    if (_params->minDistBetweenRobots>0) {
        assert(_params->numPlayersReq<=3); //for now assert we have at max 2 seekers
        assert(_params->numPlayersInClient==1);
        //use the explorer of the TwoSeekerHBExplorer
        TwoSeekerHBExplorer::useFollower(hiderPos);
    } else {
        if (_params->numPlayersInClient==1) {
            for(PlayerInfo* pInfo : _playerInfoVec) {
                if (pInfo->isSeeker()) {
                    pInfo->chosenGoalPos = hiderPos;
                }
            }
        } else {
            //AG150807: only set chosen goal of this player if we have more than 1 players in this client,
            // since the playerinfo objects are referred to the same
            playerInfo.chosenGoalPos = hiderPos;
        }
    }
}


std::string MultiSeekerHBExplorer::getName() const {
    stringstream ss;
    ss<<"MultiSeekerHBExplorer-of:"<<_autoPlayerWBelief->getName();
    return ss.str();
}

