
#include "Base/seekerhslog.h"
#include "Base/seekerhs.h"

#include <cassert>

using namespace std;

SeekerHSLog::SeekerHSLog(SeekerHS *seekerHS) : _seekerHS(seekerHS), _gamelog(NULL) {
}

SeekerHSLog::~SeekerHSLog() {
}


void SeekerHSLog::open(string gamelogFile) {
    //check if log file already exists
    ifstream logFile;
    logFile.open(gamelogFile.c_str(), ifstream::in);
    bool exists = logFile.good();

    if (exists) {
        logFile.close();
    }

    //open log
    _gamelog = new HSLog(gamelogFile.c_str(),true);

    if (!exists) {
        //add header to log
        string header = "step_num,exp_name,seeker1_x,seeker1_y,hiderO1_x,hiderO1_y,seeker2_x,seeker2_y,hiderO2_x,hiderO2_y,"                //logline
                        "seeker1_row,seeker1_col,hiderO1_row,hiderO1_col,hiderO1_p,seeker2_row,seeker2_col,hiderO2_row,hiderO2_col,"
                        "hiderO2_p,hiderChosen_row,hiderChosen_col,dist_sh,distEuc_sh,dist_s2h,distEuc_s2h,dist_ss2,distEuc_ss2,win_state," //logline2
                        "seeker1_goal_x,seeker1_goal_y,seeker1_goal_or,seeker1_goal_row,seeker1_goal_col,seeker2_goal_row,seeker2_goal_col,"
                        "seeker1_numGoals_HBs,seeker2_numGoals_HBs,seeker_reward"; //,seeker_beliefScore";
        //AG150728: removed seekerBeliefScore, because this can only be calculated with a ground truth

        _gamelog->printLine(header);
    }
}


void SeekerHSLog::close() {
    if (_gamelog!=NULL)
        _gamelog->close();
}



void SeekerHSLog::stopGame(int winState) {
    logLine1Init();

    //seeker PosXY and obs hider
    _gamelog->print("_,_,_,_");
    //seeker2 PosXY and obs
    _gamelog->print("_,_,_,_");

    logLine1AfterPos(winState);
}

void SeekerHSLog::init(GMap *gmap, const SeekerHSParams *params, const PlayerInfo *thisPlayer, const PlayerInfo *otherPlayer,
                       const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, const PosXY *otherSeekerPosXY, const PosXY *otherHiderObsPosXY) {
//(const GMap *gmap, const SeekerHSParams *params, const PlayerInfo *thisPlayer, const PlayerInfo *otherPlayer) {

    assert(gmap!=NULL);
    assert(params!=NULL);
    assert(thisPlayer!=NULL);
    //other player can be NULL

    _map = gmap;
    _params = params;
    _thisSeekerPlayerInfo = thisPlayer;
    _otherSeekerPlayerInfo = otherPlayer;
    _lineNum = 0;

    logLine(seekerPosXY, hiderObsPosXY, otherSeekerPosXY, otherHiderObsPosXY, -1);
    logLine2(NULL);
}

void SeekerHSLog::logLine(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, const PosXY *otherSeekerPosXY, const PosXY *otherHiderObsPosXY,
                          int winState) {

    assert(seekerPosXY.isSet());    
    assert(_thisSeekerPlayerInfo!=NULL);
    assert(_thisSeekerPlayerInfo->currentPos.isSet());

    logLine1Init();

    //seeker and obs
    _gamelog->print(seekerPosXY, false, false);
    _gamelog->print(hiderObsPosXY, false, false);

    //seeker 2 and obs
    if (otherSeekerPosXY==NULL) {
        assert(otherHiderObsPosXY==NULL);
        _gamelog->print("_,_,_,_");
    } else {
        assert(otherHiderObsPosXY!=NULL);
        _gamelog->print(*otherSeekerPosXY, false, false);
        _gamelog->print(*otherHiderObsPosXY, false, false);
    }

    logLine1AfterPos(winState);
}

void SeekerHSLog::logLine1Init() {
    _gamelog->print(_lineNum);
    _gamelog->print(_params->expName);
}

void SeekerHSLog::logLine1AfterPos(int winState) {
    //seeker and obs (row,col)
    _gamelog->print(_thisSeekerPlayerInfo->currentPos);
    _gamelog->print(_thisSeekerPlayerInfo->hiderObsPosWNoise);
    _gamelog->print(_thisSeekerPlayerInfo->useObsProb);

    //hider and obs (row,col)
    if (_otherSeekerPlayerInfo==NULL || !_otherSeekerPlayerInfo->posRead) {
        _gamelog->print("_,_,_,_,_");
    } else {
        assert(_otherSeekerPlayerInfo->currentPos.isSet());
        _gamelog->print(_otherSeekerPlayerInfo->currentPos);
        _gamelog->print(_otherSeekerPlayerInfo->hiderObsPosWNoise);
        _gamelog->print(_otherSeekerPlayerInfo->useObsProb);
    }
    //todo id?

    //chosen hider pos
    Pos chosenHiderPos;
    bool ok = _seekerHS->getAutoPlayer()->getChosenHiderPos(chosenHiderPos);

    if (ok && chosenHiderPos.isSet()) {
        _gamelog->print(chosenHiderPos);
    } else {
        _gamelog->print("_,_");
    }

    //if (!chosenHiderPos.isSet()) chosenHiderPos = _thisSeekerPlayerInfo->hiderObsPosWNoise;

    //distance(s-h)
    if (chosenHiderPos.isSet()) {
        _gamelog->print(_map->distance(_thisSeekerPlayerInfo->currentPos, chosenHiderPos));
        _gamelog->print(_map->distanceEuc(_thisSeekerPlayerInfo->currentPos, chosenHiderPos));
    } else {
        _gamelog->print("_,_");
    }
    //distance(s2-h)
    if (_otherSeekerPlayerInfo!=NULL && _otherSeekerPlayerInfo->posRead && chosenHiderPos.isSet()) {
        _gamelog->print(_map->distance(_otherSeekerPlayerInfo->currentPos, chosenHiderPos));
        _gamelog->print(_map->distanceEuc(_otherSeekerPlayerInfo->currentPos, chosenHiderPos));
    } else {
        _gamelog->print("_,_");
    }
    //distance(s-s2)
    if (_otherSeekerPlayerInfo!=NULL && _otherSeekerPlayerInfo->posRead) {
        _gamelog->print(_map->distance(_otherSeekerPlayerInfo->currentPos, _thisSeekerPlayerInfo->currentPos));
        _gamelog->print(_map->distanceEuc(_otherSeekerPlayerInfo->currentPos, _thisSeekerPlayerInfo->currentPos));
    } else {
        _gamelog->print("_,_");
    }

    //game state
    _gamelog->print(winState);

    _lineNum++;
}


void SeekerHSLog::logLine2(const PosXY *chosenGoalPosXY) {
    assert(chosenGoalPosXY==NULL || chosenGoalPosXY->isSet());

    //chosen goal pos
    if (chosenGoalPosXY==NULL) {
        _gamelog->print("_,_,_");
    } else {
        _gamelog->print(*chosenGoalPosXY, true, false);
    }
    //chosen goal pos row,col
    if (_thisSeekerPlayerInfo->chosenGoalPos.isSet()) {
        _gamelog->print(_thisSeekerPlayerInfo->chosenGoalPos);
    } else {
        _gamelog->print("_,_");
    }

    //chosen goal of other
    if (_otherSeekerPlayerInfo!=NULL && _otherSeekerPlayerInfo->chosenGoalPos.isSet()) {
        _gamelog->print(_otherSeekerPlayerInfo->chosenGoalPos);
    } else {
        _gamelog->print("_,_");
    }

    //num hb / num goal pos
    if (_thisSeekerPlayerInfo->multiHasGoalPoses) {
        assert(!_thisSeekerPlayerInfo->multiHasHBPoses);
        _gamelog->print(_thisSeekerPlayerInfo->multiGoalBPosesVec.size());

        if (_otherSeekerPlayerInfo!=NULL && _otherSeekerPlayerInfo->multiHasGoalPoses) {
            _gamelog->print(_otherSeekerPlayerInfo->multiGoalBPosesVec.size());
        } else {
            _gamelog->print("_");
        }
    } else if (_thisSeekerPlayerInfo->multiHasHBPoses) {
        _gamelog->print(_thisSeekerPlayerInfo->multiHBPosVec.size());

        if (_otherSeekerPlayerInfo!=NULL || _otherSeekerPlayerInfo->multiHasHBPoses) {
            _gamelog->print(_thisSeekerPlayerInfo->multiHBPosVec.size());
        } else {
            _gamelog->print("_");
        }
    } else {
        _gamelog->print("_,_");
    }

    _gamelog->printLine(_thisSeekerPlayerInfo->seekerReward);
    //_gamelog->printLine(_thisSeekerPlayerInfo->seekerBeliefScore);
}
