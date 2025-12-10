#ifndef HSGAMELOG_H
#define HSGAMELOG_H

#include "HSGame/gmap.h"
#include <QDateTime>


struct PlayerInfo;

/*!
 *
 * \brief This (abstract) class stores the game results and meta info.
 */
class HSGameLog
{
public:
     HSGameLog();

     ~HSGameLog();

     /*!
     * \brief startGame Start the game: enters the meta info into the log/db.
     * \param mapName
     * \param gmap
     * \param seekerInfo
     * \param hiderInfo
     * \param seeker2Info NULL if not used
     * \param params
     * \return
     */
    virtual int startGame(QString mapName, GMap* gmap, PlayerInfo* seekerInfo, PlayerInfo* hiderInfo, PlayerInfo* seeker2Info, SeekerHSParams* params)=0;

     /*!
      * \brief addGameStep Store each step of the game.
      * \param actionNum action num (0=halt, 1=nort,..)
      * \param sentTime
      * \param hiderTime
      * \param seekerTime
      * \param seeker2Time
      * \param hiderAction
      * \param seekerAction
      * \param seeker2Action
      * \param hider
      * \param seeker
      * \param seeker2
      * \param status
      * \param dsh
      * \param dsb
      * \param dhb
      * \param dshEuc
      * \param ds2h
      * \param ds2hEuc
      * \param vis_sh
      * \param vis_s2h
      * \param vis_ss2
      * \param hiderPosWNoiseSent
      * \param hiderPosWNoiseSent2
      * \param seekerBeliefScore
      * \param seeker2BeliefScore
      * \param seekerReward
      * \param seeker2Reward
      * \return
      */
     virtual int addGameStep(int actionNum, QDateTime sentTime,
                             QDateTime hiderTime, QDateTime seekerTime, QDateTime seeker2Time,
                             int hiderAction, int seekerAction, int seeker2Action,
                             const Pos& hider, const Pos& seeker, const Pos* seeker2,
                             int status, double dsh, double dsb, double dhb,
                             double dshEuc, double ds2h, double ds2hEuc, double dss2, double dss2Euc,
                             bool vis_sh, bool vis_s2h, bool vis_ss2,
                             const Pos* hiderPosWNoiseSent, const Pos* hiderPosWNoiseSent2,
                             double seekerBeliefScore, double seeker2BeliefScore, double seekerReward, double seeker2Reward,
                             const std::vector<Pos> goalVectorFromS1, const std::vector<double> goalBeliefVectorFromS1, const Pos& multiChosenPosS1,
                             const std::vector<Pos> goalVectorFromS2, const std::vector<double> goalBeliefVectorFromS2, const Pos& multiChosenPosS2
                             /*const Pos* goalS1FromS1, const Pos* goalS2FromS1, double goalS1FromS1Bel, double goalS2FromS1Bel,
                             const Pos* goalS1FromS2, const Pos* goalS2FromS2, double goalS1FromS2Bel, double goalS2FromS2Bel*/)=0;


    /*!
     * \brief stopGame Logs that current game has stopped.
     * \return
     */
    virtual int stopGame()=0;

     /*!
     * \brief close close the log
     * \return
     */
    virtual int close()=0;

     /*!
     * \brief getID get session ID (when using DB)
     * \return
     */
    virtual int getID()=0;


     //void setUseContinuousPos(bool b);

protected:
     //bool _useContinuousPos;
     SeekerHSParams* _params;

};

#endif // HSGAMELOG_H

/*

DROP FUNCTION IF EXISTS AddGame;
CREATE DEFINER=`root`@`localhost` FUNCTION `AddGame`(
    MapName VARCHAR(50),
    MapWidth INT,
    MapHeight INT,
    MapNumObst INT,
    MaxActions INT,
    SeekerUserID INT,
    HiderUserID INT
)
CREATE DEFINER=`root`@`localhost` FUNCTION `AddGameLine`(
    GameID INT,
    ActionNum INT,
    HiderTimeStamp DATETIME,
    SeekerTimeStamp DATETIME,
    HiderAction SMALLINT,
    SeekerAction SMALLINT,
    HiderX INT,
    HiderY INT,
    SeekerX INT,
    SeekerY INT,
    status SMALLINT
)
*/
