#ifndef CLIENTSOCKET
#define CLIENTSOCKET

#include <QTcpSocket>
#include <vector>

#include "Base/playerinfoserver.h"

//AG150430
//! A wrapper around the QTcpSocket which emits a signal (readyReadSocket) with this object as parameter,
//! whereas the QTcpSocket.readyRead signal does not send any parameters. In this way we know the origin of
//! the message.
class ClientSocket : public QObject {
    Q_OBJECT
public:
    ClientSocket(QTcpSocket* socket);

    //! get socket
    QTcpSocket* getSocket();

    /*!
     * \brief addPlayer add player to this client
     * \param player
     */
    void addPlayer(PlayerInfoServer* player);

    //! number of players for this connection
    int getNumPlayers();

    /*!
     * \brief getPlayer get player
     * \param i index in client (not ID)
     * \return
     */
    PlayerInfoServer* getPlayer(int i);

    //! number of seekers for this connection
    int getNumSeekers();

    //! to string
    std::string toString();

    //! check if a player with this ID is connected through this client
    bool hasPlayerID(int id);

    /*!
     * \brief close close connection
     * \param disableDisconnectSignal avoid a 'disconnected' signal to be sent
     */
    void close(bool disableDisconnectSignal = true);

    //! returns if client contains seekers
    //! note: a combination seeker and hider for one client is not possible
    bool isSeeker();

private slots:
    //! internal ready read for socket
    void readyRead();

    //! internal disconnected slot for socket
    void disconnected();

    //! internal error slot for socket
    void error(QAbstractSocket::SocketError);

signals:
    /*!
     * \brief readyReadSocket emits when the socket is ready for reading, using the socket
     */
    void socketReadyRead(ClientSocket*);

    /*!
     * \brief socketDisconnected emits when the socket is disconnected
     */
    void socketDisconnected(ClientSocket*);

    /*!
     * \brief socketError emits when a socket error has been detected
     */
    void socketError(ClientSocket*,QAbstractSocket::SocketError);


private:
    //! socket
    QTcpSocket* _socket;
    //! list of players covered by this socket
    std::vector<PlayerInfoServer*> _playerVector;
    //! list of player ids
    std::vector<quint8> _playerIDVector;
    //! number of seekers
    int _numSeekers;
    //! last id of client
    static int _LAST_ID;
    //! id of client
    int _id;
    //! client name
    QString _name;
    //! is seeker
    bool _isSeeker;
};

#endif // CLIENTSOCKET

