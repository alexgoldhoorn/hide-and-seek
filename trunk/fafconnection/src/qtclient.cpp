#include "qtclient.h"
#include "qtscglobal.h"

#include <iostream>
using namespace std;

QtClient::QtClient(std::string hostName, unsigned int port) : FAFConnection()  {
    _serverHost = QString::fromStdString(hostName);
    _serverPort = port;
    _socket = NULL;

    //connectToServer();
}

QtClient::~QtClient() {
    if (_socket!=NULL) delete _socket;
}

bool QtClient::connectToServer() {
    cout <<"Client: connecting to server "<<_serverHost.toStdString()<<":"<<_serverPort<<" ... " <<flush;
    bool ok = true;

    if (_socket!=NULL) { //AG150219: for if we need to reconnect(??)
        //first disconnect old socket
        if (_socket->isOpen()) _socket->close();
        delete _socket;
    }

    //create new socket
    _socket = new QTcpSocket(this);
    _socket->connectToHost(_serverHost, _serverPort);
    if(_socket->waitForConnected(10000)) {
        if (_socket->isOpen()) {
            cout << "ok"<<endl;

            //set disconnected
            connect(_socket, SIGNAL(disconnected()), this, SLOT(onServerDisconnect()));
            //connect reading slot
            connect(_socket, SIGNAL(readyRead()), this, SLOT(serverReadyRead()));

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(DATASTREAM_VERSION);
            //block size
            out <<(BLOCK_SIZE)0;
            //AG150120: message type
            out<<(MESSAGE_TYPE_SIZE)MT_ClientRequestConn;

            //block size
            out.device()->seek(0);
            out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
            _socket->write(block);
            _socket->flush();

            //Qt signal
            connected();
            //fp
            if (_connectedFP!=NULL)
                _connectedFP();
            //handler
            if (_handler!=NULL)
                _handler->gotConnected();

        } else {
            cout << "disconnected"<<endl;
            ok = false;
        }

    } else {
        cout << "Error, could not connect, try again."<<endl;
        ok = false;
    }

    return ok;
}



void QtClient::serverReadyRead() {
    cout << "serverReadyRead: "<<flush;

    readyReadGeneric();
}



void QtClient::onServerDisconnect() {
    cout << "Server disconnected"<<endl;

    //Qt signal
    disconnected();
    //fp
    if (_disconnectedFP!=NULL)
        _disconnectedFP();
    //handler
    if (_handler!=NULL)
        _handler->gotDisconnected();
}

