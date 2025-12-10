#ifndef HSGAMELOGDB_H
#define HSGAMELOGDB_H

#include "hsserverconfig.h"
#include "hsgamelog.h"

#include <QSqlDatabase>


class HSGameLogDB : public HSGameLog
{
public:
    HSGameLogDB(HSServerConfig* config);
    ~HSGameLogDB();

    virtual int startGame(QString mapName, GMap* gmap, QString seeker, QString hider, int maxActions);

    virtual int addGameStep(int actionNum, QDateTime hiderTime, QDateTime seekerTime, int hiderAction, int seekerAction,
                            int hiderX, int hiderY, int seekerX, int seekerY, int status);

    virtual int stopGame();

    virtual int close();

private:
    HSServerConfig* _config;
    QSqlDatabase _db;
    int _gameID;
    int _sessionID;
};

#endif // HSGAMELOGDB_H
