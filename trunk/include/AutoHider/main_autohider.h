#ifndef GMAPWIDGET_H
#define GMAPWIDGET_H

#include <QFile>
#include <QTimer>
#include <QtNetwork/QTcpSocket>
#include "HSGame/gworld.h"
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>


class GMapWidget  : public QObject
{
    Q_OBJECT
public:

    //GMapWidget(char * ip, char* port, char* type);
    GMapWidget(QString ip, int port, int hiderType, QString actionFile);

    ~GMapWidget();

    void sendInfomsg();

    GMap* getGMap() {
        return _gmap;
    }
    Player* getPlayer(){
        return &_player;
    }

    bool sendaction(); //1-for success, 0-for failure.

    void takenewaction(int a=-1);

    void setInitialPos(); //sets the initial position and sends it to the server


    void readyReadTest(); //AG111202: test if ready for read

public slots:
        void readyRead();
        void ondisconnect();



private:
    void init(GMap* gmap);
    void init(); //int type, Player* pl);

    GMap* _gmap;
    Player _player; //ag120903: no pointer
    QTcpSocket *tcpSocket;
    quint16 blockSize;
    QString _ip;
    int _port;
    int _map;

    bool _stopped; //AG111202: stopped program


};

#endif // GMAPWIDGET_H
