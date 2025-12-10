#ifndef HSTCPServer_H
#define HSTCPServer_H

#include <QtNetwork/QTcpServer>
#include <QMap>
#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QFile>
#include <QDateTime>
#include <QProcess>

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#include "../HSGame/gmap.h" //ag
//#include "HSGui/gmapwidget.h"
#include "hsserver.h" //ag

#include "../Utils/generic.h" //ag

#include "hsserverconfig.h"
#include "hsgamelog.h"

#ifdef AUTOHIDER_NEW
#include "../AutoHider/autohider.h"
#endif

using namespace std;



void raytrace(char** m, int x0, int y0, int x1, int y1);


static const int MAXACTIONS         = 500;
//! Struct for PlayerInfo info on server.
struct PlayerInfo {
    Pos current; //current position
    Pos previous; //AG121112: previous pos
    int numa; //number of actions taken
    int action[MAXACTIONS]; //actions
    bool flag; //1-if the PlayerInfo is ready to write, 0-if not
    //in order for the PlayerInfo to be able to write he should have read smt, either the opponentś pos or an invalid msg.
    bool set; //1 if the PlayerInfo is defined... corresponds to a socket,0 elsewise
    int win; //0-game on, 1-win, -1 -loose, 2-tie
    QString username; //playerś name
    QDateTime timestamp;    //a timestamp indicating the exact time that the user took the last action.
};


class HSTCPServer : public QTcpServer
{
    Q_OBJECT

public:
    static const float MAX_ACT_MULT_FACTOR = 2.0;


    /*static QString PATH;
    static  QStringList MAPS;*/

    HSTCPServer(HSServerConfig* config, HSGameLog* gameLog, QObject *parent=0);//constructs a world

#ifdef OLD_CHRYSO_CODE
    Pos mvPlayer(int action, Pos a ); //returns the new position of the PlayerInfo
#endif

    GMap* getGMap() {
        return _gmap;
    }

    PlayerInfo* getHider() {
        return _hider;
    }

    PlayerInfo* getSeeker() {
        return _seeker;
    }

    int checkstatus(); //returns 0-game on, 1-seeker wins, 2-hider wins, 3-tie game over.

    PlayerInfo* getwinner() {
        if(_hider->win==1)
            return _hider;
        if(_seeker->win==1)
            return _seeker;
        else return 0;
    }
    QMap<QTcpSocket*, char> mclient;

    QMap<int, QString> maps;



    //ag111201
    void setServer(HSServer* server) {
        _server = server;
    }


    int writetofile();

    void restartserver();

    void newgame();
    void initplayers();
    void  initializeLogs();
    void sendmaptoPlayers();

    void setip(QString i) {
        _ip = i;
    }

    void setport(int p) {
        _port=p;
    }

    //ag120903: update hidertype to support action file
    void callrandhider(int hiderType, QString actionFile);

    void callSeekroBot();

    void sendinittoseeker();

    //ag120523: start new server instance
    bool startNewServerInstance();

private slots:
    void newConnection();
    void hreadyRead();
    void sreadyRead();
    void ondisconnect();

    //AG120514: functions for map log path
//    QString getMapPath();
//    QString getLogPath();


private:
    //ag130724
    QString getMapName(int mapID);


    GMap* _gmap;
    PlayerInfo* _hider;
    PlayerInfo* _seeker;
    //std::vector<PlayerInfo*> _players;
    QTcpSocket* _hidersock;
    QTcpSocket* _seekersock;
    int _numcon;//number of connections;
    int status; //0-game on, 1-seeker wins, 2-hider wins, 3-tie game over.
    //GMapWidget* _w;
    HSServer* _server; //AG111201
    //AGc: log file
    QFile fd; //TODO: remove this -> to log class
    int _map;
    int _opp;
    QProcess* _proc;
    QString _ip;
    int _port;
    //QString _filename; //the map filename (with the path)

    //ag130404: win distance for winning from hider
    int _winDist;

    //ag120511
    int _maxActions;

    //ag120515
    HSServerConfig* _config;

    //ag120530
    HSGameLog* _gameLog;

#ifdef AUTOHIDER_NEW
    //AG120608
    AutoHider* _autoHider;
#endif
};



#endif // HSTCPServer_H
