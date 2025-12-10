#include "fafconnection.h"
#include "qtscglobal.h"
#include <cassert>
#include <QByteArray>

#include <iostream>

using namespace std;

/*FAFConnection::FAFConnection() {
    _socket = NULL;
}*/

FAFConnection::FAFConnection(FAFConnectionHandler* handler /*void (*connectedFP)(),
                             void (*disconnectedFP)(),
                             void (*observationsReceivedFP)(std::vector<double>, std::vector<double>),
                             void (*goalsReceivedFP)(std::vector<double>, double, std::vector<double>, double)*/)
    : QObject(), _connectedFP(NULL), _disconnectedFP(NULL), _observationsReceivedFP(NULL),_goalsReceivedFP(NULL),
      _handler(handler)
    //, _connectedFP(connectedFP), _disconnectedFP(disconnectedFP), _observationsReceivedFP(observationsReceivedFP),_goalsReceivedFP(goalsReceivedFP)
{

}

void FAFConnection::setFP(void (*connectedFP)(),
                             void (*disconnectedFP)(),
                             void (*observationsReceivedFP)(std::vector<double>, std::vector<double>),
                             void (*goalsReceivedFP)(std::vector<double>, double, std::vector<double>, double)) {
    _connectedFP = connectedFP;
    _disconnectedFP = disconnectedFP;
    _observationsReceivedFP = observationsReceivedFP;
    _goalsReceivedFP = goalsReceivedFP;
}

FAFConnection::~FAFConnection(){
}

void FAFConnection::sendObservations(std::vector<double> mySeekerPos, std::vector<double> myHiderPos) {
    assert(mySeekerPos.size()>=2);
    assert(myHiderPos.size()==0 || myHiderPos.size()>=2);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(DATASTREAM_VERSION);
    //block size
    out <<(BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_Obs;

    sendVector(out, mySeekerPos);
    sendVector(out, myHiderPos);

    //block size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _socket->write(block);
    _socket->flush();
}

void FAFConnection::sendGoals(std::vector<double> myGoalPos, double myGoalPosBelief, std::vector<double> otherGoalPos, double otherGoalPosBelief) {
    assert(myGoalPos.size()>=2);
    assert(otherGoalPos.size()==0 || otherGoalPos.size()>=2);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(DATASTREAM_VERSION);
    //block size
    out <<(BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_Goals;

    sendVector(out, myGoalPos);
    out << myGoalPosBelief;
    sendVector(out, otherGoalPos);
    out << otherGoalPosBelief;

    //block size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _socket->write(block);
    _socket->flush();
}

void FAFConnection::sendVector(QDataStream &out, const std::vector<double> &vec) {
    out << (quint8)vec.size();
    for(double d: vec) {
        out << d;
    }
}

void FAFConnection::readyReadGeneric() {
    BLOCK_SIZE blockSize=0;
    QDataStream in(_socket);
    in.setVersion(DATASTREAM_VERSION);

    if (_socket->bytesAvailable() < (int)sizeof(BLOCK_SIZE)) {
        cout<<"less bytes in the array than expected1."<<endl;
        return;
    }
    in >> blockSize;

    if (_socket->bytesAvailable() < blockSize) {
        cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
        return;
    } else if (blockSize==0) {
        cout<<"GameConnectorClient::serverReadyRead: unexpected: blockSize=0"<<endl;
        return;
    }

    //AG150120: message type
    MESSAGE_TYPE_SIZE messageType;
    in >> messageType;

    switch ((MessageType)messageType) {
        case MT_Obs:
            cout << "read obs"<<endl;
            readObservations(in);
            break;
        case MT_Goals:
            cout <<"read goals"<<endl;
            readGoals(in);
            break;
        default:
            cout << "Unkown message type."<<endl;
            return;
    }
}

void FAFConnection::readObservations(QDataStream& in) {
    vector<double> otherSeekerPos = readVector(in);
    vector<double> otherHiderPos = readVector(in);

    //Qt signal
    observationsReceived(otherSeekerPos, otherHiderPos);
    //fp
    if (_observationsReceivedFP!=NULL)
        _observationsReceivedFP(otherSeekerPos, otherHiderPos);
    //handler
    if (_handler!=NULL)
        _handler->handleObservationsReceived(otherSeekerPos, otherHiderPos);
}

void FAFConnection::readGoals(QDataStream& in) {
    vector<double> otherGoalPos = readVector(in);
    double otherGoalPosBel;
    in >> otherGoalPosBel;
    vector<double> myGoalPos = readVector(in);
    double myGoalPosBel;
    in >> myGoalPosBel;

    //Qt signal
    goalsReceived(myGoalPos, myGoalPosBel, otherGoalPos, otherGoalPosBel);
    //fp
    if (_goalsReceivedFP!=NULL)
        _goalsReceivedFP(myGoalPos, myGoalPosBel, otherGoalPos, otherGoalPosBel);
    //handler
    if (_handler!=NULL)
        _handler->handleGoalsReceived(myGoalPos, myGoalPosBel, otherGoalPos, otherGoalPosBel);
}

std::vector<double> FAFConnection::readVector(QDataStream& in) {
    quint8 sz;
    in >> sz;

    vector<double> vec(sz);

    for(quint8 i=0; i<sz; i++) {
        in >> vec[i];
    }

    return vec;
}

bool FAFConnection::isConnected() const {
    if (_socket==NULL) {
        return false;
    } else {
        return _socket->isOpen();
    }
}

void FAFConnection::closeConnection() {
    cout << "Closing connection..."<<endl;
    if (_socket!=NULL) {
        if (_socket->isOpen()) {
            _socket->close();
        }
    }
}

void FAFConnection::setConnectionHandler(FAFConnectionHandler *handler) {
    _handler = handler;
}
