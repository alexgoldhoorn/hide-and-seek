#include "Server/hsgamelogdb.h"
#include "HSGame/gmap.h"

#include <iostream>

#include <QSqlDriver>
#include <QSqlQuery>
#include <QVariant>
#include <QString>

#include "Server/hstcpserver.h"
#include "hsglobaldata.h"

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

int HSGameLogDB::startGame(QString mapName, GMap *gmap, PlayerInfo *seekerInfo, PlayerInfo *hiderInfo,
                           PlayerInfo* seeker2Info, SeekerHSParams* params) {

    _params = params;

    if (!_db.isOpen()) {
        cout << "WARNING: log db is not open"<<endl;
        return -1;
    }

    //generate query
    QSqlQuery query(_db);

    //AG140601: extra meta info
    query.prepare("SELECT AddGameByName(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
    query.addBindValue(seekerInfo->username);
    query.addBindValue(hiderInfo->username);
    //AG150112: add name seeker2
    if (seeker2Info == NULL)
        query.addBindValue("");
    else
        query.addBindValue(seeker2Info->username);

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
    //players info
    queryAddBindValueStr(query, seekerInfo->metaInfo);
    queryAddBindValueStr(query, hiderInfo->metaInfo);
    //AG150112: add name seeker2
    if (seeker2Info == NULL)
        query.addBindValue("");
    else
        queryAddBindValueStr(query, seeker2Info->metaInfo);

    queryAddBindValueStr(query, seekerInfo->comments);
    queryAddBindValueStr(query, hiderInfo->comments);
    if (seeker2Info == NULL)
        query.addBindValue("");
    else
        queryAddBindValueStr(query, seeker2Info->comments);

    //execute query
    bool ok = query.exec();

    if (!ok) return -2;

    if (query.next()) {
        _gameID =  query.value(0).toInt();
        //cout << "Game ID: "<<_gameID<<endl;
    } else {
        _gameID = -1;
        cout << "Could not add game to database!"<<endl;
    }

    return _gameID;
}

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
