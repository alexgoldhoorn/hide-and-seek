#include "Smart/twoseekerhbexplorer.h"

#include <iostream>
#include <limits>
#include <cassert>

#include "exceptions.h"
#include "Base/hsglobaldata.h"

using namespace std;

size_t TwoSeekerHBExplorer::MULTI_SEEKER_VEC_OWN_INDEX = 0;
size_t TwoSeekerHBExplorer::MULTI_SEEKER_VEC_OTHER_INDEX = 1;

TwoSeekerHBExplorer::TwoSeekerHBExplorer(SeekerHSParams* params, AutoPlayer* autoPlayerWBelief) :
    HighestBeliefFollower(params,autoPlayerWBelief), _otherSeekerPlayer(NULL)
{
    //if (_params->gameType!= HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB) {
    if (!_params->seekerSendsMultiplePoses()) { //AG150714: we cannot only accept 1 solver type, also accept ones that inherrit from this
        throw CException(_HERE_, "Expecting game type to be find-and-follow 2 robot (now is "+to_string(_params->gameType)+")");
    }
    if (_params->multiSeekerOwnObsChooseProb<0 && _params->multiSeekerOwnObsChooseProb>1) {
        throw CException(_HERE_, "The parameter multiSeekerOwnObsChooseProb should be between 0 and 1.");
    }
    if (_params->multiSeekerVisObsChooseProbIfMaybeDynObst<0 && _params->multiSeekerVisObsChooseProbIfMaybeDynObst>1) {
        throw CException(_HERE_, "The parameter multiSeekerVisObsChooseProbIfMaybeDynObst should be between 0 and 1.");
    }
    /*if (_params->multiSeekerExplorerUtilWeight<0) {
        throw CException(_HERE_,)
    }*/
    if (_params->obsEqualsThreshDist<0) {
        throw CException(_HERE_, "The parameter obsEqualsThreshDist should be positive.");
    }
    if (!autoPlayerWBelief->handles2Obs()) {
        throw CException(_HERE_, "The auto player used by TwoSeekerHBExplorer should be able to handle 2 observations.");
    }
}

TwoSeekerHBExplorer::~TwoSeekerHBExplorer()
{
}

bool TwoSeekerHBExplorer::initBeliefRun() {
    DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::initBeliefRun"<<endl;);
    //AG150605: check which autoplayer is the other seeker
    //should not be set yet
    assert(_otherSeekerPlayer==NULL);
    //there should be 2 seekers, 1 hider if it faf-2robot, otherwise at least 2 players
    assert(_playerInfoVec.size()>1);
    assert(_params->gameType!= HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB || _playerInfoVec.size()==3);
    assert(_hiderPlayer!=NULL);
    //first seeker which is not this one should be the other seeker
    for(PlayerInfo* pInfo : _playerInfoVec) {
        if (pInfo->isSeeker() && *pInfo!=playerInfo) {
            _otherSeekerPlayer = pInfo;
        }
    }

    //we should have found an other seeker
    assert(_otherSeekerPlayer!=NULL || _params->solverType!=SeekerHSParams::SOLVER_TWO_HB_EXPL);
        //AG150730: if not is twoSeekerHBExpl then this is allowed

    return true;
}

Pos TwoSeekerHBExplorer::getNextPosRun(int actionDone, int *newAction) {
    throw CException(_HERE_, "For this Solver type getNextPos should NOT be run");
}


bool TwoSeekerHBExplorer::chooseHiderObsFromMulti(Pos &chosenHider) {
    DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::chooseHiderObsFromMulti: ";);

    //AG150608: choos hider pos
    bool hiderPosConsist = true;

    if (!_otherSeekerPlayer->posRead) {
        //other not available, so use this one
        DEBUG_CLIENT(cout<<"other seeker obs not read";);
        chosenHider = playerInfo.hiderObsPosWNoise;
    } else {
        //now check which one is set
        if (_otherSeekerPlayer->hiderObsPosWNoise.isSet() && playerInfo.hiderObsPosWNoise.isSet()) {
            //both set, so check if they are consistent
            DEBUG_CLIENT(cout<<"both set, checking consistency - ";);
            if (playerInfo.hiderObsPosWNoise.distanceEuc(_otherSeekerPlayer->hiderObsPosWNoise) > _params->obsEqualsThreshDist) {
                //not consistent
                DEBUG_CLIENT(cout<<"not consistent";);
                chosenHider.clear();
                hiderPosConsist = false;
            } else {
                DEBUG_CLIENT(cout<<"consistent";);
                chosenHider = playerInfo.hiderObsPosWNoise;
            }

        } else if (playerInfo.hiderObsPosWNoise.isSet()) {
            DEBUG_CLIENT(cout<<"only this obs is visible";);
            chosenHider = playerInfo.hiderObsPosWNoise;
        } else if (_otherSeekerPlayer->hiderObsPosWNoise.isSet()) {
            DEBUG_CLIENT(cout<<"only other visible";);
            chosenHider = _otherSeekerPlayer->hiderObsPosWNoise;
        } else {
            DEBUG_CLIENT(cout<<"not visible";);
            chosenHider.clear();
        }
    }

    //AG150708: set that the goals are set
    //playerInfo.multiHasGoalPoses = true;

    DEBUG_CLIENT(cout<<" -> "<<chosenHider.toString()<<" and "<<(hiderPosConsist?"consistent":"NOT consistent")<<endl;);

    return hiderPosConsist;
}

void TwoSeekerHBExplorer::useExplorer() {
    //position of hider either not known or 'inconsistent' -> use ex
    //DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::useExplorer"<<endl;);

    //the seekers poses
    Pos seekerPos = playerInfo.currentPos;
    //vector<double> highestBeliefVec;
    //find highest belief
    vector<BPos> highestBeliefPosVec = findHighestBelief(seekerPos,_params->multiSeekerExplorerCheckNPoints,0); //,&highestBeliefVec);
    assert(highestBeliefPosVec.size()>0);

    //AG150715: initialize U vector
    vector<double> uVec(highestBeliefPosVec.size(), 1.0);

    bool calcSeeker2Goal = (_otherSeekerPlayer!=NULL && _otherSeekerPlayer->posRead);

    //get the position to go to for the current robot
    int seeker1PosI = calcExplorationEvaluation(seekerPos, highestBeliefPosVec, uVec, calcSeeker2Goal); // /*highestBeliefVec,*/ NULL);
    assert(seeker1PosI>=0 && seeker1PosI < (int)highestBeliefPosVec.size());

    //set to vector
    /*_myCalcGoalPosVec.push_back(highestBeliefPosVec[seeker1PosI]);
    _myCalcBeliefGoalPosVec.push_back(highestBeliefVec[seeker1PosI]);*/

    //AG150609: set to playerInfo vectors
    assert(playerInfo.multiGoalBPosesVec.size()==2);
    //assert(playerInfo.multiGoalBeliefVec.size()==2);
    //set goal and belief for this seeker
    playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX] = highestBeliefPosVec[seeker1PosI];
    //playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OWN_INDEX] = highestBeliefVec[seeker1PosI];
    playerInfo.multiGoalIDVec[MULTI_SEEKER_VEC_OTHER_INDEX] = playerInfo.id;

    if (calcSeeker2Goal) { //(seeker2Pos!=NULL) {
        //we have the other player's pos
        Pos seeker2Pos = _otherSeekerPlayer->currentPos;
        assert(seeker2Pos.isSet()); //because it has been read

        //get the position to go to for the current robot
        int seeker2PosI = calcExplorationEvaluation(seeker2Pos, highestBeliefPosVec, uVec, false); // /*highestBeliefVec,*/ &highestBeliefPosVec[seeker1PosI]);
        assert(seeker2PosI>=0 && seeker2PosI < (int)highestBeliefPosVec.size());
        //add to vector
        /*_myCalcGoalPosVec.push_back(highestBeliefPosVec[seeker2PosI]);
        _myCalcBeliefGoalPosVec.push_back(highestBeliefVec[seeker2PosI]);*/
        //set goal and belief for this seeker
        playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX] = highestBeliefPosVec[seeker2PosI];
        //playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OTHER_INDEX] = highestBeliefVec[seeker2PosI];
        playerInfo.multiGoalIDVec[MULTI_SEEKER_VEC_OTHER_INDEX] = _otherSeekerPlayer->id;

        if (seeker1PosI==seeker2PosI) {
            cout << "TwoSeekerHBExplorer::useExplorer: WARNING: the same goal position has been chosen for both seekers (i="<<seeker1PosI
                 <<",#="<<highestBeliefPosVec.size()<<")"<<endl;
        }


        //both seekers, choosing the one where the total distance is less (seeker1 to pos1 and s2 to p2, or vica versa)
        DEBUG_CLIENT(cout <<"TwoSeekerHBExplorer::useExplorer: checking which goal is closer: ";);
        if (        _map->distance(seekerPos,playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX]) + _map->distance(seeker2Pos,playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX])
                <=
                    _map->distance(seekerPos,playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX]) + _map->distance(seeker2Pos,playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX])
           ) {
            DEBUG_CLIENT(cout <<"ok as is"<<endl;);
        } else {
            DEBUG_CLIENT(cout <<"switching"<<endl;);
            Pos p = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX];
            playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX] = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX];
            playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX] = p;

            /*double b = playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OWN_INDEX];
            playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OWN_INDEX] = playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OTHER_INDEX];
            playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OTHER_INDEX] = b;*/

            /*Pos p = _myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX];
            _myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX] = _myCalcGoalPosVec[1];
            _myCalcGoalPosVec[1] = p;

            double b = _myCalcBeliefGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX];
            _myCalcBeliefGoalPosVec[0] = _myCalcBeliefGoalPosVec[1];
            _myCalcBeliefGoalPosVec[1] = b;*/
        }

    } else {
        //else: do nothing is the safest, don't send data until data is known about the other
        //AG150625: clear
        playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].clear();
        //playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OTHER_INDEX] = -1;
        playerInfo.multiGoalIDVec[MULTI_SEEKER_VEC_OTHER_INDEX] = _otherSeekerPlayer->id;
    }
}

void TwoSeekerHBExplorer::useFollower(const Pos& hiderPos) {
    assert(hiderPos.isSet());

    //the seekers poses
    Pos seekerPos = playerInfo.currentPos;
    Pos seeker2Pos;
    if (_otherSeekerPlayer!=NULL && _otherSeekerPlayer->posRead)
        seeker2Pos = _otherSeekerPlayer->currentPos;

    //the points to calc for both seekers
    Pos nextPos1, nextPos2;

    //check if they are close to the hider
    bool seeker1CloseToHider = seekerPos.distanceEuc(hiderPos)<=/*1.1*/_params->followPersonDist;
    bool seeker2CloseToHider = false;
    if (seeker2Pos.isSet())
        seeker2CloseToHider = seeker2Pos.distanceEuc(hiderPos)<=/*1.1*/_params->followPersonDist;

    if (seeker1CloseToHider && seeker2CloseToHider) {
        DEBUG_CLIENT(cout<<"both close to hider, stay at same pos"<<endl;);
        nextPos1 = seekerPos;
        nextPos2 = seeker2Pos;
    } else if (seeker1CloseToHider) {
        DEBUG_CLIENT(cout<<"seeker1 close - ";);
        nextPos1 = seekerPos;

        if (seeker2Pos.isSet()) {
            DEBUG_CLIENT(cout<<"approaching seeker 2"<<endl;);
            nextPos2 = calcOtherRobotPos(nextPos1, hiderPos, seeker2Pos);
        } else {
            DEBUG_CLIENT(cout<<"seeker 2's pos not known"<<endl;);
        }
    } else if(seeker2CloseToHider) {
        DEBUG_CLIENT(cout<<"seeker2 close - approaching seeker1"<<endl;);
        nextPos2 = seeker2Pos;
        nextPos1 = calcOtherRobotPos(nextPos2, hiderPos, seekerPos);
    } else {
        DEBUG_CLIENT(cout<<"none of the seekers is close - ";);

        Pos lastHiderPos = _hiderPlayer->previousPos; //AG150617: should this not be the read hider pos (for sim: previous playerInfo.hiderObsWNoise)

        bool hiderDidntMove = (!lastHiderPos.isSet() || hiderPos.distanceEuc(lastHiderPos) <= _params->hiderNotMovingDistance);

        if (hiderDidntMove) {
            DEBUG_CLIENT(cout<<"hider didn't move (or unknown dir)";);

            //angle from hider to seeker1
            double dir1 = _map->getDirection(hiderPos, seekerPos);
            //point where it has to go to
            nextPos1 = _map->tryMoveDir(dir1, hiderPos, _params->followPersonDist, false);

            if (seeker2Pos.isSet()) {
                DEBUG_CLIENT(cout<<endl;);
                //angle from hider to seeker1
                double dir2 = _map->getDirection(hiderPos, seeker2Pos);
                //point where it has to go to
                nextPos2 = _map->tryMoveDir(dir2, hiderPos, _params->followPersonDist, false);
            } else {
                DEBUG_CLIENT(cout<<" - seeker 2's pos not known"<<endl;);
            }

        } else {
            DEBUG_CLIENT(cout<<"hider moved - go behind it using it's direction";);
            //walk dir of person
            double dir = _map->getDirection(lastHiderPos, hiderPos);

            //AG15022: before calculating angle robot-hider dir, be sure that proportions
            //         of the follow dist and min robot dist are correct (otherwise asin=complex, nan)
            double followPersDist = _params->followPersonDist;
            if (_params->minDistBetweenRobots > 2*followPersDist) {
                DEBUG_CLIENT(cout<<" - follow pers dist increased to maintain min robot dist"<<endl;);
                followPersDist = _params->minDistBetweenRobots/2;
            } else {
                DEBUG_CLIENT(cout<<endl;);
            }

            //angle between robot and direction of hider
            double g = asin(_params->minDistBetweenRobots/(2*followPersDist));
            //calc poses behind robot and put each at a min dist behind them
            nextPos1 = _map->tryMoveDir/*Step*/(dir+M_PI-g, hiderPos, followPersDist, false);
                                                            //_params->seekerStepDistance, _params->doVisibCheckBeforeMove);
            nextPos2 = _map->tryMoveDir/*Step*/(dir+M_PI+g, hiderPos, followPersDist , false );
                                            //_params->seekerStepDistance, _params->doVisibCheckBeforeMove);

            if (!nextPos1.isSet()) {
                cout << "TwoSeekerHBExplorer::useFollower: WARNING next pos 1 not set, setting to hiderpos"<<endl;
                nextPos1 = hiderPos;
            }
            if (!nextPos2.isSet()) {
                cout << "TwoSeekerHBExplorer::useFollower: WARNING next pos 2 not set, setting to hiderpos"<<endl;
                nextPos2 = hiderPos;
            }

            DEBUG_CLIENT(cout <<" checking which goal is closer: ";);
            if (!seeker2Pos.isSet()) { // (seeker2Pos==NULL) {
                DEBUG_CLIENT(cout << "can't, seeker 2's pos not known - not changing"<<endl;);
            } else {
                if (        _map->distance(seekerPos,nextPos1) + _map->distance(seeker2Pos,nextPos2)
                        <=
                            _map->distance(seekerPos,nextPos2) + _map->distance(seeker2Pos,nextPos1)
                   ) {
                    DEBUG_CLIENT(cout <<"ok as is"<<endl;);
                } else {
                    DEBUG_CLIENT(cout <<"switching"<<endl;);
                    Pos p = nextPos1;
                    nextPos1 = nextPos2;
                    nextPos2 = p;
                }
            }
        }
    }

    //it could happen that the hider is too close to an obstacle and therefore the seeker's goal is inside the obstacle
    //move it to the hider's pos (we assume that this happens almost never).
    if (!nextPos1.isSet()) {
        cout << "TwoSeekerHBExplorer::useFollower: WARNING next pos 1 not set, setting to hiderpos"<<endl;
        nextPos1 = hiderPos;
    }
    /*if (!nextPos2.isSet()) {
        cout << "TwoSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 2 not set, setting to hiderpos"<<endl;
        nextPos2 = hiderPos;
    }*/

    //add to return vector
    /*_myCalcGoalPosVec.push_back(nextPos1);
    if (nextPos2.isSet())
        _myCalcGoalPosVec.push_back(nextPos2);*/

    //AG150716: adapt such that it can be used for TwoSeekerHBExplor and multi
    if (_params->solverType==SeekerHSParams::SOLVER_TWO_HB_EXPL) {
        assert(playerInfo.multiGoalBPosesVec.size()==2);
        //assert(playerInfo.multiGoalBeliefVec.size()==2);
        //set goals
        playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX] = nextPos1;
        playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX] = nextPos2;
        //beliefs
        playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b = -1;
    }

    playerInfo.chosenGoalPos = nextPos1;
    _otherSeekerPlayer->chosenGoalPos = nextPos2;
}


void TwoSeekerHBExplorer::fixSeekersNotToBeClose(const Pos& nextPos1In, const Pos& nextPos2In, Pos& nextPos1Out, Pos& nextPos2Out) {
    assert(nextPos1In.isSet());
    assert(nextPos2In.isSet());

    nextPos1Out = nextPos1In;
    nextPos2Out = nextPos2In;

    //check first if the poses are visible
    if (_map->isVisible(nextPos1In, nextPos2In,false)) { //AG150917: bug fix: first param was also nextPos2In -> was always true
        //is visible so check if they are close

        //base min dist on person follow dist and angle (we defined the positions based on the distance to the follower and the angle)
        //double minDist = 2*_params->followPersonDist*sin(_params->multiSeekerFollowAngle_rad/2);
        double d = _map->distanceEuc(nextPos1In, nextPos2In); //_myCalcGoalPosVec[0], _myCalcGoalPosVec[1]);

        //0.9: added to avoid rounding problems
        if (d < _params->minDistBetweenRobots * 0.9) {
            DEBUG_CLIENT(cout<<"too close ("<<d<<", min="<<_params->minDistBetweenRobots<<", check min="
                         <<_params->minDistBetweenRobots*0.9<< ")"<<endl;);

            //direction from goal 1 to goal 2
            //double dir = _map->getDirection(_myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX], _myCalcGoalPosVec[MULTI_SEEKER_VEC_OTHER_INDEX]);
            /*double dir = _map->getDirection(playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX],
                                            playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX]);*/
            double dir = _map->getDirection(nextPos1In, nextPos2In);
            //distance to move both to be at least at a dist. of minDist
            double moveDistEach = (_params->minDistBetweenRobots-d)/2;
            //move pos 1 (in opposite direction)
            nextPos1Out = _map->tryMoveDir/*Step*/(dir+M_PI, nextPos1In /*_myCalcGoalPosVec[0]*/, moveDistEach, false); //, _params->seekerStepDistance,false);
            //move pos 2
            nextPos2Out = _map->tryMoveDir/*Step*/(dir, nextPos2In /*_myCalcGoalPosVec[1]*/, moveDistEach, false); //_params->seekerStepDistance, false);

            if (!nextPos1Out.isSet()) {
                DEBUG_CLIENT(cout<<"  WARNING: movement of pos 1 was not possible"<<endl;);
                nextPos1Out = nextPos1In; // = _myCalcGoalPosVec[0];
            }
            if (!nextPos2Out.isSet()) {
                DEBUG_CLIENT(cout<<"  WARNING: movement of pos 2 was not possible"<<endl;);
                nextPos2Out = nextPos2In; // = _myCalcGoalPosVec[1];
            }

            //now reset values
            /*_myCalcGoalPosVec[0] = nextPos1;
            _myCalcGoalPosVec[1] = nextPos2;*/

            DEBUG_CLIENT(cout<<"  --> Moved to - s1:"<<nextPos1Out.toString()<<", s2:"<<nextPos2Out.toString()<<endl;);
        } else {
            DEBUG_CLIENT(cout<<"ok (visible but not close)"<<endl;);
        }
    } else {
        //not visible, so in theory they can't touch
        DEBUG_CLIENT(cout<<"ok (not visib. to eachother)"<<endl;);
    }
}

bool TwoSeekerHBExplorer::calcNextRobotPoses2Run(int actionDone) {
    /*getNextRobotPoses2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, std::vector<int> &actions,
                                                      std::vector<Pos> &goalPosesVec, int actionDone, int n, std::vector<double> *goalPosesBelVec) {*/

    //assert(seekerPos.isSet());
    DEBUG_CLIENT(
        cout<<"TwoSeekerHBExplorer::calcNextRobotPoses2Run: first updating belief"<<endl;//: [s1:"<<seekerPos.toString()<<",h:"<<hiderPos.toString()<<";s2:"<<flush;
    );

    assert(!playerInfo.multiHasGoalPoses);
    assert(playerInfo.initPosSet);
    assert(playerInfo.posRead);
    assert(_otherSeekerPlayer!=NULL);


    //NOTE: this could be because the simulator sends the correct hiderpos, might have to do a 'clear' of the pos
    //      (see hspomcp)
    //assert(hiderPos.isSet()==opponentVisible);

    //return vectors (also stored in class)
    /*_myCalcBeliefGoalPosVec.clear();
    _myCalcGoalPosVec.clear();*/
    //playerInfo.multiGoalBeliefVec.clear();
    playerInfo.multiGoalIDVec.clear();
    playerInfo.multiGoalBPosesVec.clear();

    //AG150608: copy values
    _autoPlayerWBelief->playerInfo.copyValuesFrom(playerInfo, true);

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

    DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::calcNextRobotPoses2Run: now select next steps - ";);

    /*_myCalcBeliefGoalPosVec.clear();
    _myCalcGoalPosVec.clear();*/

    //prepare multi goal vectors
    //from here on (here and in other functions of this class)
    //we use the first item as the current seeker, and the second as the other seeker's goal
    playerInfo.multiGoalIDVec.resize(2);
    playerInfo.multiGoalBPosesVec.resize(2);
    //playerInfo.multiGoalBeliefVec.resize(2,-1.0);
    //set indices
    playerInfo.multiGoalIDVec[MULTI_SEEKER_VEC_OWN_INDEX] = playerInfo.id;
    playerInfo.multiGoalIDVec[MULTI_SEEKER_VEC_OTHER_INDEX] = _otherSeekerPlayer->id;

    //AG150221: check if the goals for the seekers are close and move them if necessary
    bool checkIfSeekersAreClose = false;

    //2. now choose which method to use
    if (!_chosenHiderPosConsist || !_chosenHiderPos.isSet()) {
        DEBUG_CLIENT(cout<<"using explorer"<<endl;);
        useExplorer();

        if (_otherSeekerPlayer->currentPos.isSet())
            checkIfSeekersAreClose = true;
    } else {
        //hider visible and consistent observation
        DEBUG_CLIENT(cout<<"use follower"<<endl;);
        useFollower(_chosenHiderPos);
    }

    //AG150217: be sure the goals are not too close
    DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::calcNextRobotPoses2Run: be sure they are not on the same pos - ";);
    if (checkIfSeekersAreClose && _params->minDistBetweenRobots>0) { //&& _myCalcGoalPosVec.size()>1) {
        //check and fix the values such that the seeker goals are not too close
        Pos fixNextPos1, fixNextPos2;
        fixSeekersNotToBeClose(playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX], playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX],
                               fixNextPos1, fixNextPos2);

        //set the new goals
        playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX] = fixNextPos1;
        playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX] = fixNextPos2;

    } else {
        //else we don't know other's (potential) goal
        DEBUG_CLIENT(
            if (checkIfSeekersAreClose)
                cout<<"no minimum distance set"<<endl;
            else
                cout<<"already taken into account"<<endl;
        );
    }

    //AG150609: we store only the chosen hiderpos (and consistency) in _chosenHiderPos*
#ifdef OLD_CODE
    //store detected hider pos info for selectRobotPos2
    _lastHiderPos = thisHiderPos;
    _lastHiderPosConsistent = thisHiderPosConsistent;
    _lastSeekerPos = seekerPos;
    if (seeker2Pos==NULL) {
        _lastSeeker2Pos.clear();
    } else {
        _lastSeeker2Pos = *seeker2Pos;
    }

    //return vector
    if (goalPosesBelVec!=NULL) {
        goalPosesBelVec->clear();
        goalPosesBelVec->insert(goalPosesBelVec->begin(),_myCalcBeliefGoalPosVec.begin(),_myCalcBeliefGoalPosVec.end());
    }

    //copy goal poses
    goalPosesVec.clear();
    goalPosesVec.insert(goalPosesVec.begin(), _myCalcGoalPosVec.begin(), _myCalcGoalPosVec.end());
#endif

    DEBUG_CLIENT(
        cout<<"TwoSeekerHBExplorer::getNextRobotPoses2: sending to other seeker: poses: "
                <<playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].toString() /*<<" (b="<<playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OWN_INDEX]<<"), "*/<<", "
                <<playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].toString() /*<<" (b="<<playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OTHER_INDEX]<<")"*/<<endl;
        /*for (const Pos& p : playerInfo.multiGoalBeliefVec )
            cout << p.toString()<<" ";
        cout << "; beliefs: ";
        for (double d : _myCalcBeliefGoalPosVec)
            cout <<d <<" ";
        cout<<endl;*/
    );

    playerInfo.multiHasGoalPoses = true;

    return true;
}

#ifdef OLD_CODE
bool TwoSeekerHBExplorer::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    /*throw CException(_HERE_,"TwoSeekerHBExplorer::initBelief: use TwoSeekerHBExplorer::initBelief2");
    return false;*/

    bool initB = _autoPlayerWBelief->initBelief(gmap, seekerInitPos, hiderInitPos, opponentVisible);
    return initB;
}

bool TwoSeekerHBExplorer::initBelief2(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible, Pos seeker2InitPos,
                                        Pos hiderObs2InitPos, double obs1p) {
    _map = gmap;
    DEBUG_CLIENT(cout << "TwoSeekerHBExplorer::initBelief2: "<<endl;);
    _stepsToLastUpdate = 0;
    bool initB = _autoPlayerWBelief->initBelief2(gmap, seekerInitPos, hiderInitPos, opponentVisible, seeker2InitPos, hiderObs2InitPos, obs1p);

    return initB;
}

bool TwoSeekerHBExplorer::getNextRobotPoses2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, std::vector<int> &actions,
                                                      std::vector<Pos> &goalPosesVec, int actionDone, int n, std::vector<double> *goalPosesBelVec) {

    assert(seekerPos.isSet());
    DEBUG_CLIENT(
        cout<<"TwoSeekerHBExplorer::getNextRobotPoses2: [s1:"<<seekerPos.toString()<<",h:"<<hiderPos.toString()<<";s2:"<<flush;
    );

    //NOTE: this could be because the simulator sends the correct hiderpos, might have to do a 'clear' of the pos
    //      (see hspomcp)
    assert(hiderPos.isSet()==opponentVisible);

    //return vectors (also stored in class)
    _myCalcBeliefGoalPosVec.clear();
    _myCalcGoalPosVec.clear();

    //clear last hider pos
    IDPos thisHiderPos;
    bool thisHiderPosConsistent = false;

    //1. choose which observation(s) to use
    if (seeker2Pos==NULL) {
        //no second seeker pos, only use 1st
        DEBUG_CLIENT(cout<<"none] - no seeker 2 obs, using seeker 1's obs "<<endl;);

        Pos aPlayerPos = _autoPlayerWBelief->getNextPos(seekerPos, hiderPos, opponentVisible, actions, actionDone, n);

        thisHiderPos = hiderPos;
        thisHiderPosConsistent = true;
    } else {
        //there is a seeker 2 pos
        assert(seeker2Pos->isSet());
        assert(hiderObs2Pos!=NULL);
        DEBUG_CLIENT(cout<<seeker2Pos->toString()<<",h2:"<<hiderObs2Pos->toString()<<"] - ";);

        //having both observations, now check consistency
        if (hiderPos.isSet() && hiderObs2Pos->isSet()) {
            //both are set, now they can be either consistent or not
            if (hiderObs2Pos->distanceEuc(hiderPos) > _params->obsEqualsThreshDist) {
                //not equal, use both, with more prob for own
                DEBUG_CLIENT(cout<<"seeker 1's hider's obs and seeker 2's are different, using both (seeker 1 prob="
                             <<_params->multiSeekerOwnObsChooseProb<<")"<<endl;);

                Pos aPlayerPos = _autoPlayerWBelief->getNextPos2(seekerPos, hiderPos, opponentVisible, *seeker2Pos, *hiderObs2Pos,
                                                                 _params->multiSeekerOwnObsChooseProb, actions, actionDone, n);

                thisHiderPos =  hiderPos;
                thisHiderPosConsistent = false;

            } else {
                //equal, so just use it's own
                DEBUG_CLIENT(cout<<"equal, using seeker 1's obs"<<endl;);

                Pos aPlayerPos = _autoPlayerWBelief->getNextPos(seekerPos, hiderPos, opponentVisible, actions, actionDone, n);

                thisHiderPos = hiderPos;
                thisHiderPosConsistent = true;
            }
        } else if (hiderPos.isSet()) {
            //only seen by this seeker
            DEBUG_CLIENT(cout<<"hider seen by seeker 1 - ";);

            thisHiderPos = hiderPos;

            //check consistency
            if (_map->isVisible(*seeker2Pos,hiderPos,false)) {
                //inconsistent, since should be seen by seeker 2, BUT can be because of dynamic obstacle
                DEBUG_CLIENT(cout<<"inconsistent! - using both (prob of visible="<<_params->multiSeekerVisObsChooseProbIfMaybeDynObst<<")"<<endl;);
                Pos aPlayerPos = _autoPlayerWBelief->getNextPos2(seekerPos, hiderPos, opponentVisible, *seeker2Pos, *hiderObs2Pos,
                                                                 _params->multiSeekerVisObsChooseProbIfMaybeDynObst, actions, actionDone, n);

                thisHiderPosConsistent = false;
            } else {
                DEBUG_CLIENT(cout<<"consistent - using own"<<endl;);
                Pos aPlayerPos = _autoPlayerWBelief->getNextPos(seekerPos, hiderPos, opponentVisible, actions, actionDone, n);

                thisHiderPosConsistent = true;
            }

        } else if (hiderObs2Pos->isSet()) {
            //only seen by other seeker

            thisHiderPos.set(*hiderObs2Pos);

            DEBUG_CLIENT(cout<<"hider seen by seeker 2 - ";);
            //check consistency
            if (_map->isVisible(seekerPos,*hiderObs2Pos,false)) {
                //inconsistent, since should be seen by seeker, BUT can be because of dynamic obstacle
                DEBUG_CLIENT(cout<<"inconsistent! - using both (prob of visible="<<(1-_params->multiSeekerVisObsChooseProbIfMaybeDynObst)<<")"<<endl;);
                Pos aPlayerPos = _autoPlayerWBelief->getNextPos2(seekerPos, hiderPos, opponentVisible, *seeker2Pos, *hiderObs2Pos,
                                                                 1-_params->multiSeekerVisObsChooseProbIfMaybeDynObst, actions, actionDone, n);

                thisHiderPosConsistent = false;
            } else {
                DEBUG_CLIENT(cout<<"consistent - using other's"<<endl;);
                Pos aPlayerPos = _autoPlayerWBelief->getNextPos(seekerPos, thisHiderPos, true, actions, actionDone, n);

                thisHiderPosConsistent = true;
            }

        } else {
            //hidden to both
            //Passing both, because if (in worst case) an update fails and a new initial belief has to be calculated, it should
            //be calculated using both seeker's observations.
            DEBUG_CLIENT(cout<<"hidden for both, pass both"<<endl;);

            Pos aPlayerPos = _autoPlayerWBelief->getNextPos2(seekerPos, hiderPos, opponentVisible, *seeker2Pos, *hiderObs2Pos,
                                                             _params->multiSeekerOwnObsChooseProb, actions, actionDone, n);
            thisHiderPos = hiderPos; //just for ID
            thisHiderPosConsistent = true;
        }
    }

    DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::getNextRobotPoses2: now select next steps - ";);

    _myCalcBeliefGoalPosVec.clear();
    _myCalcGoalPosVec.clear();

    //AG150221: check if the goals for the seekers are close and 'compensate' if necessary
    bool checkIfSeekersAreClose = false;

    //2. now choose which method to use
    if (!thisHiderPosConsistent || !thisHiderPos.isSet()) {
        //position of hider either not known or 'inconsistent' -> use ex
        DEBUG_CLIENT(cout<<"use explorer"<<endl;);

        //find highest belief
        vector<double> highestBeliefVec;
        vector<Pos> highestBeliefPosVec = findHighestBelief(seekerPos,_params->multiSeekerExplorerCheckNPoints,0,&highestBeliefVec);
        assert(highestBeliefPosVec.size()>0);

        //get the position to go to for the current robot
        int s1i = calcExplorationEvaluation(seekerPos, highestBeliefPosVec, highestBeliefVec, NULL);
        assert(s1i>=0 && s1i < (int)highestBeliefPosVec.size());
        //add to vector
        _myCalcGoalPosVec.push_back(highestBeliefPosVec[s1i]);
        _myCalcBeliefGoalPosVec.push_back(highestBeliefVec[s1i]);

        if (seeker2Pos!=NULL) {
            //get the position to go to for the current robot
            int s2i = calcExplorationEvaluation(*seeker2Pos, highestBeliefPosVec, highestBeliefVec, &highestBeliefPosVec[s1i]);
            assert(s2i>=0 && s2i < (int)highestBeliefPosVec.size());
            //add to vector
            _myCalcGoalPosVec.push_back(highestBeliefPosVec[s2i]);
            _myCalcBeliefGoalPosVec.push_back(highestBeliefVec[s2i]);

            if (s1i==s2i) {
                cout << "TwoSeekerHBExplorer::getNextRobotPoses2: WARNING: the same goal position has been chosen for both seekers (i="<<s1i
                     <<",#="<<highestBeliefPosVec.size()<<")"<<endl;
            }


            //both seekers, choosing the one where the total distance is less (seeker1 to pos1 and s2 to p2, or vica versa)
            DEBUG_CLIENT(cout <<"TwoSeekerHBExplorer::getNextRobotPoses2: checking which goal is closer: ";);
            if (        _map->distance(seekerPos,_myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX]) + _map->distance(*seeker2Pos,_myCalcGoalPosVec[1])
                    <=
                        _map->distance(seekerPos,_myCalcGoalPosVec[1]) + _map->distance(*seeker2Pos,_myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX])
               ) {
                DEBUG_CLIENT(cout <<"ok as is"<<endl;);
            } else {
                DEBUG_CLIENT(cout <<"switching"<<endl;);
                Pos p = _myCalcGoalPosVec[0];
                _myCalcGoalPosVec[0] = _myCalcGoalPosVec[1];
                _myCalcGoalPosVec[1] = p;

                double b = _myCalcBeliefGoalPosVec[0];
                _myCalcBeliefGoalPosVec[0] = _myCalcBeliefGoalPosVec[1];
                _myCalcBeliefGoalPosVec[1] = b;
            }

            checkIfSeekersAreClose = true;

        } //else: do nothing is the safest, don't send data until data is known about the other

    } else {
        //hider visible and consistent observation
        DEBUG_CLIENT(cout<<"use follower - ";);

        assert(thisHiderPos.isSet()); //at least hider pos set, not necessarily consistent

        //the points to calc
        Pos nextPos1, nextPos2;

        //check if they are close to the hider
        bool seeker1CloseToHider = seekerPos.distanceEuc(thisHiderPos)<=/*1.1*/_params->followPersonDist;
        bool seeker2CloseToHider = false;
        if (seeker2Pos!=NULL)
            seeker2CloseToHider = seeker2Pos->distanceEuc(thisHiderPos)<=/*1.1*/_params->followPersonDist;

        if (seeker1CloseToHider && seeker2CloseToHider) {
            DEBUG_CLIENT(cout<<"both close to hider, stay at same pos"<<endl;);
            nextPos1 = seekerPos;
            nextPos2 = *seeker2Pos;
        } else if (seeker1CloseToHider) {
            DEBUG_CLIENT(cout<<"seeker1 close - ";);
            nextPos1 = seekerPos;

            if (seeker2Pos!=NULL) {
                DEBUG_CLIENT(cout<<"approaching seeker 2"<<endl;);
                nextPos2 = calcOtherRobotPos(nextPos1, thisHiderPos, *seeker2Pos);
            } else {
                DEBUG_CLIENT(cout<<"seeker 2's pos not known"<<endl;);
            }
        } else if(seeker2CloseToHider) {
            DEBUG_CLIENT(cout<<"seeker2 close - approaching seeker1"<<endl;);
            nextPos2 = *seeker2Pos;
            nextPos1 = calcOtherRobotPos(nextPos2, thisHiderPos, seekerPos);
        } else {
            DEBUG_CLIENT(cout<<"none of the seekers is close - ";);

            bool hiderDidntMove = (!_lastHiderPos.isSet() || thisHiderPos.distanceEuc(_lastHiderPos) <= _params->hiderNotMovingDistance);

            if (hiderDidntMove) {
                DEBUG_CLIENT(cout<<"hider didn't move (or unknown dir)";);

                //angle from hider to seeker1
                double dir1 = _map->getDirection(thisHiderPos, seekerPos);
                //point where it has to go to
                nextPos1 = _map->tryMoveDir(dir1, thisHiderPos, _params->followPersonDist, false);

                if (seeker2Pos!=NULL) {
                    DEBUG_CLIENT(cout<<endl;);
                    //angle from hider to seeker1
                    double dir2 = _map->getDirection(thisHiderPos, *seeker2Pos);
                    //point where it has to go to
                    nextPos2 = _map->tryMoveDir(dir2, thisHiderPos, _params->followPersonDist, false);
                } else {
                    DEBUG_CLIENT(cout<<" - seeker 2's pos not known"<<endl;);
                }

            } else {
                DEBUG_CLIENT(cout<<"hider moved - go behind it using it's direction";);
                //walk dir of person
                double dir = _map->getDirection(_lastHiderPos, thisHiderPos);

                //AG15022: before calculating angle robot-hider dir, be sure that proportions
                //         of the follow dist and min robot dist are correct (otherwise asin=complex, nan)
                double followPersDist = _params->followPersonDist;
                if (_params->minDistBetweenRobots > 2*followPersDist) {
                    DEBUG_CLIENT(cout<<" - follow pers dist increased to maintain min robot dist"<<endl;);
                    followPersDist = _params->minDistBetweenRobots/2;
                } else {
                    DEBUG_CLIENT(cout<<endl;);
                }

                //angle between robot and direction of hider
                double g = asin(_params->minDistBetweenRobots/(2*followPersDist));
                //calc poses behind robot and put each at a min dist behind them
                nextPos1 = _map->tryMoveDir/*Step*/(dir+M_PI-g, thisHiderPos, followPersDist, false);
                                                                //_params->seekerStepDistance, _params->doVisibCheckBeforeMove);
                nextPos2 = _map->tryMoveDir/*Step*/(dir+M_PI+g, thisHiderPos, followPersDist , false );
                                                //_params->seekerStepDistance, _params->doVisibCheckBeforeMove);

                if (!nextPos1.isSet()) {
                    cout << "TwoSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 1 not set, setting to hiderpos"<<endl;
                    nextPos1 = thisHiderPos;
                }
                if (!nextPos2.isSet()) {
                    cout << "TwoSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 2 not set, setting to hiderpos"<<endl;
                    nextPos2 = thisHiderPos;
                }

                DEBUG_CLIENT(cout <<" checking which goal is closer: ";);
                if (seeker2Pos==NULL) {
                    DEBUG_CLIENT(cout << "can't, seeker 2's pos not known - not changing"<<endl;);
                } else {
                    if (        _map->distance(seekerPos,nextPos1) + _map->distance(*seeker2Pos,nextPos2)
                            <=
                                _map->distance(seekerPos,nextPos2) + _map->distance(*seeker2Pos,nextPos1)
                       ) {
                        DEBUG_CLIENT(cout <<"ok as is"<<endl;);
                    } else {
                        DEBUG_CLIENT(cout <<"switching"<<endl;);
                        Pos p = nextPos1;
                        nextPos1 = nextPos2;
                        nextPos2 = p;
                    }
                }
            }
        }


        //it could happen that the hider is too close to an obstacle and therefore the seeker's goal is inside the obstacle
        //move it to the hider's pos (we assume that this happens almost never).
        if (!nextPos1.isSet()) {
            cout << "TwoSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 1 not set, setting to hiderpos"<<endl;
            nextPos1 = thisHiderPos;
        }
        /*if (!nextPos2.isSet()) {
            cout << "TwoSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 2 not set, setting to hiderpos"<<endl;
            nextPos2 = hiderPos;
        }*/

        //add to return vector
        _myCalcGoalPosVec.push_back(nextPos1);
        if (nextPos2.isSet())
            _myCalcGoalPosVec.push_back(nextPos2);


        //TODO: ALL BELOW IN THIS BLOCK CAN BE REMOVED OR DISABLED
#ifdef OLD_FOLLOWER
        //get the two positions for the seekers
        //AG150219: only use direction of hider when it really moved
        if (_lastHiderPos.isSet() && _lastHiderPosConsistent && !hiderDidntMove) {
            //use the direction, and calculate two points behind them, choose the closest
            DEBUG_CLIENT(cout<<"go behind person"<<endl;);

            //AG150130 NOTE/WARNING: this could be really sensitive to noise when this code is executed at a too high frequency
            //walk dir of person
            double dir = _map->getDirection(_lastHiderPos, thisHiderPos);

            DEBUG_CLIENT(cout<<" Person's' dir="<<(180*dir/M_PI)<<"º";);

            //now get the two poses behind the robot
            //(first get the person's direction, get the opposite direction (add 180º), then add/subtr. the angle/2 between the 2 seekers,
            // and set the goal point to followPersonDist)
            /*nextPos1 = _map->tryMoveDirStep(dir+M_PI-_params->multiSeekerFollowAngle_rad/2, thisHiderPos, _params->followPersonDist,
                                                _params->seekerStepDistance, _params->doVisibCheckBeforeMove);
            nextPos2 = _map->tryMoveDirStep(dir+M_PI+_params->multiSeekerFollowAngle_rad/2, thisHiderPos, _params->followPersonDist,
                                                _params->seekerStepDistance, _params->doVisibCheckBeforeMove);*/

            DEBUG_CLIENT(cout<<" --> pos1:"<<nextPos1.toString()<<", pos2:"<<nextPos2.toString(););

            //now decide which seeker goes to which point
            if (seeker2Pos==NULL) {
                //only for this seeker, get closest
                if (_map->distance(seekerPos,nextPos1)<_map->distance(seekerPos,nextPos2)) {
                    DEBUG_CLIENT(cout <<" - Only seeker1, choosing pos1"<<endl;);
                    _myCalcGoalPosVec.push_back(nextPos1);
                } else {
                    DEBUG_CLIENT(cout <<" - Only seeker1, choosing pos2"<<endl;);
                    _myCalcGoalPosVec.push_back(nextPos2);
                }
            } else {
                //both seekers, choosing the one where the total distance is less
                if (        _map->distance(seekerPos,nextPos1) + _map->distance(*seeker2Pos,nextPos2)
                        <
                            _map->distance(seekerPos,nextPos2) + _map->distance(*seeker2Pos,nextPos1)
                   ) {
                    DEBUG_CLIENT(cout <<" - seeker1 to pos1, seeker 2 to pos2"<<endl;);
                    _myCalcGoalPosVec.push_back(nextPos1);
                    _myCalcGoalPosVec.push_back(nextPos2);
                } else {
                    DEBUG_CLIENT(cout <<" - seeker1 to pos2, seeker 2 to pos1"<<endl;);
                    _myCalcGoalPosVec.push_back(nextPos2);
                    _myCalcGoalPosVec.push_back(nextPos1);
                }
            }

        } else { //hider not seen last time or not consistent, or hider without moving

            //last step hider has not be seen (or the hider didn't move), go to the closest location to it
            DEBUG_CLIENT(cout<<"go close to person - ";);

            //AG150220: check if the seekers are not already close to the hider, otherwise go in their direction

            if (/*hiderDidntMove &&*/ seekerPos.distanceEuc(thisHiderPos)<=1.1*_params->followPersonDist) {
                DEBUG_CLIENT(cout<<"seeker 1 already close - ";);
                nextPos1 = seekerPos;
            } else {
                DEBUG_CLIENT(cout<<"seeker 1 not close to hider yet -";);

                //direction from person to seeker
                double dir = _map->getDirection(thisHiderPos, seekerPos);

                //set goal in direction to seeker, at followPersDist
                nextPos1 = _map->tryMoveDirStep(dir, thisHiderPos, _params->followPersonDist,
                                                    _params->seekerStepDistance, _params->doVisibCheckBeforeMove);
            }
            _myCalcGoalPosVec.push_back(nextPos1);

            if (seeker2Pos!=NULL) {

                if (/*hiderDidntMove &&*/ seeker2Pos->distanceEuc(thisHiderPos)<=1.1*_params->followPersonDist) {
                    DEBUG_CLIENT(cout<<"seeker 2 already close - ";);
                    nextPos2 = *seeker2Pos;
                } else {
                    DEBUG_CLIENT(cout<<"seeker 2 not close to hider yet - ";);
                    double dir = _map->getDirection(thisHiderPos, *seeker2Pos);

                    //set goal in direction to seeker, at followPersDist
                    nextPos2 = _map->tryMoveDirStep(dir, thisHiderPos, _params->followPersonDist,
                                                        _params->seekerStepDistance, _params->doVisibCheckBeforeMove);
                }

                _myCalcGoalPosVec.push_back(nextPos2);
            }

            DEBUG_CLIENT(cout<<" --> pos s1:"<<nextPos1.toString()<<", pos s2:"<<nextPos2.toString()<<endl;);

        } //if lasthiderPos.set ...
#endif
    } // if-else thisHiderPos set and consistent

    //AG150217: be sure the goals are not too close
    DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::getNextRobotPoses2: be sure they are not on the same pos - ";);
    if (checkIfSeekersAreClose && _params->minDistBetweenRobots>0 && _myCalcGoalPosVec.size()>1) {
        assert(_myCalcGoalPosVec[0].isSet());
        assert(_myCalcGoalPosVec[1].isSet());

        //check first if the poses are visible
        if (_map->isVisible(_myCalcGoalPosVec[0],_myCalcGoalPosVec[1],false)) {
            //is visible so check if they are close

            //base min dist on person follow dist and angle (we defined the positions based on the distance to the follower and the angle)
            //double minDist = 2*_params->followPersonDist*sin(_params->multiSeekerFollowAngle_rad/2);
            double d = _map->distanceEuc(_myCalcGoalPosVec[0], _myCalcGoalPosVec[1]);

            //0.9: added to avoid rounding problems
            if (d < _params->minDistBetweenRobots * 0.9) {
                DEBUG_CLIENT(cout<<"too close ("<<d<<", min="<<_params->minDistBetweenRobots<<", check min="<<_params->minDistBetweenRobots*0.9<< ")"<<endl;);

                //direction from goal 1 to goal 2
                double dir = _map->getDirection(_myCalcGoalPosVec[0], _myCalcGoalPosVec[1]);
                //distance to move both to be at least at a dist. of minDist
                double moveDistEach = (_params->minDistBetweenRobots-d)/2;
                //move pos 1 (in opposite direction)
                Pos nextPos1 = _map->tryMoveDir/*Step*/(dir+M_PI, _myCalcGoalPosVec[0], moveDistEach, false); //, _params->seekerStepDistance,false);
                //move pos 2
                Pos nextPos2 = _map->tryMoveDir/*Step*/(dir, _myCalcGoalPosVec[1], moveDistEach, false); //_params->seekerStepDistance, false);

                if (!nextPos1.isSet()) {
                    DEBUG_CLIENT(cout<<"  WARNING: movement of pos 1 was not possible"<<endl;);
                    nextPos1 = _myCalcGoalPosVec[0];
                }
                if (!nextPos2.isSet()) {
                    DEBUG_CLIENT(cout<<"  WARNING: movement of pos 2 was not possible"<<endl;);
                    nextPos2 = _myCalcGoalPosVec[1];
                }

                //now reset values
                _myCalcGoalPosVec[0] = nextPos1;
                _myCalcGoalPosVec[1] = nextPos2;

                DEBUG_CLIENT(cout<<"  --> Moved to - s1:"<<nextPos1.toString()<<", s2:"<<nextPos2.toString()<<endl;);
            } else {
                DEBUG_CLIENT(cout<<"ok (visible but not close)"<<endl;);
            }
        } else {
            //not visible, so in theory they can't touch
            DEBUG_CLIENT(cout<<"ok (not visib. to eachother)"<<endl;);
        }
    } else {
        //else we don't know other's (potential) goal
        DEBUG_CLIENT(
            if (checkIfSeekersAreClose)
                cout<<"can't be tested, only "<<_myCalcGoalPosVec.size()<<" pose known"<<endl;
            else
                cout<<"already taken into account"<<endl;
        );
    }

    //store detected hider pos info for selectRobotPos2
    _lastHiderPos = thisHiderPos;
    _lastHiderPosConsistent = thisHiderPosConsistent;
    _lastSeekerPos = seekerPos;
    if (seeker2Pos==NULL) {
        _lastSeeker2Pos.clear();
    } else {
        _lastSeeker2Pos = *seeker2Pos;
    }

    //return vector
    if (goalPosesBelVec!=NULL) {
        goalPosesBelVec->clear();
        goalPosesBelVec->insert(goalPosesBelVec->begin(),_myCalcBeliefGoalPosVec.begin(),_myCalcBeliefGoalPosVec.end());
    }

    //copy goal poses
    goalPosesVec.clear();
    goalPosesVec.insert(goalPosesVec.begin(), _myCalcGoalPosVec.begin(), _myCalcGoalPosVec.end());

    DEBUG_CLIENT(
        cout<<"TwoSeekerHBExplorer::getNextRobotPoses2: sending to other seeker: poses: ";
        for (const Pos& p : _myCalcGoalPosVec )
            cout << p.toString()<<" ";
        cout << "; beliefs: ";
        for (double d : _myCalcBeliefGoalPosVec)
            cout <<d <<" ";
        cout<<endl;
    );

    return true;
}
#endif

int TwoSeekerHBExplorer::calcExplorationEvaluation(const Pos &seekerPos, const std::vector<BPos> &highestBeliefPosVec,
                                                   std::vector<double>& uVec, bool updateU) {

    //assert(highestBeliefPosVec.size()==highestBeliefVec.size());
    DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::calcExplorationEvaluation: for seeker "<<seekerPos.toString()/*<<", other="<<
                 (otherRobotGoal==NULL?"-":otherRobotGoal->toString())*/<<":"<<endl;);

    assert(seekerPos.isSet());
    //there should be at least 1 highest belief point, and it should be set
    assert(highestBeliefPosVec.size()>0);
    assert(highestBeliefPosVec[/*MULTI_SEEKER_VEC_OWN_INDEX*/0].isSet());
    assert(highestBeliefPosVec.size()==uVec.size());
    //AG150213: each Pos should have a belief
    //assert(highestBeliefPosVec.size() == highestBeliefVec.size());

    //index of max value to be found
    int maxValueI = -1;

    if (highestBeliefPosVec.size()==1) {
        //AG150213: only 1 item, so this is the highest/only option
        DEBUG_CLIENT(cout<<"  --> only 1, so I="<<maxValueI<<endl;);
        maxValueI = 0;

    } else {
        //max value of the equation
        double maxValue = numeric_limits<double>::lowest();

        //get maximum belief and distance for normalization
        double maxDist = _map->distance(highestBeliefPosVec[MULTI_SEEKER_VEC_OWN_INDEX], seekerPos);
        double maxBelief = highestBeliefPosVec[MULTI_SEEKER_VEC_OWN_INDEX].b;
        for(size_t i = 1; i < highestBeliefPosVec.size(); i++) {
            if (highestBeliefPosVec[i].isSet()) {
                //check highest distance
                double d = _map->distance(highestBeliefPosVec[i], seekerPos);
                if (d > maxDist)
                    maxDist = d;
                //check highest belief
                if (highestBeliefPosVec[i].b > maxDist)
                    maxDist = highestBeliefPosVec[i].b;
            }
        }

        if (maxDist==0) {
            //AG150213: max dist = 0, so this is the best one, reset to 1 to prevent division by 0
            DEBUG_CLIENT(cout<<"  maxDist=0, so resetting to 1 (for calculations)"<<endl;);
            maxDist = 1;
        }

        //check all highest belief points
        for(size_t i=0; i < highestBeliefPosVec.size(); i++) {
            if (highestBeliefPosVec[i].isSet()) {  //highestBeliefVec[i]>0) { //only when there is a belief (otherwise it's not a point to use)
                //distance high bel. point - seeker
                double d = _map->distance(highestBeliefPosVec[i], seekerPos);
                //calculate the value
                double y = _params->multiSeekerExplorerUtilWeight * uVec[i] - _params->multiSeekerExplorerDistWeight * d / maxDist +
                            _params->multiSeekerExplorerBeliefWeight * highestBeliefPosVec[i].b / maxBelief;

                if (y>maxValue) {
                    maxValue = y;
                    maxValueI = i;
                }

                DEBUG_CLIENT(cout<<"  "<<i<<") pos="<<highestBeliefPosVec[i].toString()<<",u="<<uVec[i]<<",d="<<d<</*",b="<<highestBeliefVec[i]<<*/", y="<<y<<endl;)
            }
        }

        DEBUG_CLIENT(cout<<"  --> max I="<<maxValueI<<" ("<<maxValue<<")"<<endl;);
    }

    //AG150715: max value index should be a legal index
    assert(maxValueI>=0 && maxValueI<(int)highestBeliefPosVec.size());

    //AG150715: now update U
    //utility
    if (updateU) {
        //update u for all highest belief values
        for(size_t i=0; i < highestBeliefPosVec.size(); i++) {
            double d = 0;
            if ((int)i!=maxValueI) {
                //calc distance (otherwise 0)
                d = _map->distance(highestBeliefPosVec[i], highestBeliefPosVec[maxValueI]);
            }
            if (d < _params->multiSeekerExplorerMaxRange) {
                uVec[i] -= 1 - d/_params->multiSeekerExplorerMaxRange;
            }
        }
    }

    return maxValueI;
}

Pos TwoSeekerHBExplorer::calcOtherRobotPos(const Pos& seekerPos, const Pos& hiderPos, const Pos& otherSeekerPos) {
    assert(seekerPos.isSet());
    assert(hiderPos.isSet());
    assert(otherSeekerPos.isSet());

    static const double M_2PI=2*M_PI;

    //minimum angle between seekers (with min dist to eachother and to hider)
    double angle = 2 * asin( _params->minDistBetweenRobots/(2*_params->followPersonDist));
    //direction of already located seeker as seen from hider
    double b1 = _map->getDirection(hiderPos,seekerPos);
    //the other seeker can be at a min distance to hider if the angle is between a1 and a2
    double a1 = (b1+angle);
    if (a1>M_2PI) a1 -= M_2PI;
    double a2 = (b1-angle);
    if (a2<0) a2 += M_2PI;
    //current direction hider-other seeker
    double b2 = _map->getDirection(hiderPos,otherSeekerPos);

    //goal direction other seeker
    double c =-1;

    if (a1>a2) {
        //the allowed range contains the 0 rad direction
        if ((b2>=a1 && b2<=M_2PI) || (b2>=0 && b2<=a2)) {
            //already correct direction
            c = b2; //already correct direction
        } else //find closest
            if (b2>b1) {
                c = a1;
            } else {
                c = a2;
            }

    } else {
        //0 rad dir not included
        if (b2>=a1 && b2<=a2) {
            //already correct direction
            c = b2;
        } else { //find closest
            if (b1<a1) { //seeker1 dir between 0 rad and a1 rad
                if (b2<b1 || b2>a2) {
                    c = a2;
                } else {
                    c = a1;
                }
            } else {
                if (b2>a2 && b2<b1) {
                    c = a2;
                } else {
                    c = a1;
                }
            }
        }
    }

    Pos newSeeker2Pos = _map->tryMoveDir(c,hiderPos,_params->followPersonDist,false);

    if (!newSeeker2Pos.isSet()) {
        //should occur only when hider close to obstacle
        cout << "TwoSeekerHBExplorer::calcOtherRobotPos: WARNING: new pos not consistent/correct, using hider's pos"<<endl;
        newSeeker2Pos = hiderPos;
    }

    return newSeeker2Pos;
}

Pos TwoSeekerHBExplorer::selectRobotPosMulti() {
    /* RobotPos2(Pos *otherSeekerPos1, Pos *otherSeekerPos2, double otherSeekerPos1B, double otherSeekerPos2B, int n, Pos* chosenPos) {*/

    assert(_otherSeekerPlayer!=NULL);
    assert(playerInfo.multiHasGoalPoses);
    //we assume the vectors are 2 since this 'solver' is made for 2 seekers
    //goal poses vector should be 2: this and other seeker
    assert(playerInfo.multiGoalIDVec.size()==2);
    assert(playerInfo.multiGoalBPosesVec.size()==2);
    //assert(playerInfo.multiGoalPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].isSet() && playerInfo.multiGoalPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].isSet());
    //belief either set for both or none
    //assert(playerInfo.multiGoalBeliefVec.size()==2 || playerInfo.multiGoalBeliefVec.size()==0);
    //if the other seeker's goals have been read it they should be set
    assert(!_otherSeekerPlayer->multiHasGoalPoses ||
           (_otherSeekerPlayer->multiGoalBPosesVec.size()==2 &&
                _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].isSet()
             //&&                _otherSeekerPlayer->multiGoalPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].isSet()
                //AG150708: the own pos should be set, the other might not be set (could have conn. problems)
            )
    );
    //chosen goal pos should not yet be set
    assert(!playerInfo.chosenGoalPos.isSet());

    DEBUG_CLIENT(
        cout<<"TwoSeekerHBExplorer::selectRobotPosMulti - goals: own: "
                << playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].toString() /*<<" (b="<<playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OWN_INDEX]*/<<"); other: "
                << playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].toString() /*<<" (b="<<playerInfo.multiGoalBeliefVec[MULTI_SEEKER_VEC_OTHER_INDEX]*/<<") - ";
    );

    //check if we want to update the next point: when either a timer passed, the hider is visible, or close to the previoulsy calculated position
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

        //TODO: here retrieve all poses (maybe we don't need to receive our own calculated data...)
        //check timer and if hider visible or not .. if timer not started or passed, or hider visible, OR ..
        //  then: calculate and return a new location; otherwise return prev. calculated location

        if (!_otherSeekerPlayer->multiHasGoalPoses ||
                !_otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].isSet()) {

            //either not multi goal poses received, OR only of its own
            DEBUG_CLIENT(cout<<"other has no pos for this seeker therefore keeping own"<<endl;);

            //without belief we cannot choose (or give pref to seeker which is closer..)
            //returnPos = _myCalcGoalPosVec[0];
            playerInfo.chosenGoalPos = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX];

        } else {
            DEBUG_CLIENT(cout<<"other has pos for this seeker - decide which to use - ";);

            //now do check of distances of both's goals to make decision
            //first check if there are beliefs
            //AG150708: added other index test, because in theory we can have only 1 goal, if for example the other pos was not set/read
            bool myHasBelief = (playerInfo.multiGoalBPosesVec.size()==2 && playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b>=0
                                                                        && playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b>=0);
            assert(!myHasBelief || (playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b<=1 &&
                                    playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b<=1)
            );
            //other has belief (only usefull when both have a belief)
            bool otherHasBelief = (_otherSeekerPlayer->multiGoalBPosesVec.size()==2 &&
                                   _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b>=0
                                   && _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b>=0);
            assert(!otherHasBelief || (_otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b<=1 &&
                                       _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b<=1)
            );

            if (myHasBelief && otherHasBelief && _otherSeekerPlayer->posRead && _otherSeekerPlayer->currentPos.isSet() ) {
                //use belief
                DEBUG_CLIENT(cout<<"belief available"<<endl;);

                //AG150319: disabled old check of d11_22 and belief own with belief other bigger etc.. then own (was unnecessary and illogical)

                //calculate the distances of own goal pos, and other's goal pos for this robot
                double d11_21 = _map->distance(playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX],
                                               _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX]);
                double d12_22 = _map->distance(playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX],
                                               _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX]);

                DEBUG_CLIENT(cout<<" small/large rule, belief: "<<(_params->multiSeekerSelectPosUseMaxBelief?"max":"sum")<<" - ";);

                if (d11_21<_params->multiSeekerSelectPosSmallDistBetwGoals) {

                    if (d12_22<_params->multiSeekerSelectPosSmallDistBetwGoals) {

                        DEBUG_CLIENT(cout<<" small-small - ";);
                        //compare beliefs
                        if ((_params->multiSeekerSelectPosUseMaxBelief &&
                                max(playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b,
                                    playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b) >
                                max(_otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b,
                                    _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b)
                           ) ||
                           (!_params->multiSeekerSelectPosUseMaxBelief &&
                                (playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b +
                                 playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b) >
                                (_otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b +
                                 _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b)
                           )
                        ) {
                            DEBUG_CLIENT(cout << "own belief higher"<<endl;);
                            playerInfo.chosenGoalPos = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX];
                        } else {
                            DEBUG_CLIENT(cout << "other belief higher"<<endl;);
                            playerInfo.chosenGoalPos = _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX];
                            //returnPos = *otherSeekerPos1;
                        }

                    } else {
                        DEBUG_CLIENT(cout<<" small-large - ";);
                        if (_otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b >
                                playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b) {
                            DEBUG_CLIENT(cout << "other belief higher"<<endl;);
                            playerInfo.chosenGoalPos = _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX];
                            //returnPos = *otherSeekerPos1;
                        } else {
                            DEBUG_CLIENT(cout << "own belief higher"<<endl;);
                            //returnPos = _myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX];
                            playerInfo.chosenGoalPos = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX];
                        }
                    }
                } else {

                    if (d12_22<_params->multiSeekerSelectPosSmallDistBetwGoals) {
                        DEBUG_CLIENT(cout<<" large-small - ";);
                        if (playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b>_otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b) {
                            DEBUG_CLIENT(cout << "own belief higher"<<endl;);
                            //returnPos = _myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX];
                            playerInfo.chosenGoalPos = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX];
                        } else {
                            DEBUG_CLIENT(cout << "other belief higher"<<endl;);
                            //returnPos = *otherSeekerPos1;
                            playerInfo.chosenGoalPos = _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX];
                        }

                    } else {

                        DEBUG_CLIENT(cout<<" large-large - ";);
                        //compare beliefs
                        if ((_params->multiSeekerSelectPosUseMaxBelief &&
                                max(playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b,
                                    playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b) >
                                max(_otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b,
                                    _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b)
                           ) ||
                           (!_params->multiSeekerSelectPosUseMaxBelief &&
                                (playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b +
                                 playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b) >
                                (_otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX].b +
                                 _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX].b)
                           )
                        ) {
                            DEBUG_CLIENT(cout << "own belief higher"<<endl;);
                            playerInfo.chosenGoalPos = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX];
                        } else {
                            DEBUG_CLIENT(cout << "other belief higher"<<endl;);
                            playerInfo.chosenGoalPos = _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX];
                            //returnPos = *otherSeekerPos1;
                        }
                    }
                }


            } else if  (_otherSeekerPlayer->posRead && _otherSeekerPlayer->currentPos.isSet() &&
                        _chosenHiderPos.isSet() && _chosenHiderPosConsist)
            {
                            //(_lastSeeker2Pos.isSet() && _lastHiderPos.isSet()) {
                //no belief available, but we have the positions,

                DEBUG_CLIENT(cout<<(myHasBelief && otherHasBelief?"":"no ")
                             <<"belief available and seeker 2 pos set, and last hider pos set - checking prefered - ";);

                //use the distance to decide which of the seekers can decide the position
                if (_map->distanceEuc(playerInfo.currentPos, _chosenHiderPos) <=
                        _map->distanceEuc(_otherSeekerPlayer->currentPos, _chosenHiderPos)) {

                    DEBUG_CLIENT(cout<<"this seeker's goal closer - keeping own"<<endl;);
                    //returnPos = _myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX];
                    playerInfo.chosenGoalPos = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX];

                } else {
                    DEBUG_CLIENT(cout<<" other seeker's goal closer - setting other"<<endl;);
                    //returnPos = *otherSeekerPos1;
                    playerInfo.chosenGoalPos = _otherSeekerPlayer->multiGoalBPosesVec[MULTI_SEEKER_VEC_OTHER_INDEX];
                }

                //TODO check: this could cause changes in movements of seeker
            } else {
                DEBUG_CLIENT(cout<<(myHasBelief?"":"NO ")<<"belief available for this seeker; "
                             <<(otherHasBelief?"":"NO ")<<"belief available for other seeker; "
                             <<"other player's pos "<<(_otherSeekerPlayer->posRead?"":"NOT ")
                             <<"read, and "<<(_otherSeekerPlayer->currentPos.isSet()?"":"NOT ")
                             <<" set; chosen hider pos was " <<(_chosenHiderPosConsist?"":"NOT ")
                             <<"consistent, and "<<(_chosenHiderPos.isSet()?"":" NOT")<<" set - keeping own"<<endl;
                );

                //returnPos = _myCalcGoalPosVec[MULTI_SEEKER_VEC_OWN_INDEX];
                playerInfo.chosenGoalPos = playerInfo.multiGoalBPosesVec[MULTI_SEEKER_VEC_OWN_INDEX];
            }

        } //if-else _otherSeekerPlayer->multiHasGoalPoses

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
        cout <<"TwoSeekerHBExplorer::selectRobotPosMulti: WARNING: time passed and no last pos found"<<endl;
        //returnPos = _lastSeekerPos;
        playerInfo.chosenGoalPos = playerInfo.currentPos;
    }

    DEBUG_CLIENT(cout<<"TwoSeekerHBExplorer::selectRobotPosMulti: --> chosen goal pos: "<<playerInfo.chosenGoalPos.toString(););

    if (/*hiderVisible ||*/ _params->onlySendStepGoals) {
        //get the next step towards goal, using 'navigation'
        //AG150220: stayFromGoal=false because this is already taken into account
        //returnPos = getNextPosAsStep(_lastSeekerPos, returnPos, n, false /*hiderVisible*/);

        playerInfo.nextPos = getNextPosAsStep(playerInfo.currentPos, playerInfo.chosenGoalPos, 1 /*n*/, false /*hiderVisible*/);

        DEBUG_CLIENT(cout<<", in steps, next step: "<<playerInfo.nextPos.toString(););
    } else {
        //AG150610: no steps so the next pos is the pos we've chosen here
        playerInfo.nextPos = playerInfo.chosenGoalPos;
    }
    DEBUG_CLIENT(cout<<endl;);

    //update steps counter
    if (_stepsToLastUpdate>0)
        _stepsToLastUpdate--;

    return playerInfo.nextPos;
}

#ifdef OLD_CODE
Pos TwoSeekerHBExplorer::selectRobotPos2(Pos *otherSeekerPos1, Pos *otherSeekerPos2, double otherSeekerPos1B, double otherSeekerPos2B,
                                           int n, Pos* chosenPos) {

/*cout << "selectrobotpos2: otherpos1="<<(otherSeekerPos1==NULL?"null":otherSeekerPos1->toString())
     << ";otherpos2="<<(otherSeekerPos2==NULL?"null":otherSeekerPos2->toString())
     << ";otherpos1b="<<otherSeekerPos1B<<";otherpos2b="<<otherSeekerPos2B<<endl;*/
    DEBUG_CLIENT(
        cout<<"TwoSeekerHBExplorer::selectRobotPos2 - goals: ";
        if (otherSeekerPos1==NULL)
            cout<<"none";
        else
            cout<<otherSeekerPos1->toString();
        cout <<" ";
        if (otherSeekerPos2==NULL)
            cout<<"none";
        else
            cout<<otherSeekerPos2->toString();
        cout << " beliefs: "<<otherSeekerPos1B<<" "<<otherSeekerPos2B<<endl;
    );

    if (_myCalcGoalPosVec.empty()) {
        throw CException(_HERE_, "the goal positions have not been calculated yet");
    }

    Pos returnPos;
    //bool hiderVisible = _lastHiderPos.isSet() && _lastHiderPosConsistent;

    //check if we want to update the next point: when either a timer passed, the hider is visible, or close to the previoulsy calculated position
    if ( _lastHiderPos.isSet() || //AG15022: if hider pos known, always update goal

        (_params->highBeliefFollowerUpdateGoalNumSteps==0 && _params->highBeliefFollowerUpdateGoalTime_ms==0)  || //AG150205: if no timer set
        (_timerID<0 && _params->highBeliefFollowerUpdateGoalTime_ms>0) ||      //no timer yet
            (_params->highBeliefFollowerUpdateGoalNumSteps>0 && _stepsToLastUpdate<=0)
            ||             //OR: a number of steps to update is set and it is passed
            (       _params->highBeliefFollowerUpdateGoalTime_ms>0
                    && _timer.getTime_ms(_timerID)>=_params->highBeliefFollowerUpdateGoalTime_ms
            )              //OR: there is update time set, and the time is passed
           // || (thisHiderPos.isSet() && thisHiderPosConsistent)
        //OR: the hider is visible to a seeker and is consistent (between teh seekers)
            || _lastSeekerPos.distanceEuc(_lastGoalPos)<=_params->followPersonDist  //winDist
                        /*|| (seeker2Pos!=NULL && _lastSeeker2GoalPos.isSet() && seeker2Pos.distanceEuc(_lastSeeker2GoalPos)<=_params->winDist )*/ )
        //OR: the last goal is close for this or the other seeker
        //AG150128 - NOTE: not sure if also need to search for new points if _lastSeeker2GoalPos is not set,
        //                 this would imply that he goal can change at every iteration
    {

        //check if poses are set
        //bool mySeeker2PosSet = _myCalcGoalPosVec.size()>=2;
        bool otherSeekerPos1Set = (otherSeekerPos1!=NULL && otherSeekerPos1->isSet());
        bool otherSeekerPos2Set = (otherSeekerPos2!=NULL && otherSeekerPos2->isSet());

        //only when both belief are available we can use them for comparison
        bool myHasBelief = _myCalcBeliefGoalPosVec.size()==2;
        assert(!myHasBelief || (_myCalcBeliefGoalPosVec[0]>=0 && _myCalcBeliefGoalPosVec[0]<=1 &&
                               _myCalcBeliefGoalPosVec[1]>=0 && _myCalcBeliefGoalPosVec[1]<=1) );
        //other has belief (only usefull when both have a belief)
        bool otherHasBelief = (otherSeekerPos1B>=0 && otherSeekerPos2B>=0);
        assert(!otherHasBelief || (otherSeekerPos1B<=1 && otherSeekerPos2B<=1) );


        //DEBUG_CLIENT(cout<<endl<<""<<endl<<"  ";)

        //TODO: here retrieve all poses (maybe we don't need to receive our own calculated data...)
        //check timer and if hider visible or not .. if timer not started or passed, or hider visible, OR ..
        //  then: calculate and return a new location; otherwise return prev. calculated location

        if (!otherSeekerPos1Set) {
            DEBUG_CLIENT(cout<<"has no pos for this seeker - ";);

            //without belief we cannot choose (or give pref to seeker which is closer..)
            returnPos = _myCalcGoalPosVec[0];
            DEBUG_CLIENT(cout<<"keeping own"<<endl;);

            /*if (!otherSeekerPos2Set || !mySeeker2PosSet) {
                returnPos = _myCalcGoalPosVec[0];
                DEBUG_CLIENT(cout<< (!otherSeekerPos2Set?"neither for other (not received?)":"only 1 seeker goal calculated")
                                     <<endl<<"Use the own calculated goal: "<<returnPos.toString()<<endl;);
            } else {
                DEBUG_CLIENT(cout<<"checking if goal close to seeker 2's - ";);
                if (_map->distance(_myCalcGoalPosVec[0],*otherSeekerPos2)<_params->multiSeekerSelectPosSmallDistBetwGoals) {
                    returnPos = _myCalcGoalPosVec[1];
                    DEBUG_CLIENT(cout<<"close, switching to goal 2: "<<returnPos.toString()<<endl;);
                } else {
                    returnPos = _myCalcGoalPosVec[1];
                    DEBUG_CLIENT(cout<<"not close, keeping goal 1: "<<returnPos.toString()<<endl;);
                }
            }*/
        } else {
            DEBUG_CLIENT(cout<<"has pos for this seeker - decide which to use - ";);
            if (!otherSeekerPos2Set) {
                throw CException(_HERE_, "the seeker's 2 pos should be passed and set, since the seeker 1's pos has been passed");
            }
            //now do check of distances of both's goals to make decision

            if (myHasBelief && otherHasBelief && _lastSeeker2Pos.isSet()) {
                //use belief
                DEBUG_CLIENT(cout<<"belief available"<<endl;);

                //AG150319: disabled '
                /*double d11_22 = _map->distance(_myCalcGoalPosVec[0],*otherSeekerPos2);

                if (d11_22<_params->multiSeekerSelectPosSmallDistBetwGoals
                        && abs(_myCalcBeliefGoalPosVec[0]-otherSeekerPos1B)>_params->multiSeekerSelectPosBeliefDiffBetwGoals
                        && abs(otherSeekerPos2B-_myCalcBeliefGoalPosVec[1])>_params->multiSeekerSelectPosBeliefDiffBetwGoals
                    ) {

                    returnPos = _myCalcGoalPosVec[0];
                    DEBUG_CLIENT(cout<<" pre-rule (small |d11-22|), goal is own"<<endl;);
                } else {*/
                    double d11_21 = _map->distance(_myCalcGoalPosVec[0],*otherSeekerPos1);
                    double d12_22 = _map->distance(_myCalcGoalPosVec[1],*otherSeekerPos2);

                    DEBUG_CLIENT(cout<<" small/large rule, belief: "<<(_params->multiSeekerSelectPosUseMaxBelief?"max":"sum")<<" - ";);

                    if (d11_21<_params->multiSeekerSelectPosSmallDistBetwGoals) {

                        if (d12_22<_params->multiSeekerSelectPosSmallDistBetwGoals) {

                            DEBUG_CLIENT(cout<<" small-small - ";);
                            //compare beliefs
                            if ((_params->multiSeekerSelectPosUseMaxBelief &&
                                    max(_myCalcBeliefGoalPosVec[0],_myCalcBeliefGoalPosVec[1]) >
                                        max(otherSeekerPos1B,otherSeekerPos2B)
                               ) ||
                               (!_params->multiSeekerSelectPosUseMaxBelief &&
                                    _myCalcBeliefGoalPosVec[0] +_myCalcBeliefGoalPosVec[1] >
                                        otherSeekerPos1B + otherSeekerPos2B
                               )
                            ) {
                                DEBUG_CLIENT(cout << "own belief higher"<<endl;);
                                returnPos = _myCalcGoalPosVec[0];
                            } else {
                                DEBUG_CLIENT(cout << "other belief higher"<<endl;);
                                returnPos = *otherSeekerPos1;
                            }

                        } else {
                            DEBUG_CLIENT(cout<<" small-large - ";);
                            if (otherSeekerPos2B>_myCalcBeliefGoalPosVec[1]) {
                                DEBUG_CLIENT(cout << "other belief higher"<<endl;);
                                returnPos = *otherSeekerPos1;
                            } else {
                                DEBUG_CLIENT(cout << "own belief higher"<<endl;);
                                returnPos = _myCalcGoalPosVec[0];
                            }
                        }
                    } else {

                        if (d12_22<_params->multiSeekerSelectPosSmallDistBetwGoals) {
                            DEBUG_CLIENT(cout<<" large-small - ";);
                            if (_myCalcBeliefGoalPosVec[0]>otherSeekerPos1B) {
                                DEBUG_CLIENT(cout << "own belief higher"<<endl;);
                                returnPos = _myCalcGoalPosVec[0];
                            } else {
                                DEBUG_CLIENT(cout << "other belief higher"<<endl;);
                                returnPos = *otherSeekerPos1;
                            }

                        } else {

                            DEBUG_CLIENT(cout<<" large-large - ";);
                            //compare beliefs
                            if ((_params->multiSeekerSelectPosUseMaxBelief &&
                                    max(_myCalcBeliefGoalPosVec[0],_myCalcBeliefGoalPosVec[1]) >
                                        max(otherSeekerPos1B,otherSeekerPos2B)
                               ) ||
                               (!_params->multiSeekerSelectPosUseMaxBelief &&
                                    _myCalcBeliefGoalPosVec[0] +_myCalcBeliefGoalPosVec[1] >
                                        otherSeekerPos1B + otherSeekerPos2B
                               )
                            ) {
                                DEBUG_CLIENT(cout << "own belief higher"<<endl;);
                                returnPos = _myCalcGoalPosVec[0];
                            } else {
                                DEBUG_CLIENT(cout << "other belief higher"<<endl;);
                                returnPos = *otherSeekerPos1;
                            }
                        }
                    }
                //}

            } else if (_lastSeeker2Pos.isSet() && _lastHiderPos.isSet()) {
                returnPos = _myCalcGoalPosVec[0];
                DEBUG_CLIENT(cout<<(myHasBelief&&otherHasBelief?"":"no ")
                             <<"belief available and seeker 2 pos set, and last hider pos set - checking prefered - ";);

                if (_map->distanceEuc(_lastSeekerPos,_lastHiderPos)<=_map->distanceEuc(_lastSeeker2Pos,_lastHiderPos)) {
                    DEBUG_CLIENT(cout<<" this seeker's goal closer - keeping own"<<endl;);
                    returnPos = _myCalcGoalPosVec[0];
                } else {
                    DEBUG_CLIENT(cout<<" other seeker's goal closer - setting other"<<endl;);
                    returnPos = *otherSeekerPos1;
                }

                //TODO check: this could cause changes in movements of seeker
            } else {
                returnPos = _myCalcGoalPosVec[0];
                DEBUG_CLIENT(cout<<(myHasBelief&&otherHasBelief?"":"no ")<<"belief available and seeker 2 pos"
                                    <<(_lastSeeker2Pos.isSet()?"":" not")<<" set, and last hider pos "
                                    <<(_lastHiderPos.isSet()?"":" not")<<" set - keeping own"<<endl;);
            }

        } //if-else otherSeekerPos1Set

        //start timer again
        if (_params->highBeliefFollowerUpdateGoalTime_ms>0) {
            if (_timerID<0) {
                _timerID = _timer.startTimer();
            } else {
                _timer.restartTimer(_timerID);
            }
        }
        _stepsToLastUpdate = _params->highBeliefFollowerUpdateGoalNumSteps;

        //last goal
        _lastGoalPos = returnPos;

    } else if (_lastGoalPos.isSet()) {     // timer not passed / or hider not seen
        DEBUG_CLIENT(cout<<"no update"<<endl;);
        //return last position
        returnPos = _lastGoalPos;
    } else {
        //SHOULD not occur
        cout <<"WARNING: time passed and no last pos found"<<endl;
        returnPos = _lastSeekerPos;
    }

    DEBUG_CLIENT(cout<<" --> "<<returnPos.toString()<<endl;);

    //AG150214: pos chosen (without steps)
    if (chosenPos!=NULL)
        *chosenPos = returnPos;

    if (/*hiderVisible ||*/ _params->onlySendStepGoals) {
        //get the next step towards goal, using 'navigation'
        //AG150220: stayFromGoal=false because this is already taken into account
        returnPos = getNextPosAsStep(_lastSeekerPos, returnPos, n, false /*hiderVisible*/);

        DEBUG_CLIENT(cout<<", in steps, next step: "<<returnPos.toString(););
    }
    DEBUG_CLIENT(cout<<endl;);


    _stepsToLastUpdate--;

    return returnPos;
}
#endif


std::string TwoSeekerHBExplorer::getName() const {
    stringstream ss;
    ss<<"TwoSeekerHBExplorer-of:"<<_autoPlayerWBelief->getName();
    return ss.str();
}

bool TwoSeekerHBExplorer::useGetAction() const {
    return false;
}

bool TwoSeekerHBExplorer::handles2Obs() const {
    return true;
}

bool TwoSeekerHBExplorer::getChosenHiderPos(Pos &chosenHiderPos) const {
    chosenHiderPos = _chosenHiderPos;
    return _chosenHiderPosConsist;
}
