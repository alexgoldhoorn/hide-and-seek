#include "Base/playerinfo.h"

#include "exceptions.h"

#include <cassert>
#include <sstream>

using namespace std;

PlayerInfo::PlayerInfo() {
    clear();
}

PlayerInfo::~PlayerInfo(){
}

void PlayerInfo::clear() {
    previousPos.clear();
    currentPos.clear();
    nextPos.clear();
    hiderObsPosWNoise.clear();
    chosenGoalPos.clear();

    id = -1;
    numberActions = 0;
    lastAction = -1;
    playerType = HSGlobalData::P_None;
    initPosSet = false;
    multiHasGoalPoses = false;
    multiHasHBPoses = false;
    seekerBeliefScore = -1;
    seekerReward = 0;
    useObsProb = 1;
    posRead = false;
    //hiderObsTrustProb = -1;
    seekerID = -1;
#ifdef USE_QT
    metaInfo.clear();
    comments.clear();
    username.clear();
#endif
}


bool PlayerInfo::isSeeker() const {
    return playerType==HSGlobalData::P_Seeker; // || playerType==HSGlobalData::P_Seeker2;
}

std::string PlayerInfo::toString(bool showPos, bool showNoisePos, bool showHiderNoiseObsPos) const {
    //AG150716: use stringstream
    stringstream sstr;

    sstr << "{"<<id<<"["<<(this)<<"]";

#ifdef USE_QT
    sstr << "|"<<username.toStdString();
    sstr << "(";
    if ((int)playerType<0) {
        sstr << "?";
    } else {
        assert(playerType>=0 && playerType<HSGlobalData::PLAYER_TYPE_NAMES.size());
        sstr << HSGlobalData::PLAYER_TYPE_NAMES[playerType].toStdString();
    }
    sstr << ")";
#endif

    if (!posRead) sstr<<"!";
    if (showPos) sstr<< "Pos=" << currentPos.toString() << ";";
    if (showNoisePos) sstr <<"NPos=" << currentPosWNoise.toString()<<";";
    if (showHiderNoiseObsPos) {
        sstr << "HObs=" << hiderObsPosWNoise.toString();
        /*if (hiderObsTrustProb>=0)
            sstr << ",p="<< hiderObsTrustProb;*/
        if (useObsProb>=0)  //AG150804: use this prob
            sstr << ",p="<<useObsProb;
    }
    sstr << "}";
    return sstr.str();
/*
    QString s = "{"+QString::number(id)+"|"+_username+"(";
    if ((int)playerType<0) {
        s += "?";
    } else {
        assert(playerType>=0 && playerType<HSGlobalData::PLAYER_TYPE_NAMES.size());
        s += HSGlobalData::PLAYER_TYPE_NAMES[playerType];
    }
    s += ")";
    if (!posRead) s+="!";
    if (showPos) s = s + "Pos=" + QString::fromStdString(currentPos.toString())+";";
    if (showNoisePos) s = s + "NPos=" + QString::fromStdString(currentPosWNoise.toString())+";";
    if (showHiderNoiseObsPos) {
        s = s + "HObs=" + QString::fromStdString(hiderObsPosWNoise.toString());
        if (hiderObsTrustProb>=0)
            s+=",p="+QString::number(hiderObsTrustProb);
    }
    s += "}";
    return s.toStdString();*/
}


bool PlayerInfo::operator ==(const PlayerInfo& other) const {
    if (id==-1) throw CException(_HERE_, "the id of this player is not set");
    if (other.id==-1) throw CException(_HERE_, "the id of the other player is not set");
    return other.id==id;
}

bool PlayerInfo::operator !=(const PlayerInfo& other) const {
    if (id==-1) throw CException(_HERE_, "the id of this player is not set");
    if (other.id==-1) throw CException(_HERE_, "the id of the other player is not set");
    return other.id!=id;
}

void PlayerInfo::prepareNextStep() {
    posRead = multiHasGoalPoses = multiHasHBPoses = false;
    previousPos = currentPos;
    currentPos.clear();
    currentPosWNoise.clear();
    nextPos.clear();
    chosenGoalPos.clear();
    hiderObsPosWNoise.clear();
    seekerReward = 0;
    seekerBeliefScore = -1;
    dynObsVisibleVec.clear();
}

void PlayerInfo::copyValuesFrom(const PlayerInfo &copyPI, bool copyMetaData) {
    if (copyMetaData) {
        playerType      = copyPI.playerType;
        //hiderObsTrustProb = copyPI.hiderObsTrustProb;
        id              = copyPI.id;
#ifdef USE_QT
        //setUserName(copyPI.getUserName());
        username        = copyPI.username;
        comments        = copyPI.comments;
        metaInfo        = copyPI.metaInfo;
#endif
    }

    currentPos              = copyPI.currentPos;
    previousPos             = copyPI.previousPos;
    currentPosWNoise        = copyPI.currentPosWNoise;
    hiderObsPosWNoise       = copyPI.hiderObsPosWNoise;
    nextPos                 = copyPI.nextPos;
    numberActions           = copyPI.numberActions;
    initPosSet              = copyPI.initPosSet;
    multiHasGoalPoses       = copyPI.multiHasGoalPoses;
    multiGoalBPosesVec      = copyPI.multiGoalBPosesVec;
    //multiGoalBeliefVec      = copyPI.multiGoalBeliefVec;
    chosenGoalPos           = copyPI.chosenGoalPos;
    multiHBPosVec           = copyPI.multiHBPosVec;
    //multiHBBeliefVec        = copyPI.multiHBBeliefVec;
    multiHasHBPoses         = copyPI.multiHasHBPoses;
    seekerBeliefScore       = copyPI.seekerBeliefScore;
    seekerReward            = copyPI.seekerReward;
    lastAction              = copyPI.lastAction;
    useObsProb              = copyPI.useObsProb;
    posRead                 = copyPI.posRead;
    dynObsVisibleVec        = copyPI.dynObsVisibleVec;
}
