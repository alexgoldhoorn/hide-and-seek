#include "Smart/multiseekerhbexplorer.h"

#include <iostream>
#include <limits>

#include "exceptions.h"

using namespace std;

MultiSeekerHBExplorer::MultiSeekerHBExplorer(SeekerHSParams* params, AutoPlayer* autoPlayerWBelief) :
    HighestBeliefFollower(params,autoPlayerWBelief)
{
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
        throw CException(_HERE_, "The auto player used by MultiSeekerHBExplorer should be able to handle 2 observations.");
    }
}

MultiSeekerHBExplorer::~MultiSeekerHBExplorer()
{
}

bool MultiSeekerHBExplorer::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    /*throw CException(_HERE_,"MultiSeekerHBExplorer::initBelief: use MultiSeekerHBExplorer::initBelief2");
    return false;*/

    bool initB = _autoPlayerWBelief->initBelief(gmap, seekerInitPos, hiderInitPos, opponentVisible);
    return initB;
}

bool MultiSeekerHBExplorer::initBelief2(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible, Pos seeker2InitPos,
                                        Pos hiderObs2InitPos, double obs1p) {
    _map = gmap;
    DEBUG_CLIENT(cout << "MultiSeekerHBExplorer::initBelief2: "<<endl;);
    _stepsToLastUpdate = 0;
    bool initB = _autoPlayerWBelief->initBelief2(gmap, seekerInitPos, hiderInitPos, opponentVisible, seeker2InitPos, hiderObs2InitPos, obs1p);

    return initB;
}

bool MultiSeekerHBExplorer::getNextRobotPoses2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, std::vector<int> &actions,
                                                      std::vector<Pos> &goalPosesVec, int actionDone, int n, std::vector<double> *goalPosesBelVec) {

    assert(seekerPos.isSet());
    DEBUG_CLIENT(
        cout<<"MultiSeekerHBExplorer::getNextRobotPoses2: [s1:"<<seekerPos.toString()<<",h:"<<hiderPos.toString()<<";s2:"<<flush;
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

    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::getNextRobotPoses2: now select next steps - ";);

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
                cout << "MultiSeekerHBExplorer::getNextRobotPoses2: WARNING: the same goal position has been chosen for both seekers (i="<<s1i
                     <<",#="<<highestBeliefPosVec.size()<<")"<<endl;
            }


            //both seekers, choosing the one where the total distance is less (seeker1 to pos1 and s2 to p2, or vica versa)
            DEBUG_CLIENT(cout <<"MultiSeekerHBExplorer::getNextRobotPoses2: checking which goal is closer: ";);
            if (        _map->distance(seekerPos,_myCalcGoalPosVec[0]) + _map->distance(*seeker2Pos,_myCalcGoalPosVec[1])
                    <=
                        _map->distance(seekerPos,_myCalcGoalPosVec[1]) + _map->distance(*seeker2Pos,_myCalcGoalPosVec[0])
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
                    cout << "MultiSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 1 not set, setting to hiderpos"<<endl;
                    nextPos1 = thisHiderPos;
                }
                if (!nextPos2.isSet()) {
                    cout << "MultiSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 2 not set, setting to hiderpos"<<endl;
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
            cout << "MultiSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 1 not set, setting to hiderpos"<<endl;
            nextPos1 = thisHiderPos;
        }
        /*if (!nextPos2.isSet()) {
            cout << "MultiSeekerHBExplorer::getNextRobotPoses2: WARNING next pos 2 not set, setting to hiderpos"<<endl;
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
    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::getNextRobotPoses2: be sure they are not on the same pos - ";);
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
        cout<<"MultiSeekerHBExplorer::getNextRobotPoses2: sending to other seeker: poses: ";
        for (const Pos& p : _myCalcGoalPosVec )
            cout << p.toString()<<" ";
        cout << "; beliefs: ";
        for (double d : _myCalcBeliefGoalPosVec)
            cout <<d <<" ";
        cout<<endl;
    );

    return true;
}

int MultiSeekerHBExplorer::calcExplorationEvaluation(const Pos &seekerPos, const std::vector<Pos> &highestBeliefPosVec,
                                                        const std::vector<double> &highestBeliefVec, Pos *otherRobotGoal) {


    assert(highestBeliefPosVec.size()==highestBeliefVec.size());
    DEBUG_CLIENT(cout<<"MultiSeekerHBExplorer::calcExplorationEvaluation: for seeker "<<seekerPos.toString()<<", other="<<
                 (otherRobotGoal==NULL?"-":otherRobotGoal->toString())<<":"<<endl;);

    assert(seekerPos.isSet());
    assert(highestBeliefPosVec.size()>0);
    assert(highestBeliefPosVec[0].isSet());

    //AG150213:
    assert(highestBeliefPosVec.size() == highestBeliefVec.size());

    int maxValueI = -1;

    if (highestBeliefPosVec.size()==1) {
        //AG150213: only 1 item, so this is the highest/only option
        DEBUG_CLIENT(cout<<"  --> only 1, so I="<<maxValueI<<endl;);
        maxValueI = 0;

    } else {
        //max value of the equation
        double maxValue = numeric_limits<double>::lowest();

        //get maximum belief and distance for normalization
        double maxDist = _map->distance(highestBeliefPosVec[0], seekerPos);
        double maxBelief = highestBeliefVec[0];
        for(size_t i = 1; i < highestBeliefPosVec.size(); i++) {
            if (highestBeliefPosVec[i].isSet()) {
                //check highest distance
                double d = _map->distance(highestBeliefPosVec[i], seekerPos);
                if (d > maxDist)
                    maxDist = d;
                //check highest belief
                if (highestBeliefVec[i] > maxDist)
                    maxDist = highestBeliefVec[i];
            }
        }

        if (maxDist==0) {
            //AG150213: max dist = 0, so this is the best one
            DEBUG_CLIENT(cout<<"  maxDist=0, so resetting to 1 (for calculations)"<<endl;);
            maxDist = 1;
        }

        //check all highest belief points
        for(size_t i=0; i < highestBeliefPosVec.size(); i++) {
            if (highestBeliefPosVec[i].isSet()) {  //highestBeliefVec[i]>0) { //only when there is a belief (otherwise it's not a point to use)
                //utility
                double u = 1;
                if (otherRobotGoal!=NULL) {
                    double d = _map->distance(highestBeliefPosVec[i], *otherRobotGoal);
                    if (d<_params->multiSeekerExplorerMaxRange) {
                        u -= 1 - d/_params->multiSeekerExplorerMaxRange;
                    }
                }
                //distance high bel. point - seeker
                double d = _map->distance(highestBeliefPosVec[i], seekerPos);
                //calculate the value
                double y = _params->multiSeekerExplorerUtilWeight * u - _params->multiSeekerExplorerDistWeight * d / maxDist +
                            _params->multiSeekerExplorerBeliefWeight * highestBeliefVec[i] / maxBelief;

                if (y>maxValue) {
                    maxValue = y;
                    maxValueI = i;
                }

                DEBUG_CLIENT(cout<<"  "<<i<<") pos="<<highestBeliefPosVec[i].toString()<<",u="<<u<<",d="<<d<<",b="<<highestBeliefVec[i]<<", y="<<y<<endl;)
            }
        }

        DEBUG_CLIENT(cout<<"  --> max I="<<maxValueI<<" ("<<maxValue<<")"<<endl;);
    }

    return maxValueI;
}



Pos MultiSeekerHBExplorer::calcOtherRobotPos(const Pos& s1p, const Pos& hpos, const Pos& s2p) {
    assert(s1p.isSet());
    assert(hpos.isSet());
    assert(s2p.isSet());

    static const double M_2PI=2*M_PI;

    //minimum angle between seekers (with min dist to eachother and to hider)
    double a = 2 * asin( _params->minDistBetweenRobots/(2*_params->followPersonDist));
    //direction of already located seeker as seen from hider
    double b1 = _map->getDirection(hpos,s1p);
    //the other seeker can at a min distance to hider if the angle is between a1 and a2
    double a1 = (b1+a);
    if (a1>M_2PI) a1 -= M_2PI;
    double a2 = (b1-a);
    if (a2<0) a2 += M_2PI;
    //current direction hider-other seeker
    double b2 = _map->getDirection(hpos,s2p);

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

    Pos s2p2 = _map->tryMoveDir(c,hpos,_params->followPersonDist,false);

    if (!s2p2.isSet()) {
        //should occur only when hider close to obstacle
        cout << "MultiSeekerHBExplorer::calcOtherRobotPos: WARNING: new pos not consistent/correct, using hider's pos"<<endl;
        s2p2 = hpos;
    }

    /*cout << "S1:"<<s1p.toString()<<" H:"<<hpos.toString()<<" S2:"<<s2p.toString()<<endl
         << "a="<<rad2deg(a)<<"º b1="<<rad2deg(b1)<<"º a1="<<rad2deg(a1)<<"º a2="<<rad2deg(a2)<<"º"<<endl
         << "b2="<<rad2deg(b2)<<"º --> c="<<rad2deg(c)<<"º - "<< s2p2.toString()<<endl<<endl;*/

    return s2p2;
}

Pos MultiSeekerHBExplorer::selectRobotPos2(Pos *otherSeekerPos1, Pos *otherSeekerPos2, double otherSeekerPos1B, double otherSeekerPos2B,
                                           int n, Pos* chosenPos) {

/*cout << "selectrobotpos2: otherpos1="<<(otherSeekerPos1==NULL?"null":otherSeekerPos1->toString())
     << ";otherpos2="<<(otherSeekerPos2==NULL?"null":otherSeekerPos2->toString())
     << ";otherpos1b="<<otherSeekerPos1B<<";otherpos2b="<<otherSeekerPos2B<<endl;*/
    DEBUG_CLIENT(
        cout<<"MultiSeekerHBExplorer::selectRobotPos2 - goals: ";
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

Pos MultiSeekerHBExplorer::getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos,
                                          Pos hiderObs2Pos, double obs1p, std::vector<int> &actions, int actionDone, int n) {

    throw CException(_HERE_,"MultiSeekerHBExplorer::getNextPosRun2: not implemented, use getNexRobotPoses2");
}

Pos MultiSeekerHBExplorer::getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    throw CException(_HERE_,"MultiSeekerHBExplorer::getNextPosRun: not implemented, use getNexRobotPoses2");
}

std::string MultiSeekerHBExplorer::getName() const {
    stringstream ss;
    ss<<"MultiSeekerHBExplorer-of:"<<_autoPlayerWBelief->getName();
    return ss.str();
}

bool MultiSeekerHBExplorer::useGetAction() const {
    return false;
}

bool MultiSeekerHBExplorer::handles2Obs() const {
    return true;
}


