#ifndef QTClient_H
#define QTClient_H

#include <string>

#include "fafconnection.h"

class QtClient : public FAFConnection
{
    Q_OBJECT

public:
    QtClient(std::string hostName, unsigned int port);

    virtual ~QtClient();

    //connect to server
    bool connectToServer();


protected slots:
    //! info received from server
    void serverReadyRead();

    //! server disconnected
    void onServerDisconnect();


private:

    QString _serverHost;
    quint16 _serverPort;
};



#endif
