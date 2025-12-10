#ifndef HSGAMELOG_H
#define HSGAMELOG_H

#include "../HSGame/gmap.h"
#include <QDateTime>

/*!
 *
 * \brief This (abstract) class stores the game results.
 */
class HSGameLog
{
public:
    //HSGameLog();

    /*!
     * \brief startGame Start game  details.
     * \param mapName
     * \param gmap
     * \param seeker
     * \param hider
     * \param maxActions
     * \return
     */
    virtual int startGame(QString mapName, GMap* gmap, QString seeker, QString hider, int maxActions)=0;

    /*!
     * \brief addGameStep Store each step of the game.
     * \param actionNum
     * \param hiderTime
     * \param seekerTime
     * \param hiderAction
     * \param seekerAction
     * \param hiderX
     * \param hiderY
     * \param seekerX
     * \param seekerY
     * \param status
     * \return
     */
    virtual int addGameStep(int actionNum, QDateTime hiderTime, QDateTime seekerTime, int hiderAction, int seekerAction,
                            int hiderX, int hiderY, int seekerX, int seekerY, int status)=0;

    /*!
     * \brief stopGame Logs that current game has stopped.
     * \return
     */
    virtual int stopGame()=0;


    virtual int close()=0;
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
