#ifndef CHECKCONNECTION_H
#define CHECKCONNECTION_H

#include <QObject>

#include "Base/gameconnectorclient.h"

//! This class should detect if the GameConnecterClient has received the
//! parameters of the server (and thus they clients should be connected).
//! If this takes too long, it will exit.
class CheckConnection : public QObject {
    Q_OBJECT
public:
    CheckConnection(GameConnectorClient* gameConnector);

    //! start timer
    void start(int msec);

public slots:
    void serverParamsRead();

    //! called if time passed
    void timePassed();

private:
    bool _initDone;
    int _msec;
    GameConnectorClient* _gameConnector;
};

#endif // CHECKCONNECTION_H
