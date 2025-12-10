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
    bool ok = query.prepare("SELECT AddGame(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
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
        //assert(pInfo->chosenGoalPos.isSet());

        QSqlQuery queryP(_db);
        //bool ok = queryP.prepare("INSERT INTO GameUserLine (GameUserID,GameLineID,ClientTimeStamp) values (?,?,?);");

                /*ClientTimeStamp,`Action`,
                `Row`,
                `Col`,
                `RowCont`,
                `ColCont`,
                HiderRowContWNoise,
                HiderColContWNoise,
                d_sh,
                d_shEuc,
                d_pb,
                d_pbEuc,
                vis_sh,
                visDyn_sh,
                SeekerBeliefScore,
                SeekerReward,
                `ChosenGoalRow`,
                `ChosenGoalCol`
            ) VALUES (
                GameUserID,
                GameLineID,
                ClientTimeStamp,
                `Action`,
                `Row`,
                `Col`,
                `RowCont`,
                `ColCont`,
                HiderRowContWNoise,
                HiderColContWNoise,
                d_sh,
                d_shEuc,
                d_pb,
                d_pbEuc,
                vis_sh,
                visDyn_sh,
                SeekerBeliefScore,
                SeekerReward,
                `ChosenGoalRow`,
                `ChosenGoalCol`
            );*/






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
            bool vis_sh = _map->isVisible(pInfo->currentPos, hiderPlayerInfo->currentPos, false);
            bool vis_shDyn = vis_sh;
            if (vis_shDyn) vis_shDyn = _map->isVisible(pInfo->currentPos, hiderPlayerInfo->currentPos, true);
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

#ifdef OLD_CODE
//AG150113: adapted for 2 seekers
int HSGameLogDB::addGameStep(int actionNum, QDateTime sentTime,
                             QDateTime hiderTime, QDateTime seekerTime, QDateTime seeker2Time,
                             int hiderAction, int seekerAction, int seeker2Action,
                             const Pos& hider, const Pos& seeker, const Pos* seeker2,
                             int status, double dsh, double dsb, double dhb,
                             double dshEuc, double ds2h, double ds2hEuc, double dss2, double dss2Euc,
                             bool vis_sh, bool vis_s2h, bool vis_ss2,
                             const Pos* hiderPosWNoiseSent, const Pos* hiderPosWNoiseSent2,
                             double seekerBeliefScore, double seeker2BeliefScore, double seekerReward, double seeker2Reward,
                             const std::vector<Pos> goalVectorFromS1, const std::vector<double> goalBeliefVectorFromS1, const Pos& multiChosenPosS1,
                             const std::vector<Pos> goalVectorFromS2, const std::vector<double> goalBeliefVectorFromS2, const Pos& multiChosenPosS2) {
    assert(_params!=NULL);
    if (!_db.isOpen()) {
        cout << "WARNING: log db is not open"<<endl;
        return -1;
    }

    QSqlQuery query(_db);

    //AG150113: only do when using 2 seeker robots
    bool use2Robots = (_params->gameType == HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB);

    if (_params->useContinuousPos) {
        query.prepare("SELECT AddGameLineCont(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
    } else {
        query.prepare("SELECT AddGameLine(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
    }

    query.addBindValue(_gameID);
    query.addBindValue(actionNum);
    query.addBindValue(sentTime.toString(DATE_TIME_MYSQL_FORMAT));
    query.addBindValue(hiderTime.toString(DATE_TIME_MYSQL_FORMAT));
    query.addBindValue(seekerTime.toString(DATE_TIME_MYSQL_FORMAT));
    if (use2Robots)
        query.addBindValue(seeker2Time.toString(DATE_TIME_MYSQL_FORMAT));
    else
        query.addBindValue(QVariant(QVariant::String));

    if (hiderAction==-1) {
        query.addBindValue(QVariant(QVariant::Int));
    } else {
        query.addBindValue(hiderAction);
    }
    if (seekerAction==-1) {
        query.addBindValue(QVariant(QVariant::Int));
    } else {
        query.addBindValue(seekerAction);
    }
    if (!use2Robots || seeker2Action==-1) {
        query.addBindValue(QVariant(QVariant::Int));
    } else {
        query.addBindValue(seeker2Action);
    }

    query.addBindValue(hider.row());
    query.addBindValue(hider.col());
    query.addBindValue(seeker.row());
    query.addBindValue(seeker.col());
    if (use2Robots && seeker2!=NULL) {
        query.addBindValue(seeker2->row());
        query.addBindValue(seeker2->col());
    } else {
        query.addBindValue(QVariant(QVariant::Int));
        query.addBindValue(QVariant(QVariant::Int));
    }

    if (_params->useContinuousPos) {
        query.addBindValue(hider.rowDouble());
        query.addBindValue(hider.colDouble());
        query.addBindValue(seeker.rowDouble());
        query.addBindValue(seeker.colDouble());

        if (use2Robots && seeker2!=NULL) {
            query.addBindValue(seeker2->rowDouble());
            query.addBindValue(seeker2->colDouble());
        } else {
            query.addBindValue(QVariant(QVariant::Double));
            query.addBindValue(QVariant(QVariant::Double));
        }
    }

    query.addBindValue(status);

    //AG140212: distances
    if (dsh==-1) {
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(dsh);
    }
    if (dsb==-1) {
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(dsb);
    }
    if (dhb==-1) {
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(dhb);
    }

    //AG140601: dsh eucledian
    if (dshEuc==-1) {
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(dshEuc);
    }    

    if (!use2Robots || ds2h==-1) {
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(ds2h);
    }
    if (!use2Robots || ds2hEuc==-1) {
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(ds2hEuc);
    }
    if (!use2Robots || dss2==-1) {
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(dss2);
    }
    if (!use2Robots || dss2Euc==-1) {
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(dss2Euc);
    }

    //AG150113: add visibility
    query.addBindValue(vis_sh);
    query.addBindValue(vis_s2h);
    query.addBindValue(vis_ss2);

    //AG140606: hider pos with noise
    if (hiderPosWNoiseSent==NULL) {
        query.addBindValue(QVariant(QVariant::Double));
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(hiderPosWNoiseSent->rowDouble());
        query.addBindValue(hiderPosWNoiseSent->colDouble());
    }

    //AG150203: hider pos with noise sent to seeker 2
    if (hiderPosWNoiseSent2==NULL) {
        query.addBindValue(QVariant(QVariant::Double));
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(hiderPosWNoiseSent2->rowDouble());
        query.addBindValue(hiderPosWNoiseSent2->colDouble());
    }

    //AG140409: seeker belief score
    if (seekerBeliefScore==-1) {
        //should bind a NULL value
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(seekerBeliefScore);
    }
    //AG140612: seeker reward
    query.addBindValue(seekerReward);

    if (!use2Robots || seeker2BeliefScore==-1) {
        //should bind a NULL value
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(seeker2BeliefScore);
    }
    if (!use2Robots) {
        //should bind a NULL value
        query.addBindValue(QVariant(QVariant::Double));
    } else {
        query.addBindValue(seeker2Reward);
    }

    //AG150214: write poses for selection
    //from S1 - goal S1
    if (use2Robots && goalVectorFromS1.size()>0) {
        query.addBindValue(goalVectorFromS1[0].rowDouble());
        query.addBindValue(goalVectorFromS1[0].colDouble());
    } else {
        query.addBindValue(QVariant(QVariant::Double));
        query.addBindValue(QVariant(QVariant::Double));
    }
    //from S1 - belief goal S1
    if (use2Robots && goalBeliefVectorFromS1.size()>0) {
        query.addBindValue(goalBeliefVectorFromS1[0]);
    } else {
        query.addBindValue(QVariant(QVariant::Double));
    }
    //from S1 - goal S2
    if (use2Robots && goalVectorFromS1.size()>1) {
        query.addBindValue(goalVectorFromS1[1].rowDouble());
        query.addBindValue(goalVectorFromS1[1].colDouble());
    } else {
        query.addBindValue(QVariant(QVariant::Double));
        query.addBindValue(QVariant(QVariant::Double));
    }
    //from S1 - belief goal S2
    if (use2Robots && goalBeliefVectorFromS1.size()>0) {
        query.addBindValue(goalBeliefVectorFromS1[1]);
    } else {
        query.addBindValue(QVariant(QVariant::Double));
    }

    //from S2 - goal S1
    if (use2Robots && goalVectorFromS2.size()>0) {
        query.addBindValue(goalVectorFromS2[0].rowDouble());
        query.addBindValue(goalVectorFromS2[0].colDouble());
    } else {
        query.addBindValue(QVariant(QVariant::Double));
        query.addBindValue(QVariant(QVariant::Double));
    }
    //from S2 - belief goal S1
    if (use2Robots && goalBeliefVectorFromS2.size()>0) {
        query.addBindValue(goalBeliefVectorFromS2[0]);
    } else {
        query.addBindValue(QVariant(QVariant::Double));
    }
    //from S2 - goal S2
    if (use2Robots && goalVectorFromS2.size()>1) {
        query.addBindValue(goalVectorFromS2[1].rowDouble());
        query.addBindValue(goalVectorFromS2[1].colDouble());
    } else {
        query.addBindValue(QVariant(QVariant::Double));
        query.addBindValue(QVariant(QVariant::Double));
    }
    //from S2 - belief goal S2
    if (use2Robots && goalBeliefVectorFromS2.size()>0) {
        query.addBindValue(goalBeliefVectorFromS2[1]);
    } else {
        query.addBindValue(QVariant(QVariant::Double));
    }

    //multi chosen pos - S1
    if (use2Robots) {
        query.addBindValue(multiChosenPosS1.rowDouble());
        query.addBindValue(multiChosenPosS1.colDouble());
    } else {
        query.addBindValue(QVariant(QVariant::Double));
        query.addBindValue(QVariant(QVariant::Double));
    }
    //multi chosen pos - S2
    if (use2Robots) {
        query.addBindValue(multiChosenPosS2.rowDouble());
        query.addBindValue(multiChosenPosS2.colDouble());
    } else {
        query.addBindValue(QVariant(QVariant::Double));
        query.addBindValue(QVariant(QVariant::Double));
    }

    bool ok = query.exec();

    if (!ok) return -2;

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}
#endif

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
    /*do {
        //QThread::msleep(1);
        cout << "."<<flush;
    } while (_db.isOpen());
*/
    return 0;
}


int HSGameLogDB::getID() {
    return _sessionID;
}
