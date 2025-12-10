#ifndef HSTCPServer_H
#define HSTCPServer_H

#include <QtNetwork/QTcpServer>
#include <QMap>
#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QFile>
#include <QDateTime>
#include <QProcess>
#include <QTimer>

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <random>

#include "HSGame/gmap.h"
#include "Server/hsserver.h"
#include "Base/playerinfoserver.h"

#include "Utils/generic.h" //ag

#include "Server/hsserverconfig.h"
#include "Server/hsgamelog.h"
#include "Server/clientsocket.h"

#include "AutoHider/autowalker.h"
#include "Base/seekerhsparams.h"

//#ifdef DEBUG_SERVER_ON
#ifndef COUT_SHOW_ID
#define COUT_SHOW_ID << "[" << (_gameLog==NULL?-10:_gameLog->getID()) << "] "
#else
#define COUT_SHOW_ID
#endif


static const int MAXACTIONS         = 30000; //max int: 32767 (?)



/*!
 * \brief The HSTCPServer class The server which handles the connections to the clients.
 */
class HSTCPServer : public QTcpServer
{
    Q_OBJECT

public:

    //AG150120: depricated by message type 'stopServer
    //const static quint8 SERVER_STOP_SIGNAL = 255; //65535;

    //create server based on config
    HSTCPServer(HSServerConfig* config, HSGameLog* gameLog, QObject *parent=0);
    
    virtual ~HSTCPServer();

    GMap* getGMap();

    /*PlayerInfo* getHider();

    PlayerInfo* getSeeker();*/

    //! returns and updates the game status
    int checkStatus();


    //ag111201
    void setServer(HSServer* server);

    void setip(QString i);

    void setport(int p);

    //! close server, but first log and start new server if required
    void closeServer(int exitStatus, bool startNewServer);


protected:
    //! init log
    void initializeLogs();

    //! write step to log
    long writeStepToLog(); //const Pos* hiderPosWNoiseSent, const Pos* hiderPosWNoiseSent2);

    //! start another server
    void restartserver();

    //! start a new game
    void newGame();

    //! init players
    void initPlayers();

    //! send the (final) game parameters to the agents
    void writeGameParamsToStream(QDataStream& out, QByteArray &byteArray, ClientSocket* client); //PlayerInfo* recPlayer); // char playerType, PlayerInfo* p2Info, PlayerInfo *p3Info, bool sendNoisePos);

    //! send the parameters to all players: map + game params
    void sendParamsToPlayers();

    //! start new server instance
    bool startNewServerInstance();

    //! receive position
    void readInitPos(QDataStream& in, PlayerInfoServer* info);

    //AG150430
    //! receive positions of all players of 1 connection
    void readInitPoses(QDataStream& in);

    /*!
     * \brief readNewAction receive new action and pos for player
     * \param in
     * \param info
     * \return if the flag (to read) was set
     */
    bool readNewAction(QDataStream& in, PlayerInfoServer* info/*, double* beliefScore, double* seekerReward*/);

    //AG150504
    /*!
     * \brief readNewActions receive positions and actions for all players of client
     * \param in
     * \return wether on player's flag was already true
     */
    bool readNewActions(QDataStream& in);

    //! write opponent
    //void writeOppInfo(QTcpSocket* socket, PlayerInfo* infoToSend, QString text);

    //! write update to stream
    //! if there are 2 seekers then order is: hider, seeker, seeker2 (but not sending own)
    void writeUpdateToStream(QDataStream& out, QByteArray &byteArray, /*ClientSocket* client,*/ /*PlayerInfo* oppInfo*/
                             /*const Pos& pos,*/ std::vector<IDPos>& nextPosAutoWalkers/*,
                                bool sendPosWNoise, Pos* posWNoise=NULL, Pos* pos2=NULL, Pos* posWNoise2=NULL*/);

    //! send update (position and game state) to all players
    void sendUpdateToPlayers();

    //AG140605 TODO: should go somewhere else, maybe reuse with POCMP/HSSimulatorCont
    //AG150504: apply noise to hider only
    //! apply noise and/or false positive/negative prob. (TODO)
    Pos applyNoiseHider(Pos seekerPos, Pos hiderPos);

    //AG150504: apply noise to a pos
    Pos applyNoise(Pos pos);

    //! cehck if both players sent, and then send the updated positions
    void checkIfBothSentAndSend();

    //! call the randomhider
    void callRandhider(int hiderType, QString actionFile, int randomPosDistToBaseHider);

    //! create the auto walker
    void createAutoWalker(int autoWalkerType, int n);

    //! init auto walkers
    void initAutoWalkers();

    //! move auto walkers
    void moveAutoWalkers(std::vector<IDPos>& nextPosAutoWalkers);

    //AG150120
    //! read message sent by client and forward it to the client(s)
    void readAndForwardMessage(QDataStream& in, PlayerInfoServer& sender);

    //AG150202
    //! read the robot goal poses from the client (only multi seeker)
    void readRobotGoalPoses(QDataStream& in, ClientSocket* client);

    //! write the robot goals to the client (seekers) (only multi seeker)
    void sendRobotGoals();

    //AG150508
    //! read the highest belief poses and belief from the client (only multi seeker)
    void readRobotHB(QDataStream& in, ClientSocket* client);

    //! write the robot goals to the stream
    void writeRobotGoalsToStream(QDataStream &out, QByteArray &byteArray /*, PlayerInfo& p2Info*/);

    //! write the HB and beliefs to clients (only multi seeker)
    void sendRobotHB();

    //! write the robot goals to the stream
    void writeRobotHBToStream(QDataStream &out, QByteArray &byteArray);

    //AG150214
    //! read robot's chosen goal pose (can be different than next step)
    //AG150216: use sendaction (if 2seekers and isseeker)
    //void readRobotChosenGoalPose(QDataStream& in, PlayerInfo& sender);

    //! normalizes the useObsProb for all players
    void normalizeObsProb();

protected slots:
    //AG150430: ready read with socket as parameter
    //AG150506: changed to ClientSocket (to have more info)
    //! general readyRead handler for the different agents
    void agentReadyRead(ClientSocket* client); //, PlayerInfo &playerInfo, double *seekerBeliefScore = NULL, double* seekerReward = NULL);

private slots:
    //! handles new connections to the server
    void handleNewConnection();

    //! handles disconnection of the server
    void ondisconnect();

    //AG150513: added client
    //! handles client disconnection
    void clientDisconnected(ClientSocket* client);

    //! for timer (test/debug)
    void timerTestUpdate();

    //AG150513: added client
    //!in case of error with connection
    void acceptError(ClientSocket* client, QAbstractSocket::SocketError s);

    //! in case of error with server connection
    void acceptError(QAbstractSocket::SocketError s);


private:

    //! get name of map
    QString getMapName();

    //! init, send position
    void startGame();


    //! the map
    GMap* _gmap;

    //! hider info (this is a ref. to an object which also should be in _playerInfoVec)
    PlayerInfoServer* _hiderPlayer;
    //! seeker info (first)
    PlayerInfoServer* _seeker1Player;
    /*PlayerInfo _seeker;
    //AG141124
    //! seeker 2 info
    PlayerInfo _seeker2;*/

    //AG150427: multi agents seekers
    //! multi seekers
    std::vector<PlayerInfoServer*> _playerVec;


    /*//! hider socket
    QTcpSocket* _hiderSocket;
    //! seeker socket
    QTcpSocket* _seekerSocket;
    //AG141124
    QTcpSocket* _seeker2Socket;*/

    //!number of connections;
    unsigned int _numConn;

    //AG150430
    //! number of players
    unsigned int _numPlayers;
    //! number of connected players
    unsigned int _numPlayersWInitPos;

    //! Game status: 0-game on, 1-seeker wins, 2-hider wins, 3-tie game over.
    int _gameStatus;

    //! server
    HSServer* _server;

    //! ip of server
    QString _ip;
    //! port of server
    int _port;

    //! config file
    HSServerConfig* _config;

    //! game log
    HSGameLog* _gameLog;

    //! time when sent the last action
    QDateTime _sentLastActionTime;

    //AG140409: added score for belief of seeker //AG150430: now to playerinfo
    /*//! belief score seeker
    double _seekerBeliefScore, _seeker2BeliefScore;
    //! reward seeker
    double _seekerReward, _seeker2Reward;*/

    //AG140426
    //! a list of walker generators (persons/dynamic objects)
    std::vector<AutoWalker*> _walkerVector;

    //AG140506: instead of local vast, TODO: later
    //! params
    SeekerHSParams* _params;

    //AG140605 TODO: move to other place
    //! probability distribution, 0 to 1
    std::uniform_real_distribution<> _uniformProbDistr;
    //! noise distribution
    std::normal_distribution<double> _gausObsNoiseDistr;
    //! random device
    std::random_device _randomDevice;
    //! random generator
    std::mt19937 _randomGenerator;

    //! timer for alive testing
    QTimer _timerTest;
    unsigned long _timerTestC;

    //AG150430 - AG150518: use min number of players connected
    // start game when hider has been connected
    //bool _startGameAfterHiderConnected;

    //AG150430
    //! list of sockets
    std::vector<ClientSocket*> _clientSocketVec;



};


#endif // HSTCPServer_H
