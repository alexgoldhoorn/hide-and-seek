#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include "HSGame/bpos.h"
#include "Base/hsglobaldata.h"

#include <vector>


/*!
 * \brief The PlayerInfo struct contains all the meta information of the player.
 *
 * While running the following fields are updated:
 * - currentPos: [in] the position read as observation of the player;
 * - currentPosWNoise: [in] idem, but with noise (set by server);
 * - previousPos: [in] the previous (in the last time step) observed position of the player;
 * - nextPos: [out] the position as set as goal (output of the algorithm);
 * - hiderObsPosWNoise: [in] observed position of the hider (possibly with noise as set by server).
 * - lastAction: [out] last action done
 *
 * The algorithms update the following fields (depending on the algo.):
 * - multiGoalPosesVec and multiGoalBeliefVec: contain the goal poses and beliefs of all the seekers
 *      as calculated by this player. Used by find-and-follow 2 players version (IROS2015).
 * - multiHBPosVec and multiHBBeliefVec: contain the highest belief points. Used by find-and-follow
 *      multi players.
 *
 * Flags:
 * - initPosSet: init position has been set;
 * - multiHasGoalPoses: goal poses set;
 * - multiHasHBPoses: highest belief poses set;
 * - posRead: position has been read;
 * - nextPos.isSet: the next position is set, ready to send.
 *
 */
struct PlayerInfo {
    PlayerInfo();

    virtual ~PlayerInfo();

    //AG131210: clear all values (to -1)
    //! clear the variables
    virtual void clear();

    //! is a seeker, otherwise hider
    bool isSeeker() const;

    //!get user and player type name (for debug): [name(type)]
    std::string toString(bool showPos=false, bool showNoisePos=false, bool showHiderNoiseObsPos=false) const;

    //! compare, they are equal if the id is equal
    virtual bool operator ==(const PlayerInfo& other) const;

    //! compare, they are not equal if the id is not equal
    //! note: throws a CException if the id==-1
    virtual bool operator !=(const PlayerInfo& other) const;

    //! this function should be called before the current pos is read again,
    //! it sets the previousPos to currentPos and clears current and nextPos and other flags
    void prepareNextStep();

    /*!
     * \brief copyValuesFrom copy values from other player info, copies all data, and also meta data (includes name,
     * metaData, playerType, id, comments) if the flag copyMetaData is set
     * \param playerInfo
     * \param copyMetaData
     */
    void copyValuesFrom(const PlayerInfo& copyPI, bool copyMetaData);

    //! current position
    Pos currentPos;

    //AG121112:
    //! previous pos
    Pos previousPos;

    //AG150504
    //! current position with noise
    Pos currentPosWNoise;

    //! observation of hider, as seen by this player
    Pos hiderObsPosWNoise;

    //AG150611
    //! a probability of how much the hider's pos observation can be trusted
    //double hiderObsTrustProb; //AG150804: use useObsProb instead

    //AG150522
    //! the next position, which should be the next goal
    Pos nextPos;

    //!number of actions taken
    int numberActions;

    //! player type (seeker/hider)
    HSGlobalData::Player playerType;

    //ag150430
    //! init pos set
    bool initPosSet;

    //AG1500202
    //! indicates if the goal poses are received already (for multiple seekers)
    bool multiHasGoalPoses;

    //AG150711: added belief to Pos->BPos
    //! list of goal (with belief) for the seekers (for multiple seekers)
    //! first value refers to goal of this player
    std::vector<BPos> multiGoalBPosesVec;

    //! list of belief values for the seekers (for multiple seekers)
    //! same order as multiGoalPosesVec
    //std::vector<double> multiGoalBeliefVec;

    //! multi goal pos player info IDs (representing the player IDs of the same index of the vectors
    //! multiGoalPosesVec and multiGoalBeliefVec)
    std::vector<int> multiGoalIDVec;

    //! chosen goal pos by seeker, this can differ from nextPos, if HSSeekerParams::onlySendStepGoals is
    //! set, i.e. only steps can be sent and not farther goals
    Pos chosenGoalPos;

    //AG150609 TODO: use a struct, or the Pos to store Pos+Belief
    //AG150508
    //! list of highest belief points
    std::vector<BPos> multiHBPosVec;
    //! list of belief of hb's
    //std::vector<double> multiHBBeliefVec;
    //! indicates if the HB poses are received already (for multiple seekers)
    bool multiHasHBPoses;

    //AG150427: added id for multi
    //! id of player
    /*quint8*/ int id;

    //AG150430: moved from tcp class to here
    //! seeker belief score
    double seekerBeliefScore;

    //! seeker reward
    double seekerReward;

    //int action[MAXACTIONS]; //actions
    //AG131210
    //! change to last action
    int lastAction;

    //AG150519
    //! this is the probability of using the observation of this player,
    //! it indicates how trustworthy its readings are
    double useObsProb;

    //! indicates whether the positions have been read
    bool posRead;


#ifdef USE_QT
    //! set the username
    /*void setUserName(QString uname);

    //! get user name
    QString getUserName() const;*/

    //AG140531
    //! meta info
    QString metaInfo;
    //! comments user
    QString comments;

    //! username sent
    QString username;

/*protected:
    QString _username; //players name*/
#endif

};


#endif // PLAYERINFO_H
