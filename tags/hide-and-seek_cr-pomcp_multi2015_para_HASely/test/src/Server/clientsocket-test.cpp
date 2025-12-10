#include "test/include/Server/clientsocket-test.h"
#include <QTest>
#include <QTcpSocket>

#include "Server/clientsocket.h"
#include "Base/playerinfoserver.h"

#include "exceptions.h"

void ClientSocketTest::clientSocketTest() {
    QTcpSocket socket;

    ClientSocket clientSocket(&socket);

    QVERIFY(clientSocket.getSocket()==&socket);

    PlayerInfoServer pi;
    pi.id = 100;

    clientSocket.addPlayer(&pi);

    QVERIFY(clientSocket.hasPlayerID(pi.id));
    QVERIFY(pi.id==100);
    QVERIFY(!clientSocket.hasPlayerID(0));
    QVERIFY_EXCEPTION_THROWN(clientSocket.hasPlayerID(-1), CException);
    QVERIFY_EXCEPTION_THROWN(clientSocket.addPlayer(&pi), CException);

    PlayerInfoServer pi2;
    pi2.id=0;
    clientSocket.addPlayer(&pi2);
    QVERIFY(clientSocket.hasPlayerID(0));
}


void ClientSocketTest::addSeekerHiderTest() {
    QTcpSocket socket;
    ClientSocket clientSocket(&socket);

    PlayerInfoServer pi;
    pi.playerType = HSGlobalData::P_Seeker;
    pi.id = 100;
    PlayerInfoServer pi2;
    pi2.playerType = HSGlobalData::P_Seeker;
    pi2.id=0;

    clientSocket.addPlayer(&pi);
    clientSocket.addPlayer(&pi2);

    PlayerInfoServer pi3;
    pi3.playerType = HSGlobalData::P_Hider;
    pi3.id=1;

    QVERIFY_EXCEPTION_THROWN(clientSocket.addPlayer(&pi3), CException);

}
