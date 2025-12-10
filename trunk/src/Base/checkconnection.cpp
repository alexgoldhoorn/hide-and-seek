#include "Base/checkconnection.h"

#include <QTimer>
#include <QCoreApplication>

#include <cassert>
#include <iostream>


CheckConnection::CheckConnection(GameConnectorClient* gameConnector) : _initDone(false),
    _msec(0), _gameConnector(gameConnector) {

    assert(gameConnector!=nullptr);
    connect(gameConnector, SIGNAL(serverParamsRead()), this, SLOT(serverParamsRead()));
}

void CheckConnection::start(int msec) {
    assert(!_initDone);
    _msec = msec;
    //set timer
    QTimer::singleShot(msec, this, SLOT(timePassed()));
}

void CheckConnection::serverParamsRead() {
    assert(!_initDone);
    _initDone = true;
}

void CheckConnection::timePassed() {
    if (!_initDone) {
        std::cout << "ERROR: time passed ("<<(_msec/1000)<<" s) and no server parameters received, exiting..."<<std::endl;
        QCoreApplication::exit(10);
    }
}

