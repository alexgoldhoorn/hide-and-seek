#include "hsgamelogdb.h"
#include "../HSGame/gmap.h"


#include <QSqlDriver>
#include <QSqlQuery>

#include <QVariant>
#include <QString>

HSGameLogDB::HSGameLogDB(HSServerConfig *config)
{
    _config = config;

    QString dbServer,dbDB,dbUser,dbPass;
    _config->getLogDB(dbServer,dbDB,dbUser,dbPass);
    //cout << "Log DB:        " << dbDB.toStdString() << "@" << dbServer.toStdString() << endl;



    _db = QSqlDatabase::addDatabase("QMYSQL"); //,QString::fromStdString(_connectionName));
    _db.setHostName(dbServer);
    _db.setDatabaseName(dbDB);
    _db.setUserName(dbUser);
    _db.setPassword(dbPass);

    /*cout << "Log DB:        '" << dbDB.toStdString() << "@" << dbServer.toStdString() << "'"<<endl;
    cout << "  user: '"<<dbUser.toStdString()<<"', p:'"<<dbPass.toStdString()<<"'"<<endl;*/

    _sessionID=-1;

    bool ok = _db.open();
    if (ok) {
        //_driverHasFeatQuerySize = _db.driver()->hasFeature(QSqlDriver::QuerySize);
        cout << "DB connected" <<endl;
        //cout << "has query size feat: "<<_driverHasFeatQuerySize<<endl;

        //now get session id
        QSqlQuery query(_db);
        ok = query.exec("SELECT StartServer();");
        if (ok && query.next()) {
            _sessionID = query.value(0).toInt();
        }

        if (_sessionID>=0) {
            cout << " session id: "<<_sessionID<<endl;
        } else {
            cout << " could not open a session!"<<endl;
        }
    } else {
        cout << "Could not open the database."<<endl;
    }

    //return ok;
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

int HSGameLogDB::startGame(QString mapName, GMap *gmap, QString seeker, QString hider, int maxActions) {
    if (!_db.isOpen()) {
        cout << "WARNING: log db is not open"<<endl;
        return -1;
    }
    /*
CREATE DEFINER=`root`@`localhost` FUNCTION `AddGameByName`(
    MapName VARCHAR(50),
    MapWidth INT,
    MapHeight INT,
    MapNumObst INT,
    BaseX INT,
    BaseY INT,
    MaxActions INT,
    SeekerUser VARCHAR(50),
    HiderUser VARCHAR(50)
)
     */

    QSqlQuery query(_db);

    query.prepare("SELECT AddGameByName(?, ?, ?, ?, ?, ?, ?, ?, ?);");
    query.addBindValue(mapName);
    query.addBindValue(gmap->colCount());
    query.addBindValue(gmap->rowCount());
    query.addBindValue(gmap->numObstacles()); //ag130304: renamed fromcountObstacles
    query.addBindValue(gmap->getBase().row);
    query.addBindValue(gmap->getBase().col);
    query.addBindValue(maxActions);
    query.addBindValue(seeker);
    query.addBindValue(hider);

    bool ok = query.exec();

    if (!ok) return -2;

    if (query.next()) {
        _gameID =  query.value(0).toInt();
    } else {
        _gameID = -1;
    }

    return _gameID;
}


int HSGameLogDB::addGameStep(int actionNum, QDateTime hiderTime, QDateTime seekerTime, int hiderAction,
                             int seekerAction, int hiderRow, int hiderCol, int seekerRow, int seekerCol, int status) {
    if (!_db.isOpen()) {
        cout << "WARNING: log db is not open"<<endl;
        return -1;
    }

    QSqlQuery query(_db);

    query.prepare("SELECT AddGameLine(?,?,?,?,?,?,?,?,?,?,?);");
    query.addBindValue(_gameID);
    query.addBindValue(actionNum);
    query.addBindValue(hiderTime);
    query.addBindValue(seekerTime);
    query.addBindValue(hiderAction);
    query.addBindValue(seekerAction);
    query.addBindValue(hiderRow);
    query.addBindValue(hiderCol);
    query.addBindValue(seekerRow);
    query.addBindValue(seekerCol);
    query.addBindValue(status);

    bool ok = query.exec();

    if (!ok) return -2;

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
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
    cout << "Closing DB for log"<<endl;
    _db.close();
    /*do {
        //QThread::msleep(1);
        cout << "."<<flush;
    } while (_db.isOpen());
*/
    return 0;
}
