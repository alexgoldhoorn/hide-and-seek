#include "qtserver.h"
#include <iostream>

#include "qtscglobal.h"

using namespace std;


QtServer::QtServer(/*std::string hostName,*/ unsigned int port) : FAFConnection(),_counter(0) {
    cout << "Starting server on port "<< port <<"..."<<endl;

    //_serverHost = QString::fromStdString(hostName);
    _serverPort = port;
    //_socket = NULL;

    connect(&_server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));

    serve();
}

void QtServer::serve() {

    if (!_server.listen(QHostAddress::Any,_serverPort)) {
        cout <<"Server. Unable to start the server, exiting... "<<endl;
        //exit(-1);
        return;
    }
}

QtServer::~QtServer() {
    if (_socket!=NULL)
        delete _socket;
}

void QtServer::handleNewConnection() {
    cout << "New connection: #"<<(++_counter)<<": "<<flush;

    _socket = _server.nextPendingConnection();
    //assert(_socket!=NULL);
    cout <<_socket->socketDescriptor()<<" "<<flush;

    if(_socket->isOpen()) {
        //wait to get data
        while (!_socket->waitForReadyRead(6000)) {
            cout<<"."<<flush;
        }
        //connect to slot
        connect(_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(_socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

        cout<<" reading: "<<flush;

        //read
        BLOCK_SIZE blockSize=0;
        QDataStream in(_socket);
        in.setVersion(DATASTREAM_VERSION);

        if (blockSize == 0) {
            if (_socket->bytesAvailable() < (int)sizeof(BLOCK_SIZE)) {
                cout <<"less bytes in the array than expected: size."<<endl;
                _socket->flush();
                return; //wait for more (?)
            }
            //block size
            in >> blockSize;
        }


        if (_socket->bytesAvailable() < blockSize) {
            cout <<"less bytes in the array than expected 1. blocksize="
                         <<blockSize<<", available:"<<_socket->bytesAvailable() <<endl;
            _socket->flush();
            return;
        }

        //AG150120: read the message type (although here shoulde be con request)
        quint8 messageType;
        in >> messageType;

        switch ((MessageType)messageType) {
            case MT_ClientRequestConn:
                //ok
                cout << "all ok"<<endl;
                break;
            default:
                cout << "Unkown message type."<<endl;
                return;
        }

        //read more??

        //Qt signal
        connected();
        //fp
        if (_connectedFP!=NULL)
            _connectedFP();
        //handler
        if (_handler!=NULL)
            _handler->gotConnected();

    } else {
        cout << "connection closed"<<endl;
    }
}

void QtServer::readyRead() {
    cout<<"readyRead: "<<endl;

    readyReadGeneric();
}

void QtServer::onDisconnected() {
    cout << "Disconnected"<<endl;

    //Qt signal
    disconnected();
    //fp
    if (_disconnectedFP!=NULL)
        _disconnectedFP();
    //handler
    if (_handler!=NULL)
        _handler->gotDisconnected();
}

