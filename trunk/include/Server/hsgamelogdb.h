#ifndef HSGAMELOGDB_H
#define HSGAMELOGDB_H

#include "Server/hsserverconfig.h"
#include "Server/hsgamelog.h"

#include <QSqlDatabase>


class HSGameLogDB : public HSGameLog
{
public:

    HSGameLogDB(HSServerConfig* config);
    ~HSGameLogDB();


    virtual int startGame(QString mapName, GMap* gmap, SeekerHSParams* params, std::vector<PlayerInfoServer*> playerInfoVec);


    virtual long addGameStep(int actionNum, QDateTime sentTime, int status,
                                std::vector<PlayerInfoServer*> playerInfoVec,
                                PlayerInfoServer* hiderPlayerInfo);

    virtual int stopGame();

    virtual int close();

    virtual int getID() const;

    virtual bool isOpen() const;

protected:
    /*!
     * \brief queryAddBindValue add a string to the query and NULL if it is empty
     * \param query
     * \param str
     */
    void queryAddBindValueStr(QSqlQuery& query, std::string str);

    /*!
     * \brief queryAddBindValueStr queryAddBindValue add a string to the query and NULL if it is empty
     * \param query
     * \param str
     */
    void queryAddBindValueStr(QSqlQuery& query, QString str);


    static const QString DATE_TIME_MYSQL_FORMAT;

private:
    //! config file
    HSServerConfig* _config;

    //! database connection
    QSqlDatabase _db;

    //! game id of current game
    int _gameID;

    //! session id of opened session to db
    int _sessionID;

    //! map to do dist calculations
    GMap* _map;
};

#endif // HSGAMELOGDB_H
