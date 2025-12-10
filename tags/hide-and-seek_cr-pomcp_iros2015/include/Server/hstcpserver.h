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
#include "Server/playerinfo.h"

#include "Utils/generic.h" //ag

#include "Server/hsserverconfig.h"
#include "Server/hsgamelog.h"

#include "autowalker.h"
#include "seekerhsparams.h"

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

    PlayerInfo* getHider();

    PlayerInfo* getSeeker();

    //! returns and updates the game status
    int checkStatus();


    //ag111201
    void setServer(HSServer* server);

    void setip(QString i);

    void setport(int p);

    //! close server, but first log and start new server if required
    void closeServer(int exitStatus, bool startNewServer);


protected:


    int writeStepToLog(const Pos* hiderPosWNoiseSent, const Pos* hiderPosWNoiseSent2);

    void restartserver();

    void newGame();
    void initPlayers();
    void initializeLogs();

    //! send the (final) game parameters to the agents
    void writeGameParamsToStream(QDataStream& out, QByteArray &byteArray, char playerType, PlayerInfo* p2Info, PlayerInfo *p3Info, bool sendNoisePos);

    //! send the parameters to all players: map + game params
    void sendParamsToPlayers();

    //! start new server instance
    bool startNewServerInstance();

    //! receive position
    void readInitPos(QDataStream& in, PlayerInfo* info);


    //! receive position
    void readNewAction(QDataStream& in, PlayerInfo* info, double* beliefScore, double* seekerReward);

    //! write opponent
    //void writeOppInfo(QTcpSocket* socket, PlayerInfo* infoToSend, QString text);

    //! write update to stream
    //! if there are 2 seekers then order is: hider, seeker, seeker2 (but not sending own)
    void writeUpdateToStream(QDataStream& out, QByteArray &byteArray, /*PlayerInfo* oppInfo*/ const Pos& pos, std::vector<IDPos>& nextPosAutoWalkers,
                                bool sendPosWNoise, Pos* posWNoise=NULL, Pos* pos2=NULL, Pos* posWNoise2=NULL);

    //! send update (position and game state) to all players
    void sendUpdateToPlayers();

    //AG150605 TODO: should go somewhere else, maybe reuse with POCMP/HSSimulatorCont
    //! apply noise and/or false positive/negative prob.
    Pos applyNoise(Pos seekerPos, Pos hiderPos);

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

    //! general readyRead handler for the different agents
    void agentReadyRead(QTcpSocket* socket, PlayerInfo &playerInfo, double *seekerBeliefScore = NULL, double* seekerReward = NULL);

    //AG150120
    //! read message sent by client and forward it to the client(s)
    void readAndForwardMessage(QDataStream& in, PlayerInfo& sender);

    //AG150202
    //! read the robot goal poses from the client (only multi seeker)
    void readRobotGoalPoses(QDataStream& in, PlayerInfo& sender);

    //! write the robot goals to the client (seekers) (only multi seeker)
    void sendRobotGoals();

    //! write the robot goals to the stream
    void writeRobotGoalsToStream(QDataStream &out, QByteArray &byteArray, PlayerInfo& p2Info);

    //AG150214
    //! read robot's chosen goal pose (can be different than next step)
    //AG150216: use sendaction (if 2seekers and isseeker)
    //void readRobotChosenGoalPose(QDataStream& in, PlayerInfo& sender);


private slots:
    void handleNewConnection();
    void hiderReadyRead();
    void seekerReadyRead();
    void seeker2ReadyRead();
    void ondisconnect();
    void clientDisconnected();


private:

    //! get name of map
    QString getMapName();

    //! init, send position
    void startGame();


    //! the map
    GMap* _gmap;

    //! hider info
    PlayerInfo _hider;
    //! seeker info
    PlayerInfo _seeker;
    //AG141124
    //! seeker 2 info
    PlayerInfo _seeker2;

    //! hider socket
    QTcpSocket* _hiderSocket;
    //! seeker socket
    QTcpSocket* _seekerSocket;
    //AG141124
    QTcpSocket* _seeker2Socket;

    //!number of connections;
    int _numConn;

    //! Game status: 0-game on, 1-seeker wins, 2-hider wins, 3-tie game over.
    int _gameStatus;

    //! server
    HSServer* _server;

    // TODO!! USE also SeekerHSParams to store all this vars !!

    //! map id
    //int _mapID;

    //! opponent type
    //int _opp;

    //! ip of server
    QString _ip;
    //! port of server
    int _port;

    //! win distance for winning from hider
    //double _winDist;

    //! maximum number of actions
    //int _maxActions;

    //! config file
    HSServerConfig* _config;

    //! game log
    HSGameLog* _gameLog;

    //! time when sent the last action
    QDateTime _sentLastActionTime;

    //! dist to base for init pos
    //int _randomPosDistToBaseHider;

    //ag140130:
    //! use continuous space
    //bool _useContinuousPos;

    //AG140212
    //! game type
    //char _gameType;

    //AG140409: added score for belief of seeker
    //! belief score seeker
    double _seekerBeliefScore, _seeker2BeliefScore;
    //! reward seeker
    double _seekerReward, _seeker2Reward;

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
    std::mt19937 _randomGenerator; //(_randomDevice());

    //! timer for alive testing
    QTimer _timerTest;
    unsigned long _timerTestC;

private slots:
    //! for timer (test/debug)
    void timerTestUpdate();

    //!in case of error with connection
    void acceptError(QAbstractSocket::SocketError s);
};



#endif // HSTCPServer_H
