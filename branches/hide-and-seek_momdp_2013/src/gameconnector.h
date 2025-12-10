#ifndef GAMECONNECTOR_H
#define GAMECONNECTOR_H

/*

  AG120402: renamed from GMapWidget -> gameconnector
  */


#include <QFile>
#include <QTimer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>

//#include "Solver/hsmomdp.h"
#include "autoplayer.h"

#include "HSGame/gplayer.h" //gworld.h"

#include "seekerhs.h"


/*! This class handles the game connection to the server and the game flow.
  AG-TODO:
  - refactor!!
  */
class GameConnector : public QObject
{
    Q_OBJECT
public:

    //ag120112: momdp assumed to be initialized, however should calculate the initial belief (based on the player map)
    //AG130226: HSMOMDP -> AutoPlayer
    GameConnector(string ip, int port, int type, AutoPlayer* autoplayer, SeekerHS* seekerHS = NULL, int mapID = 0,
                  int optype=0, string username="SeekroBot", int numofgames=1, string actionFileList="", int winDist=0, GMap* gmap=NULL);

    ~GameConnector();

    //! send info msg, and send gmap if not NULL
    void sendInfomsg(GMap* gmap);

    GMap* getGMap() {
        return _gmap;
    }
    Player* getPlayer(){
        return _player;
    }

    bool sendaction(); //1-for success, 0-for failure.

    void takenewaction(int a=-1);

    void setInitialPos(); //sets the initial position and sends it to the server


    void readyReadTest(); //AG111202: test if ready for read

    void receivehinitpos(); // for the seeker only. receives from server the initial pos of the hider

    void takenewseekeraction(); // for the seeker only! seeker chooses his next action -through POMDP

    //ag130417: to test seekerhs
    void setSeekerHS(SeekerHS* seekerHS);

public slots:
        void readyRead();
        void ondisconnect();



private:
    GMap* _gmap;
    Player* _player;
    void init(GMap* gmap);
    void init(int type, Player* pl, AutoPlayer* autoplayer, string username,GMap* gmap);
    QTcpSocket tcpSocket;
    quint16 blockSize;
    QString _ip;
    int _port;
    int _map;
    int _optype;//opponent type 0=human, 1=randomhider, 2=smarthider
    int _numofgames; //the number of games that will be played in the case that humans are not involved.
    int _winDist; //ag130404: distance for winning to hider

    bool _stopped; //AG111202: stopped program

    //HSMOMDP* _momdp; //AG120112: momdp
    AutoPlayer* _autoplayer; //AG130226: auto player (instead of momdp)

    //int _actions; //ag120112: round to know when the first round is played or later

    //120904: action file list
    QString _actionFileList;

    SeekerHS* _seekerHS;

};

#endif // GAMECONNECTOR_H
