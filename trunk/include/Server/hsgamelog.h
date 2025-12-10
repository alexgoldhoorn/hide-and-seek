#ifndef HSGAMELOG_H
#define HSGAMELOG_H

#include "HSGame/gmap.h"
#include <QDateTime>

#include "Base/playerinfoserver.h"

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
      * \brief startGame Initializes the log in the database with the map name and size, player meta info, and some essential parameters info.
      * It returns an ID which refers to this game log.
      * \param mapName
      * \param gmap
      * \param params
      * \param playerInfoVec
      * \return ID of the entry of this game log, or <0 if an error occurred.
      */
     virtual int startGame(QString mapName, GMap* gmap, SeekerHSParams* params, std::vector<PlayerInfoServer*> playerInfoVec) = 0;

     /*!
      * \brief addGameStep add a game step of all players.
      * \param actionNum
      * \param sentTime
      * \param status
      * \param playerInfoVec
      * \param hiderPlayerInfo
      * \return the id of the game log line entry, or <0 if an error occurred.
      */
     virtual long addGameStep(int actionNum, QDateTime sentTime, int status,
                                 std::vector<PlayerInfoServer*> playerInfoVec,
                                 PlayerInfoServer* hiderPlayerInfo) = 0;


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
    virtual int getID() const=0;

     /*!
     * \brief isOpen indicates of the log (db/file/..) is open.
     * \return
     */
    virtual bool isOpen() const=0;

protected:
     //bool _useContinuousPos;
     SeekerHSParams* _params;

};

#endif // HSGAMELOG_H

