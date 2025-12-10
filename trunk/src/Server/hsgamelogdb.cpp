#include "Server/hsgamelogdb.h"
#include "HSGame/gmap.h"

#include <iostream>

#include <QSqlDriver>
#include <QSqlQuery>
#include <QVariant>
#include <QString>
#include <QSqlError>

#include "Server/hstcpserver.h"
#include "Base/hsglobaldata.h"

using namespace std;

const QString HSGameLogDB::DATE_TIME_MYSQL_FORMAT = "yyyy-MM-dd hh:mm:ss.zzz";

HSGameLogDB::HSGameLogDB(HSServerConfig *config) : HSGameLog()
{
    _config = config;

    //get DB info from config
    QString dbServer,dbDB,dbUser,dbPass;
    _config->getLogDB(dbServer,dbDB,dbUser,dbPass);

    //connect to database
    _db = QSqlDatabase::addDatabase("QMYSQL");
    _db.setHostName(dbServer);
    _db.setDatabaseName(dbDB);
    _db.setUserName(dbUser);
    _db.setPassword(dbPass);

    _sessionID = -1;
    _gameID = -1;

    bool ok = _db.open();
    if (ok) {
        //now get session id
        QSqlQuery query(_db);
        ok = query.exec("SELECT StartServer();");
        if (ok && query.next()) {
            _sessionID = query.value(0).toInt();
        }

        if (_sessionID>=0) {
            //cout << " with session id: "<<_sessionID<<endl;
        } else {
            cout << "Database could not open a session!"<<endl;
        }
    } else {
        cout << "Could not open the database."<<endl;
    }
}

HSGameLogDB::~HSGameLogDB() {
    if (!_db.isOpen()) {
        cout << "WARNING: log db is not open"<<endl;
    } else {
        //close session
        QSqlQuery query(_db);
        query.prepare("SELECT StopServer(?);");
        query.addBindValue(_sessionID);
        if (!query.exec()) {
            cout << "could not register stopping of server"<<endl;
        }
        _db.close();
    }
}

void HSGameLogDB::queryAddBindValueStr(QSqlQuery &query, QString str) {
    if (str.isEmpty()) {
        query.addBindValue(QVariant(QVariant::String));
    } else {
        query.addBindValue(str);
    }
}

void HSGameLogDB::queryAddBindValueStr(QSqlQuery &query, string str) {
    queryAddBindValueStr(query, QString::fromStdString(str));
}

int HSGameLogDB::startGame(QString mapName, GMap* gmap, SeekerHSParams* params, std::vector<PlayerInfoServer*> playerInfoVec) {

    _params = params;
    _map = gmap;

    assert(_params!=NULL);
    assert(_map!=NULL);
    assert(playerInfoVec.size()==_params->numPlayersReq);

    if (!_db.isOpen()) {
        cout << "HSGameLogDB::startGame: WARNING: log db is not open"<<endl;
        return -1;
    }

    //generate query
    QSqlQuery query(_db);

    //AG150713: first add game, then game users
    bool ok = query.prepare("SELECT AddGame(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
    if (!ok) {
        cout << "HSGameLogDB::startGame: could not prepare query to add game. Last err: "<<query.lastError().text().toStdString()<<endl;
        return -3;
    }

    query.addBindValue(mapName);
    query.addBindValue(gmap->colCount());
    query.addBindValue(gmap->rowCount());
    query.addBindValue(gmap->numObstacles());
    if (gmap->getBase().isSet()) {
        query.addBindValue(gmap->getBase().row());
        query.addBindValue(gmap->getBase().col());
    } else {
        query.addBindValue(QVariant(QVariant::Int));
        query.addBindValue(QVariant(QVariant::Int));
    }

    //AG160414: experiment name
    query.addBindValue(QString::fromStdString(_params->expName));

    query.addBindValue(_params->opponentType);
    queryAddBindValueStr(query, _params->oppActionFile);
    query.addBindValue(_params->autoWalkerType);
    query.addBindValue(_params->autoWalkerN);
    queryAddBindValueStr(query, _params->autoWalkerPosFile);

    //params: use continuous, game type, maxac
    query.addBindValue(_params->gameType);
    query.addBindValue(_params->useContinuousPos);
    query.addBindValue(_params->solverType);
    query.addBindValue(_params->stopAfterNumSteps);
    query.addBindValue(_params->stopAtWin);
    query.addBindValue((qint32)_params->maxNumActions);
    query.addBindValue(_params->winDist);
    query.addBindValue(_params->allowInconsistObs);
    query.addBindValue(_params->simObsNoiseStd);
    query.addBindValue(_params->simObsFalseNegProb);
    query.addBindValue(_params->simObsFalsePosProb);
    query.addBindValue(_params->seekerStepDistance);
    query.addBindValue(_params->hiderStepDistance);

    //execute query
    ok = query.exec();

    if (!ok) {
        cout << "HSGameLogDB::startGame: Error while adding game to database."<<endl;
        return -2;
    }

    if (query.next()) {
        _gameID =  query.value(0).toInt();
        //cout << "Game ID: "<<_gameID<<endl;
        if (_gameID<0) {
            cout << "HSGameLogDB::startGame: Error while adding game to database, couldn't read the game ID."<<endl;
            return -1;
        }
    } else {
        _gameID = -1;
        cout << "HSGameLogDB::startGame: Could not add game to database!"<<endl;
        return -1;
    }

    //players info
    for (PlayerInfoServer* pInfo : playerInfoVec) {
        assert(pInfo!=NULL);

        QSqlQuery queryP(_db);

        queryP.prepare("SELECT AddGameUserByName(?,?,?,?,?);");
        queryP.addBindValue(_gameID);
        queryP.addBindValue(pInfo->username);
        queryP.addBindValue(pInfo->isSeeker());
        queryP.addBindValue(pInfo->metaInfo);
        queryP.addBindValue(pInfo->comments);

        ok = queryP.exec();

        if (!ok) {
            cout << "HSGameLogDB::startGame: Error while adding player "<<pInfo->toString()<<" to the DB"<<endl;
            return -2;
        }

        if (queryP.next()) {
            pInfo->gameUserDBID = queryP.value(0).toInt(&ok);
            cout << "GameUser ID for "<<pInfo->toString()<<": "<<pInfo->gameUserDBID<<endl;
            if (!ok || pInfo->gameUserDBID<0) {
                cout << "HSGameLogDB::startGame: Error while adding player "<<pInfo->toString()<<" to game "<<
                        _gameID << " to the database!"<<endl;
                return -1;
            }
        } else {
            pInfo->gameUserDBID = -1;
            cout << "HSGameLogDB::startGame: Could not add player "<<pInfo->toString()<<" to game "<<
                    _gameID << " to the database!"<<endl;
        }
    }

    return _gameID;
}

long HSGameLogDB::addGameStep(int actionNum, QDateTime sentTime, int status, std::vector<PlayerInfoServer *> playerInfoVec,
                             PlayerInfoServer *hiderPlayerInfo) {

    assert(_params!=NULL);
    assert(_map!=NULL);
    assert(hiderPlayerInfo!=NULL);
    assert(hiderPlayerInfo->currentPos.isSet());
    assert(playerInfoVec.size()>=2);

    if (!_db.isOpen()) {
        cout << "HSGameLogDB::addGameStep: WARNING: log db is not open"<<endl;
        return -1;
    }

    qlonglong gameLineID = -1;
    Pos basePos = _map->getBase();

    QSqlQuery query(_db);

    //add game line
    query.prepare("SELECT AddGameLine(?,?,?,?);");

    query.addBindValue(_gameID);
    query.addBindValue(actionNum);
    query.addBindValue(sentTime.toString(DATE_TIME_MYSQL_FORMAT));
    query.addBindValue(status);

    bool ok = query.exec();

    if (!ok) {
        cout << "HSGameLogDB::addGameStep: Error: could not execute query. Last error: "<<query.lastError().text().toStdString()<<endl;
        return -2;
    }

    if (query.next()) {
        gameLineID = query.value(0).toLongLong(&ok);

        if (!ok || gameLineID<0) {
            cout << "HSGameLogDB::addGameStep: failed to add game log line"<<endl;
            return -1;
        }
    } else {
        cout << "HSGameLogDB::addGameStep: failed to add game log line (no return value)"<<endl;
        return -1;
    }

    //now log player locations
    for(PlayerInfoServer* pInfo : playerInfoVec) {
        assert(pInfo!=NULL);
        assert(pInfo->currentPos.isSet());
        assert(pInfo->posRead);

        QSqlQuery queryP(_db);

        ok = queryP.prepare("SELECT AddGameUserLine(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
        if (!ok) {
            cout << "HSGameLogDB::addGameStep: failed to prepare query to add user "<<pInfo->toString()<<" to game log line."<<endl;
            return -2;
        }

        assert(pInfo->gameUserDBID>=0);

        queryP.addBindValue(pInfo->gameUserDBID);
        queryP.addBindValue(gameLineID);

        queryP.addBindValue(pInfo->timestamp.toString(DATE_TIME_MYSQL_FORMAT));

        if (pInfo->lastAction==-1) {
            queryP.addBindValue(QVariant(QVariant::Int));
        } else {
            queryP.addBindValue(pInfo->lastAction);
        }
        assert(pInfo->currentPos.isSet());
        queryP.addBindValue(pInfo->currentPos.row());
        queryP.addBindValue(pInfo->currentPos.col());
        queryP.addBindValue(pInfo->currentPos.rowDouble());
        queryP.addBindValue(pInfo->currentPos.colDouble());

        if (pInfo->isSeeker()) {
            queryP.addBindValue(pInfo->hiderObsPosWNoise.rowDouble());
            queryP.addBindValue(pInfo->hiderObsPosWNoise.colDouble());

            //calculate distances
            double d_sh = _map->distance(pInfo->currentPos, hiderPlayerInfo->currentPos);
            double d_shEuc = _map->distanceEuc(pInfo->currentPos, hiderPlayerInfo->currentPos);
            queryP.addBindValue(d_sh);
            queryP.addBindValue(d_shEuc);
        } else {
            //hider obs pos
            queryP.addBindValue(QVariant(QVariant::Double));
            queryP.addBindValue(QVariant(QVariant::Double));
            //d_sh, euc
            queryP.addBindValue(QVariant(QVariant::Double));
            queryP.addBindValue(QVariant(QVariant::Double));
        }

        if (basePos.isSet()) {
            double d_pb = _map->distance(pInfo->currentPos, basePos);
            double d_pbEuc = _map->distanceEuc(pInfo->currentPos, basePos);
            queryP.addBindValue(d_pb);
            queryP.addBindValue(d_pbEuc);
        } else {
            queryP.addBindValue(QVariant(QVariant::Double));
            queryP.addBindValue(QVariant(QVariant::Double));
        }

        if (pInfo->isSeeker()) {
            bool vis_sh = _map->isVisible(pInfo->currentPos, hiderPlayerInfo->currentPos, false,_params->simNotVisibDist );
            bool vis_shDyn = vis_sh;
            if (vis_shDyn) vis_shDyn = _map->isVisible(pInfo->currentPos, hiderPlayerInfo->currentPos, true,_params->simNotVisibDist);
            //AG160506: this should NOT be done here, the dynObsVis are the dyn. obst. which are visible to the robot
            //,                                                       _params->useDynObstForVisibCheck?&pInfo->dynObsVisibleVec:nullptr );
            queryP.addBindValue(vis_sh);
            queryP.addBindValue(vis_shDyn);

            if (pInfo->seekerBeliefScore==-1)
                queryP.addBindValue(QVariant(QVariant::Double));
            else
                queryP.addBindValue(pInfo->seekerBeliefScore);
            queryP.addBindValue(pInfo->seekerReward);

        } else {
            //vis_sh
            queryP.addBindValue(QVariant(QVariant::Bool));
            queryP.addBindValue(QVariant(QVariant::Bool));
            //seeker belief score
            queryP.addBindValue(QVariant(QVariant::Double));
            //seeker reward
            queryP.addBindValue(QVariant(QVariant::Double));
        }

        if (pInfo->chosenGoalPos.isSet()) {
            queryP.addBindValue(pInfo->chosenGoalPos.rowDouble());
            queryP.addBindValue(pInfo->chosenGoalPos.colDouble());
        } else {
            queryP.addBindValue(QVariant(QVariant::Double));
            queryP.addBindValue(QVariant(QVariant::Double));
        }

        //write to db
        ok = queryP.exec();

        if (!ok) {
            cout << "HSGameLogDB::addGameStep: failed to add user "<<pInfo->toString()<<" to game log line. Last error: "
                 <<queryP.lastError().text().toStdString()<<endl;
            return -2;
        }

        if (queryP.next()) {
            qlonglong gamelogUserLineID = queryP.value(0).toLongLong(&ok);

            if (!ok || gamelogUserLineID<0) {
                cout << "HSGameLogDB::addGameStep: failed to add user "<<pInfo->toString()<<" to game log line (retrieving id)"<<endl;
                return -1;
            }
        }
    } // for playerinfo


    return (long)gameLineID;
}

int HSGameLogDB::stopGame() {
    if (!_db.isOpen()) {
        cout << "WARNING: log db is not open"<<endl;
        return -1;
    }

    QSqlQuery query(_db);
    int ret = -1;
    query.prepare("SELECT StopGame(?);");
    query.addBindValue(_gameID);

    bool ok = query.exec();
    if (ok && query.next()) {
        ret = query.value(0).toInt();
    }

    return ret;
}


int HSGameLogDB::close() {
    cout << "Closing DB log"<<endl;
    _db.close();
    return 0;
}


int HSGameLogDB::getID() const {
    return _sessionID;
}

bool HSGameLogDB::isOpen() const {
    return _db.isOpen();
}
