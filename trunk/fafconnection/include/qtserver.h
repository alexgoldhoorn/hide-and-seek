#ifndef QTServer_H
#define QTServer_H

#include <QtNetwork/QTcpServer>

#include <string>

#include "fafconnection.h"

class QtServer : public FAFConnection
{
    Q_OBJECT

public:
    QtServer(/*std::string hostName,*/ unsigned int port);

    virtual ~QtServer();

    void serve();


protected slots:
    void handleNewConnection();
    void readyRead();
    void onDisconnected();

private:
    unsigned int _counter;

    //QString _serverHost;
    quint16 _serverPort;

    QTcpServer _server;
};

#endif
