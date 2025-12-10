#include "Server/clientsocket.h"

#include "exceptions.h"

#include <cassert>


using namespace std;

int ClientSocket::_LAST_ID = 0;

ClientSocket::ClientSocket(QTcpSocket *tcpSocket) : _socket(tcpSocket), _numSeekers(0), _isSeeker(true) {
    assert(_socket!=NULL);

    _id = _LAST_ID++;
    _name = "Client#"+QString::number(_id)+":{";

    //connect signals
    connect(_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
}

void ClientSocket::readyRead() {
    emit socketReadyRead(this);
}

void ClientSocket::disconnected() {
    emit socketDisconnected(this);
}

void ClientSocket::error(QAbstractSocket::SocketError error) {
    emit socketError(this, error);
}

void ClientSocket::addPlayer(PlayerInfoServer *player) {
    assert(player!=NULL);
    if (player->id<0) throw CException(_HERE_, "Player ID should be >=0");
    if (hasPlayerID(player->id)) throw CException(_HERE_, "There already exists a player with the same ID "+QString::number(player->id).toStdString());

    if (player->isSeeker()) {
        _numSeekers++;

        //AG150710: assure there are only seekers
        if (!_playerVector.empty() && !_isSeeker)
            throw CException(_HERE_,"in one client either seekers or hiders are allowed, not a combination, now trying to add a seeker while a hider was added");
        _isSeeker = true;
    } else {
        //AG150710: assure there are only hiders
        if (!_playerVector.empty() && _isSeeker)
            throw CException(_HERE_,"in one client either seekers or hiders are allowed, not a combination, now trying to add a hider while a seeker was added");
        _isSeeker = false;
    }

    //add player
    _playerVector.push_back(player);
    _playerIDVector.push_back((quint8)player->id);

    //prepare name string
    if (_playerVector.size()>1) _name += ",";
    _name += QString::fromStdString(player->toString());
}

PlayerInfoServer* ClientSocket::getPlayer(int i) {
    return _playerVector[i];
}

int ClientSocket::getNumPlayers() {
    return _playerVector.size();
}

int ClientSocket::getNumSeekers() {
    return _numSeekers;
}

QTcpSocket* ClientSocket::getSocket() {
    return _socket;
}

std::string ClientSocket::toString() {
    return _name.toStdString() + "}";
}

bool ClientSocket::hasPlayerID(int id) {
    if (id<0) throw CException(_HERE_, "Player ID should be >=0");
        /*return false;
    else*/
        return find(_playerIDVector.begin(), _playerIDVector.end(), (quint8)id)!=_playerIDVector.end();
}

void ClientSocket::close(bool disableDisconnectSignal) {
    //avoid disconnection signal
    if (disableDisconnectSignal)
        disconnect(_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

    //disconnect
    _socket->close();
}

bool ClientSocket::isSeeker() {
    return _isSeeker;
}
