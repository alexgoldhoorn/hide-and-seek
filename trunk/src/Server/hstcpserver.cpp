#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>

#include <pthread.h>
#include <unistd.h> //ag130813: added for sleep in Ubuntu 13.04

#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include "Server/hstcpserver.h"
#include "Server/hsserverclientcommunication.h"

#include "Base/hsglobaldata.h"
#include "Base/hsconfig.h"
#include "Base/hsglobaldata.h"

#include "AutoHider/sfmwalkers.h"
#include "AutoHider/randomhider.h"
#include "AutoHider/randomwalker.h"
#include "AutoHider/fromlisthider.h"
#include "AutoHider/randomfixedhider.h"

#include "exceptions.h"


using namespace std;

HSTCPServer::HSTCPServer(HSServerConfig* config, HSGameLog* gameLog, QObject *parent)
    : QTcpServer(parent), _config(config), _gameLog(gameLog), _uniformProbDistr(0,1), _gausObsNoiseDistr(0,1), _randomGenerator(_randomDevice())
{
    DEBUG_SERVER_VERB(cout COUT_SHOW_ID << "Starting server..."<<endl;);

    _gmap = new GMap(_params);
    _gameStatus = HSGlobalData::GAME_STATE_NOT_STARTED;
    _numConn = 0;
    _hiderPlayer = NULL;
    _seeker1Player = NULL;
    _numPlayers = 0;

    _params = NULL;

    //connect to receiving new client connection
    connect(this, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    //AG150224: handle error
    connect(this, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));

    //AG150223: timer to debug issue of not closing servers and see when there are still servers running in the background
#ifdef SERVERTIMER_ENABLED
    connect(&_timerTest, SIGNAL(timeout()), this, SLOT(timerTestUpdate()));
    _timerTestC = 0;
    _timerTest.start(30000);
#endif
}

HSTCPServer::~HSTCPServer() {
    for(AutoWalker* walker: _walkerVector) {
        delete walker;
    }
    //AG150427
    for(PlayerInfoServer* info: _playerVec) {
        delete info;
    }
    //AG150430
    for(ClientSocket* socket: _clientSocketVec) {
        delete socket;
    }
}

QString getFilePath(QString path, QString file) {
    //AG140605: first test if file exists
    QFile fileTest(file);
    if (fileTest.exists()) {
        return file;
    } else {
        if (path.right(1).compare(QDir::separator())==0) {
            return path + file;
        } else {
            return path + QDir::separator() + file;
        }
    }
}

void HSTCPServer::handleNewConnection() {
    DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"New connection #"<<_numConn<<endl;);

    //get the next pending connection
    QTcpSocket* socket = this->nextPendingConnection();
    assert(socket!=NULL);
    DEBUG_SERVER(cout COUT_SHOW_ID <<"new connection request: "<<socket->socketDescriptor()<<endl;);

    //if the socket still open
    if(socket->isOpen()) {
        //wait to get data
        while (!socket->waitForReadyRead(6000)) {
            DEBUG_SERVER_VERB(cout<<"."<<flush;);
        }

        //read block size
        BLOCK_SIZE blockSize=0;
        QDataStream in(socket);
        in.setVersion(HSGlobalData::DATASTREAM_VERSION);


        if (blockSize == 0) {
            if (socket->bytesAvailable() < (int)sizeof(BLOCK_SIZE)) {
                DEBUG_SERVER(cout COUT_SHOW_ID <<"HSTCPServer::handleNewConnection: less bytes in the array than expected: size."<<endl;);
                socket->flush();
                return; //wait for more (?)
            }
            //block size
            in >> blockSize;
        }


        if (socket->bytesAvailable() < blockSize) {
            DEBUG_SERVER(cout COUT_SHOW_ID <<"HSTCPServer::handleNewConnection: less bytes in the array than expected 1. blocksize="
                         <<blockSize<<", available:"<<socket->bytesAvailable() <<endl;);
            socket->flush();
            return;
        }

        //AG140530: this is a manner of reading a big map that exceeds the buffer
        //          note: this has NOT been implemented yet for the receiving side,
        //          because it does work for 100x100 maps.
        /*QDataStream retrStream;
        quint64 bytesRead = 0;
        while (connection->bytesAvailable() + bytesRead < blockSize) {
            uint bytesAvailable = 1; // (uint)connection->bytesAvailable();

            DEBUG_SERVER(cout COUT_SHOW_ID <<"HSTCPServer::handleNewConnection: less bytes in the array than expected. blocksize="<<blockSize<<", available:"<<connection->bytesAvailable()<<", uint bytesav="<<bytesAvailable <<", read="<<bytesRead <<endl;);
            char* data = NULL;

            in.readBytes(data, bytesAvailable);

            retrStream.writeBytes(data, bytesAvailable);

            delete [] data;

            bytesRead = bytesAvailable + bytesRead;
        }        */

        //--- Start reading the message ---

        //AG150120: read the message type (although here shoulde be con request)
        MESSAGE_TYPE_SIZE messageType;
        in >> messageType;

        switch ((MessageType)messageType) {
            case MT_StopServer:
                cout  COUT_SHOW_ID << "Server stop requested"<<endl;
                closeServer(0,false);
                return; //AG150528: such that it does not continue
                break;
            case MT_ClientRequestConn:
                //all ok
                break;
            default:
                cout << "HSTCPServer::handleNewConnection: ERROR - unknown message type ("<<(int)messageType
                     <<"), expected ClientRequestConn or ServerStop"<<endl;
                return;
        }

        //start reading
        quint8 gameType, useContinuous;
        qint8 myType;
        quint16 mapID, randomPosDistToBaseHider, autoWalkerN, numSteps;
        qint16 oppType, autoWalkerType;
        QString username,expName;
        quint8 numPlayersInClient; //AG150430
        QString actionFile ="";//ag120903
        QString autoWalkerPosFile = "";
        quint8 numPlayersReq;
        bool stopAtWin;

        //AG140212: game type
        in >> gameType;

        //check connection number
        //AG141124: for find-and-follow 2 rob allow an extra player
        //AG150427: allow multiple robots

        if (_numConn >= HSGlobalData::MAX_NUM_PLAYERS || (_params!=NULL && _numConn>=_params->maxNumberOfPlayers())) { //AG150430: use num of players

            cout COUT_SHOW_ID <<"ERROR: maximum number of connections already reached: "<<_numConn<<endl;
            return;
        } else if(_gameStatus==HSGlobalData::GAME_STATE_RUNNING)  {
            cout COUT_SHOW_ID <<"ERROR: The game is on with different players. Try again later."<<endl;
            return;
        } else if(_gameStatus > HSGlobalData::GAME_STATE_RUNNING)  { //the previous game is over...
            cout COUT_SHOW_ID <<"Previous game is over. Starting new game."<<endl;
            newGame();
            return;
        }

        //read map id
        in >> mapID;

        //ag130723: receive map from client
        if (mapID==HSGlobalData::MAP_PASSED_BY_NETWORK) {
            //cout << "LOADING MAP FROM STREAM"<<endl;
            _gmap->readMapFromStream(in);
            DEBUG_SERVER_VERB(cout COUT_SHOW_ID << "Map passed by player."<<endl;);
        }

        //AG140531: get map distance matrix file
        QString mapDistMatFileStr;
        in >> mapDistMatFileStr;

        //ag140130: continuous or not
        in >> useContinuous;

        double winDist = 0;
        //ag130404: first read win distance
        in >> winDist;

        //ag140506: obstacle distance
        double minObstDist = 0;
        in >> minObstDist;

        //read opponent type
        in >> oppType;

        //AG120903: opponent type, if is list, expecting name of action list (file)
        if ((int)oppType==HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST || (int)oppType==HSGlobalData::OPPONENT_TYPE_HIDER_FILE) {
            in >> actionFile;
            DEBUG_SERVER_VERB(cout COUT_SHOW_ID << "Action/Pos file: "<<actionFile.toStdString()<<endl;);
        }

        //random init pos distance to base for hider (if used)
        in >> randomPosDistToBaseHider;

        //AG140426: auto walker type and count
        in >> autoWalkerType;
        in >> autoWalkerN;
        //AG140605: get auto walker pos file
        if ((int)autoWalkerType==HSGlobalData::AUTOWALKER_FILE) {
            in >> autoWalkerPosFile;
            DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"AutoWalker Pos file: "<<autoWalkerPosFile.toStdString()<<endl;);
        }

        //AG140608: solver type
        quint8 solverType;
        in >> solverType;

        //AG140520: stop after win, and num. of iterations to do
        in >> stopAtWin;
        in >> numSteps;

        //get player details
        in >> myType; //1:seeker, 0:hider

        //AG150430: number of players in this client
        in >> numPlayersInClient;
        assert(numPlayersInClient>0);

        in >> username;

        //AG160414
        in >> expName;

        //AG140531: sim. obs. noise std.dev.
        double simObsNoiseStd;
        in >> simObsNoiseStd;
        double simObsFalseNegProb,simObsFalsePosProb;
        in >> simObsFalseNegProb;
        in >> simObsFalsePosProb;

        //AG140531: meta info
        QString metaInfo;
        in >> metaInfo;
        //AG140601
        QString comments;
        in >> comments;

        //AG150518: number of players required
        in >> numPlayersReq;

        //AG150519: probability of using this user's obs
        double usePlayerObsP;
        in >> usePlayerObsP;

        //AG151118: user can see all
        bool seesAll;
        in >> seesAll;

        //AG160216: sim max visib dist
        bool simNotVisibDist;
        in >> simNotVisibDist;

        //AG160505: use dyn. obst for v.
        bool useDynObstForV;
        in >> useDynObstForV;

        //message has been read
        //flush the socket
        socket->flush();

        //--- create player struct ---

        //AG141124: generalized connections
        if(_numConn==0) {
            //init players at the first connectioon
            initPlayers();
        }
        DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"Connection established with player "<<(_numConn+1)<<endl;); //" - ";);
        //cout << ">- seeker.set="<<_seeker.set<<", hider.set="<<_hider.set<<", seeker2.set="<<_seeker2.set<<"---"<<endl;

        //AG150430: create player info object
        PlayerInfoServer* player = new PlayerInfoServer();
        player->socket = socket;
        //player->set = true;
        player->username = username; // setUserName(username);
        player->metaInfo = metaInfo;
        player->comments = comments;
        player->useObsProb = usePlayerObsP;
        player->seesAll =  seesAll;
        //_params = &_hider.params;
        //curParams = &(player->params);

        //make signal-slot connections to the client socket, which is a wrapper of the socket
        ClientSocket* clientSocket = new ClientSocket(socket);
        _clientSocketVec.push_back(clientSocket);
        //connect the signals of the clientSocket to this object's slots
        connect(clientSocket, SIGNAL(socketReadyRead(ClientSocket*)), this, SLOT(agentReadyRead(ClientSocket*)));
        //AG150223: handle disconnection
        connect(clientSocket, SIGNAL(socketDisconnected(ClientSocket*)), this, SLOT(clientDisconnected(ClientSocket*)));
        //AG150224: handle error
        connect(clientSocket, SIGNAL(socketError(ClientSocket*,QAbstractSocket::SocketError)), this, SLOT(acceptError(ClientSocket*,QAbstractSocket::SocketError)));

        //check player type
        switch((HSGlobalData::Player)myType) {
        case HSGlobalData::P_Seeker:
            player->playerType=HSGlobalData::P_Seeker;
            if (_seeker1Player==NULL) {
                _seeker1Player = player;
            }
            break;
        case HSGlobalData::P_Hider:
            if (_hiderPlayer != NULL) {
                cout << "ERROR: there is already one hider"<<endl;
                return;
            }
            player->playerType=HSGlobalData::P_Hider;
            _hiderPlayer = player;
            break;
        default:
            cout << "ERROR: unknown player type: "<<myType<<endl;
            //exit(1);
            closeServer(1,false);
            return; //AG150528: such that it does not continue
            break;
        }

        //add the other players of the same client
        player->id = (int)_playerVec.size();
        _playerVec.push_back(player);
        clientSocket->addPlayer(player);
        //add other players
        for(quint8 i = 1; i<numPlayersInClient; i++) {
            //create player info (copy from first)
            PlayerInfoServer* otherPlayer = new PlayerInfoServer(*player);
            //set id
            otherPlayer->id = (int)_playerVec.size();
            //change name (add id to original name)
            otherPlayer->username = otherPlayer->username + QString::number(otherPlayer->id);
            //add player to list of players
            _playerVec.push_back(otherPlayer);
            //and add to connection
            clientSocket->addPlayer(otherPlayer);
        }

        //AG150528: here was check for starting game..but now put after setting of _params

        DEBUG_SERVER(
        cout <<"-------[Connection #"<<_numConn<<"]-------"<<endl
             <<"Game type: "<<(int)gameType<<endl
             <<"Exp. name: "<<expName.toStdString()<<endl
             <<"Name: "<<username.toStdString()<<endl
             <<"#Players: "<<(int)numPlayersInClient<<" - "<<clientSocket->getNumSeekers()<<" seekers, "
                            <<(clientSocket->getNumPlayers()-clientSocket->getNumSeekers())<<" hider(s)"<<endl
             <<"Comments: "<<comments.toStdString()<<endl
             <<"Meta info: "<<metaInfo.toStdString()<<endl
             <<"Map ID: "<<mapID<<endl
             <<"Use continuous: "<<(int)useContinuous<<endl
             <<"Win dist.: "<<winDist<<endl
             <<"Min. obst. dist.="<<minObstDist<<endl
             <<"Opp. type: "<<oppType<<endl
             <<"Random Pos dist to base: "<<randomPosDistToBaseHider<<endl
             <<"Auto Walker Type="<<autoWalkerType<<"#"<<autoWalkerN<<" file:"<<autoWalkerPosFile.toStdString()<<endl
             <<"Noise: std="<<simObsNoiseStd<<"; false neg. prob.="<<simObsFalseNegProb<<"; false pos. prob="<<simObsFalsePosProb<<endl
             <<"Solver type: "<<(int)solverType<<endl
             //<<"is global params="<<(_params==curParams)<<endl
             <<"stop after win="<<stopAtWin<<", after "<<numSteps<<" steps"<<endl
             //<<"start game after hider conn: "<<startGameAH<<" (global: "<<_startGameAfterHiderConnected<<")"<<endl
             <<"req. number of clients: "<<(int)numPlayersReq<<endl
             <<"Use player obs. prob="<<usePlayerObsP<<endl
             <<"Sees all="<<seesAll<<endl
             <<"Sim. max. visib. dist.="<<simNotVisibDist<<endl
             <<"Use dyn. obst. for visib check="<<useDynObstForV<<endl
             <<"-------------"<<endl;
        );

        SeekerHSParams* curParams = &(player->params);
        //set current params in struct
        curParams->gameType = gameType;
        curParams->mapID = (int)mapID;
        curParams->solverType = (char)solverType;
        curParams->opponentType = (int)oppType;
        curParams->oppActionFile = actionFile.toStdString();
        curParams->useContinuousPos = (useContinuous==(quint8)1);
        curParams->winDist = winDist;
        curParams->minDistToObstacle = minObstDist;
        curParams->randomPosDistToBase = randomPosDistToBaseHider;
        curParams->autoWalkerType = autoWalkerType;
        curParams->autoWalkerN = autoWalkerN;
        curParams->autoWalkerPosFile = autoWalkerPosFile.toStdString();
        curParams->stopAfterNumSteps = numSteps;
        curParams->stopAtWin = stopAtWin;
        curParams->simObsNoiseStd = simObsNoiseStd;
        curParams->simObsFalseNegProb = simObsFalseNegProb;
        curParams->simObsFalsePosProb = simObsFalsePosProb;
        curParams->mapDistanceMatrixFile = mapDistMatFileStr.toStdString();
        curParams->multiSeekerOwnObsChooseProb = usePlayerObsP;
        curParams->numPlayersReq = numPlayersReq;
        curParams->simNotVisibDist = simNotVisibDist;
        curParams->expName = expName.toStdString();
        curParams->useDynObstForVisibCheck = useDynObstForV;

        //for first connection
        if(_numConn==0) {
            //set 1st connection's player params struct to be the global
            //i.e. all params are set by the 1st player
            _params = curParams;

            //AG140509: here the params are chosen, set it for GMap
            _gmap->setParams(_params);
        }

        //AG150430: check if there will be too many players
        _numPlayers += numPlayersInClient;
        if (_numPlayers > _params->maxNumberOfPlayers()) {
            cout << "ERROR: the maximum number of players "<<_params->maxNumberOfPlayers()<<" has been exceeded ("<<_numPlayers<<")"<<endl;
            return;
        }

        //check/warning
        if (_numPlayers>_params->numPlayersReq) {
            cout COUT_SHOW_ID <<"WARNING the required number of players ("<<_params->numPlayersReq<<") has been exceeded: "<<_numPlayers<<endl;
        } else if (_numPlayers==_params->numPlayersReq && _hiderPlayer==NULL) {
            cout COUT_SHOW_ID <<"WARNING the required number of players ("<<_params->numPlayersReq<<") has been reached, but no hider has connected yet"<<endl;
        }

        // for the first connection
        if (_numConn==0) {
            //AG NOTE: map is fixed here, since the first player decides the map!

            //AG140608: set params such that it gets the rigth params (continuous/not..)
            //_gmap->setParams(curParams);

            DEBUG_SERVER_VERB(cout COUT_SHOW_ID << "Game type: "<<(int)_params->gameType<<endl;);

            //AG140426: create auto walker
            createAutoWalker(autoWalkerType, autoWalkerN);

            DEBUG_SERVER_VERB(cout COUT_SHOW_ID << "Win distance: "<<_params->winDist<<endl;);

            //if(_opp != HSGlobalData::OPPONENT_TYPE_HUMAN) { call hider??

            if (mapID!=HSGlobalData::MAP_PASSED_BY_NETWORK) { //AG130723
                //read map if not set before
                QString mapFile = getFilePath(_config->getMapPath(), HSGlobalData::MAPS[_params->mapID]);
                DEBUG_SERVER(cout << "Loading map file: " << mapFile.toStdString()<<endl;);

                _gmap->readMapFile(mapFile.toStdString());
            }

            //continuous or not
            DEBUG_SERVER(cout COUT_SHOW_ID << "Space and movements: " << (_params->useContinuousPos?"continuous":"discrete")<<endl;);
            _gmap->setUsingContActions(_params->useContinuousPos);

            //AG140605: map fixed
            _gmap->setMapFixed();

            //AG140601: reading distance matrix
            if (!_params->mapDistanceMatrixFile.empty()) {
                //if file doesn't exist, it generates an error
                _gmap->readDistanceMatrixToFile(_params->mapDistanceMatrixFile);
            }

            //ag120511: set max actions dependend on map size:
            //max1: (#rows*#cols)/2
            //max2: (#rows+#cols)*1.5   <- implemented here
            //_params->maxNumActions = (int)((_gmap->colCount()+_gmap->rowCount())*HSGlobalData::MAX_ACT_MULT_FACTOR);
            //_params->maxNumActions = min(_params->maxNumActions,MAXACTIONS);

            if (!stopAtWin || numSteps==0) {
                //calculate max number of actions
                switch (_config->getMaxActCalcType()) {
                case HSServerConfig::MAX_ACT_CALC_TYPE1:
                    _params->maxNumActions = (int)((_gmap->colCount()+_gmap->rowCount())*2); //HSGlobalData::MAX_ACT_MULT_FACTOR);
                    break;
                case HSServerConfig::MAX_ACT_CALC_TYPE2:
                    _params->maxNumActions = (_gmap->colCount()*_gmap->rowCount());
                    break;
                case HSServerConfig::MAX_ACT_CALC_FIXED:
                    _params->maxNumActions = _config->getMaxActionsFixed();
                    break;
                default:
                    cout << "ERROR: unknown max action calculation type!"<<endl;
                    closeServer(-1,false);
                    return; //AG150528: such that it does not continue
                    break;
                }

                if (_params->stopAfterNumSteps>0 && _params->stopAfterNumSteps>_params->maxNumActions) {
                    cout << "WARNING: number of max. actions ("<<_params->maxNumActions<<") is lower than the steps after which the game should stop ("
                            << _params->stopAfterNumSteps<<"), so reset to this."<<endl;
                    _params->maxNumActions = _params->stopAfterNumSteps;
                }
            } else {
                _params->maxNumActions = numSteps;
            }

            DEBUG_SERVER_VERB(_gmap->printMap(););
            DEBUG_SERVER_VERB(cout << "maximum actions: "<<_params->maxNumActions<<endl;);

            //start opponent
            if(_params->opponentType != HSGlobalData::OPPONENT_TYPE_HUMAN) {
               //AG120903
               if (_params->opponentType==HSGlobalData::OPPONENT_TYPE_SEEKER) {
                   //callSeekroBot();
                   cout<<"Error: automated seeker type not supported as opponent"<<endl;
                   ondisconnect(); //disconnect
               } else {
                   callRandhider(_params->opponentType,actionFile,(int)randomPosDistToBaseHider);
               }
            }


        } //if numConn==0

        //AG150518: starting criterium: there is a hider, and a min. amount of players
        //check if starting
        bool startGame = _hiderPlayer!=NULL && _numPlayers>=_params->numPlayersReq; /*(
                                    (_params->numPlayersReq==0 && _numPlayers>=2)
                                || (_params->numPlayersReq>0 && _numPlayers>=_params->numPlayersReq) );*/

        if (startGame) {
            //close this server and start new one
            disconnect(this, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
            this->close();

            //ag120523:
            startNewServerInstance();
        }

        /*//ag141125: if it is the last connection... start the game
        if ((_params->gameHas2Seekers() && _numConn == 2) ||
                (!_params->gameHas2Seekers() && _numConn == 1)) {*/
        if (startGame) { //AG150430: starting game
            //we have two (or 3) connected players

            _gameStatus = HSGlobalData::GAME_STATE_RUNNING;
            //set false, wait for initial position of both
            //_seeker.set = _seeker2.set = _hiderPlayer->set = false;

            DEBUG_SERVER(cout COUT_SHOW_ID << "---STARTING GAME---"<<endl;);

            DEBUG_SERVER_VERB(
                /*cout COUT_SHOW_ID << "Starting the game, seeker "<<_seeker.params.userName;
                if (_params->gameHas2Seekers())
                    cout << "& "<<_seeker2.params.userName;
                cout<<" vs. hider "<<_hiderPlayer.params.userName<<endl;*/
                //AG150430: list all players
                cout COUT_SHOW_ID << "Players: ";
                for(PlayerInfoServer* info : _playerVec) cout << info->toString()<<" ";
                cout<<endl;
            );

            //send map to players
            sendParamsToPlayers();
        }

        //increase connection number
        _numConn++;
    } else {
        DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"Connection closed beforehand by client."<<endl;);
    }

    //cout << "<- seeker.set="<<_seeker.set<<", hider.set="<<_hiderPlayer.set<<", seeker2.set="<<_seeker2.set<<"---"<<endl;
}


void HSTCPServer::readInitPoses(QDataStream& in) {
    quint8 numPlayers,id;
    in >> numPlayers;
    for(quint8 i=0; i<numPlayers; i++) {
        in >> id;
        readInitPos(in, _playerVec[id]);
    }
}

void HSTCPServer::readInitPos(QDataStream& in, PlayerInfoServer *player) {
    if (player->initPosSet) {
        throw CException(_HERE_, "a player had already the init pos set");
    }
    //AG121112: set prev pos
    player->previousPos = player->currentPos;
    player->currentPos.readPosFromStream(in, _params->useContinuousPos);

    player->initPosSet = player->posRead = true;
    player->timestamp = QDateTime::currentDateTime();
    _numPlayersWInitPos++;

    DEBUG_SERVER(cout COUT_SHOW_ID << " init pos "<<player->toString()<<": "<<player->currentPos.toString()<<endl;);
}

bool HSTCPServer::readNewActions(QDataStream &in) {
    quint8 numPlayers,id;
    bool flagWasSet = false;
    in >> numPlayers;
    for(quint8 i=0; i<numPlayers; i++) {
        //read id of player and read its action
        in >> id;
        if (id>=_playerVec.size()) throw CException(_HERE_,"the passed player id is out of bounds: "+QString::number(id).toStdString());
        //read action
        if (readNewAction(in, _playerVec[id]))
            flagWasSet = true;
    }

    return flagWasSet;
}

bool HSTCPServer::readNewAction(QDataStream& in, PlayerInfoServer *player) {
    assert(player!=NULL);
    //retrieve info
    qint16 action;

    in >> action;

    Pos pos;
    pos.readPosFromStream(in, _params->useContinuousPos);

    //AG140409: belief score (NOTE: this is of the previous position and belief)
    in >> player->seekerBeliefScore;
    //AG140612: seeker reward
    in >> player->seekerReward;

    //AG150216: read multi chosen pos
    player->chosenGoalPos.readPosFromStream(in,_params->useContinuousPos);

    if (!player->flag) {
        //not yet received for this turn
        player->lastAction = (int)action; //action[_hider.numa] = (int)a;
        player->numberActions++;
        player->flag = true;
        //AG121112 set previous
        player->previousPos = player->currentPos;
        //set hider current
        player->currentPos = pos;
        player->timestamp = QDateTime::currentDateTime();
        //socket->flush();

        //flag was not set, so not read before
        return false;

    } else {
        //already received, so wait for next player to finish
        DEBUG_SERVER(cout  COUT_SHOW_ID << "HSTCPServer::readNewAction: WARNING: " << player->toString() << " Ignoring - wait for opponent's' turn"<<endl;);

        //flag was set, so already read before
        return true;
    }
}

void HSTCPServer::writeUpdateToStream(QDataStream &out, QByteArray& byteArray, vector<IDPos>& nextPosAutoWalkers) {

    out.setVersion(HSGlobalData::DATASTREAM_VERSION);

    DEBUG_SERVER_VERB(cout COUT_SHOW_ID << "Sending "<<": "<<endl;);
    DEBUG_SERVER(cout << "{i"<<_playerVec[0]->numberActions<<"}";);

    out << (BLOCK_SIZE)0; //block size
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_Update;
    out << (quint16)1; //validation byte
    out << (quint16)_gameStatus; //game status

    //AG150504: send of all players to all players
    out << (quint8)_numPlayers;
    assert(_numPlayers==_playerVec.size());
    DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"num players: "<<(int)_numPlayers<<endl;);
    for(PlayerInfoServer* player : _playerVec) {
        assert(player->id>=0);
        out << (quint8) player->id;
        player->currentPos.writePostoStream(out,_params->useContinuousPos);
        player->currentPosWNoise.writePostoStream(out,_params->useContinuousPos);

        DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"- "<<player->id<<" curpos: "<<player->currentPos.toString()
                          <<" w noise:"<<player->currentPosWNoise.toString()<<endl;);
    }

    //AG150711: there should be 1 hider, rest seekers
    out << (quint8)(_numPlayers-1);

    //for debug
    DEBUG_SERVER(quint8 numSeekersSent = 0;);
    DEBUG_SERVER_VERB(cout COUT_SHOW_IDcout<<"hider poses:"<<endl;);

    for(PlayerInfoServer* player : _playerVec) { //AG150711: all players should be sent
        if (player->isSeeker()) {
            //send noisy current pos
            assert(player->id>=0); //could be -1
            out << (quint8) player->id;

            player->hiderObsPosWNoise.writePostoStream(out,_params->useContinuousPos);
            //AG150611: trust prob of obs; AG150708: changed to useObsProb
            assert(player->useObsProb>=0);
            out << player->useObsProb;

            //AG160503: send visible autowalkers
            out << (quint16)player->dynObsVisibleVec.size();
            for(IDPos& pos : player->dynObsVisibleVec) {
                pos.writePostoStream(out, _params->useContinuousPos);
            }
            DEBUG_SERVER_VERB(
                        cout COUT_SHOW_ID <<"- "<<player->id<<" hiderobs: "<<player->hiderObsPosWNoise.toString()
                                                <<" #visib. auddtow: "<<player->dynObsVisibleVec.size()<<endl);
            DEBUG_SERVER( numSeekersSent++;);
        }
    }

    //debug test:
    DEBUG_SERVER(
        if (numSeekersSent!=_numPlayers-1) {
            throw CException(_HERE_, "Number of sent seekers ("+QString::number(numSeekersSent).toStdString()
                                +") is not equal to the number of seeker ("+ QString::number(_numPlayers-1).toStdString()+")");
        }
    );

    //AG140426: send auto walkers pos
    //first count
    out << (quint16)nextPosAutoWalkers.size();
    //now all pos
    for(IDPos& pos: nextPosAutoWalkers) {
        pos.writePostoStream(out, _params->useContinuousPos);
    }

    out.device()->seek(0);
    out << (BLOCK_SIZE)(byteArray.size() - sizeof(BLOCK_SIZE));
}

void HSTCPServer::sendUpdateToPlayers() {
    //AG140505: move the autowalkers if present
    //NOTE: maybe should be moved to another place, remember that now they'll use the NEW positions

    //at the server we should always have the hider pos
    assert(_hiderPlayer->currentPos.isSet());

    //AG160428: moved autowalker update here, because they also move in same step
    //update auto walker poses
    vector<IDPos> nextPosAutoWalkers;
    moveAutoWalkers(nextPosAutoWalkers);
    //AG160428: set for map, such that it takes into account autowalkers
    _gmap->setDynamicObstacles(nextPosAutoWalkers);


    //add noise to player position (represents the player's error in own localization, which is sent to all)
    for(PlayerInfoServer* player : _playerVec) {
        //add noise to own pos
        player->currentPosWNoise = applyNoise(player->currentPos);
        //set (noisy) obs of hider by a seeker
        if (player->isSeeker()) {
            if (player->seesAll) { //AG151118: if player sees all pass always correct pos
                player->hiderObsPosWNoise = _hiderPlayer->currentPos;
                player->dynObsVisibleVec = nextPosAutoWalkers;
            } else {
                //add noise to hider
                player->hiderObsPosWNoise = applyNoiseHider(player->currentPos, _hiderPlayer->currentPos);
                //now check which dyn. obst. are visible to the seeker
                player->dynObsVisibleVec.clear();
                for(IDPos pos : nextPosAutoWalkers) {
                    //add noise to auto walker pos, taking into account location of seeker
                    //NOTE: the auto walker is treated as hider
                    pos.set(applyNoiseHider(player->currentPos, pos, false));
                    //add if it is visible
                    if (pos.isSet())
                        player->dynObsVisibleVec.push_back(pos);
                }
            }
        }

        player->flag = false;
    }

    //AG160428: moved autowalkers move from here to up

    //AG160602: set time sent
    /*QDateTime sentTime = QDateTime::currentDateTime();
    for(PlayerInfoServer* player : _playerVec) {
       player->obsSentTimeStamp = sentTime;
    }*/

    //AG150711: send all players to all clients
    //create data stream
    QByteArray blockToClient;
    QDataStream outClient(&blockToClient, QIODevice::WriteOnly);
    //fill data block
    writeUpdateToStream(outClient, blockToClient, nextPosAutoWalkers);
    for(ClientSocket* client : _clientSocketVec) {
        //send data over socket
        client->getSocket()->write(blockToClient);
        client->getSocket()->flush();
    }

    //ag150717: here the actions have been sent
    _sentLastActionTime = QDateTime::currentDateTime();

    //log the actions in the files.
    writeStepToLog();
}

Pos HSTCPServer::applyNoiseHider(Pos seekerPos, Pos hiderPos, bool useDynObst, bool asDynObst) {
    DEBUG_SERVER(cout<<"HSTCPServer::applyNoiseHider: Adding noise to "<<hiderPos.toString()<<": ";);
    assert(seekerPos.isSet());
    assert(hiderPos.isSet());

    //AG150204: DO take into account dynamical obstacles, when they are obstructing..it should not be visible
    bool isVisib = _gmap->isVisible(seekerPos,hiderPos,useDynObst,_params->simNotVisibDist);
    Pos pos(hiderPos);

    if (isVisib) {
        //AG160124:. calculate probability of not seeing based on distance
        if (_config->simNotVisibDist()) {
            //TODO: could have been passed from client OR in config, since they could be different
            //AG160124: vars for visibility - TODO: global
            double y0, x0, k0;

            //AG160504: set params based on hider or dyn. obst.
            if (asDynObst) {
                y0 = SeekerHSParams::SIM_NOT_VISIB_DIST_DEFAULT_DO_Y0;
                x0 = SeekerHSParams::SIM_NOT_VISIB_DIST_DEFAULT_DO_X0;
                k0 = SeekerHSParams::SIM_NOT_VISIB_DIST_DEFAULT_DO_K0;
            } else {
                y0 = SeekerHSParams::SIM_NOT_VISIB_DIST_DEFAULT_Y0;
                x0 = SeekerHSParams::SIM_NOT_VISIB_DIST_DEFAULT_X0;
                k0 = SeekerHSParams::SIM_NOT_VISIB_DIST_DEFAULT_K0;
            }

            //use distance to calculate the probability
            double d = seekerPos.distanceEuc(hiderPos);
            double pVisib = y0;
            if (d > x0)
                pVisib += k0*(d-x0);

            if (pVisib>0) {
                //probabililty of being seen depends on distance
                double p = _uniformProbDistr(_randomGenerator);
                if (p>pVisib) {
                    DEBUG_SERVER(cout<<"not visible due to distance prob. (p="<<pVisib<<")";);
                    //set false negative
                    isVisib = false;
                }
            } else {
                // too far, so not visible
                isVisib = false;
            }

        }

        if (isVisib && _params->simObsFalseNegProb>0) {
            //TODO:THIS PROBABLY WON'T WORK, BECAUSE WE EXPECT A CORRECT POS IN SEEKER(?)

            //check whether to add a false negative
            double p = _uniformProbDistr(_randomGenerator);
            if (p<=_params->simObsFalseNegProb) {
                DEBUG_SERVER(cout<<"false neg.";);
                //set false negative
                isVisib = false;
            }
        }
        //TODO: missing 'incorPosProb'

        if (_params->simObsFalsePosProb>0) {
            //AG140514: check to add incorrect reading
            //NOTE: now we give preference to false negative ..
            double p = _uniformProbDistr(_randomGenerator);
            if (p<=_params->simObsFalsePosProb) {
                DEBUG_SERVER(cout<<"false pos.";);
                pos = _gmap->genRandomPos();
                isVisib = true;
            }
        }

        //AG160127: set visibility
        if (isVisib) {
            //AG150511: use the function applyNoise directly
            pos = applyNoise(pos);
        } else {
            //clear pos
            pos.clear();
        }
    } else {
        pos.clear();
        DEBUG_SERVER(cout<<" (not visib.)";);
    }

    DEBUG_SERVER(cout <<" -> "<<pos.toString()<<endl;);
    return pos;
}

Pos HSTCPServer::applyNoise(Pos pos) {
    DEBUG_SERVER(cout<<"HSTCPServer::applyNoise: Adding noise to "<<pos.toString()<<": ";);
    //assert(pos.isSet());

    if (pos.isSet()) {
        assert(_gmap->isPosInMap(pos) && !_gmap->isObstacle(pos));
        //try to add noise
        uint c=0;
        Pos noisyPos;
        do {
            noisyPos.set(pos.rowDouble() + _gausObsNoiseDistr(_randomGenerator),
                    pos.colDouble() + _gausObsNoiseDistr(_randomGenerator) );
            c++;
            if (c>=100) {
                cout << "HSTCPServer::applyNoise: WARNING: no valid noise found to get a correct pos, returning original"<<endl;
                noisyPos = pos;
            }
        } while(!_gmap->isPosInMap(noisyPos) || _gmap->isObstacle(noisyPos));
        pos = noisyPos;
    }

    DEBUG_SERVER(cout <<" -> "<<pos.toString()<<endl;);
    return pos;
}

//AG150112: check if ALL are ready and send
void HSTCPServer::checkIfBothSentAndSend() {
    //bool use2Robots = (_params->gameHas2Seekers());

    //AG150506: check if all have the positions ready
    //AG150511: also check HB poses
    bool allHaveMultiPosesReady = false;
    bool allHaveHBPosesReady = false;
    if (_params->seekerSendsMultiplePoses()) {
        allHaveMultiPosesReady = true;
        allHaveHBPosesReady = true;

        //check if either all have Multi poses or HB poses ready
        for(PlayerInfoServer* player : _playerVec) {
            //AG150710: only seekers
            if (player->isSeeker()) {
                //goal poses
                if (!player->multiHasGoalPoses) {
                    allHaveMultiPosesReady = false;
                }
                //hb poses
                if (!player->multiHasHBPoses) {
                    allHaveHBPosesReady = false;
                }
                //check if stop
                if (!allHaveHBPosesReady && !allHaveMultiPosesReady) {
                    break;
                }
            }
        }

        //shouldn't be true both (different types of solvers)
        assert(!(allHaveHBPosesReady && allHaveMultiPosesReady));

        //ready so send robot poses and beliefs
        if (allHaveMultiPosesReady) sendRobotGoals();

        //AG150511: send HB points and beliefs
        if (allHaveHBPosesReady) sendRobotHB();
    }

    //AG150506: check if we need to send update send update
    if (!allHaveHBPosesReady && !allHaveMultiPosesReady) {
        //check if ready to send observation
        bool allHaveObsReady = true;
        for(PlayerInfoServer* player : _playerVec) {
            if (!player->flag) {
                allHaveObsReady = false;
                break;
            }
        }

        if (allHaveObsReady) {
            //check if number of actions are consistent
            bool numActOk = true;
            for(PlayerInfoServer* player : _playerVec) {
                if (player->numberActions!=_hiderPlayer->numberActions) {
                    numActOk = false;
                    cout COUT_SHOW_ID << "HSTCPServer::checkIfBothSentAndSend: ERROR number of sent actions is not consistent, for hider: "<<
                                         _hiderPlayer->numberActions<<", "<<player->toString()<<": "<<player->numberActions<<endl;
                    break;
                }
            }

            if (numActOk) {
                //check if 'game' is still running
                _gameStatus = checkStatus();

                if (_gameStatus > HSGlobalData::GAME_STATE_RUNNING && _gameLog!=NULL) {
                    //AG120531: stop game
                    _gameLog->stopGame();
                }

                //send pos to both players
                sendUpdateToPlayers();

                if(_gameStatus > HSGlobalData::GAME_STATE_RUNNING) {
                    //disconnect both users and start over if wanted
                    newGame();
                }
            }
        }

    }
}

//AG140112: now a readyRead function for each connection,
//          for more agents could be 1, whereby the agent sends it's id/name
//          and to be more secure it could send a by the server generated ID
//AG150112: made a general function for all agents
void HSTCPServer::agentReadyRead(ClientSocket* client) { //, PlayerInfo &playerInfo, double *seekerBeliefScore, double* seekerReward) {
    BLOCK_SIZE blockSize=0;
    QTcpSocket* socket = client->getSocket();
    QDataStream in(socket);
    in.setVersion(HSGlobalData::DATASTREAM_VERSION);

    if (blockSize == 0) {
        if (socket->bytesAvailable() < (int)sizeof(BLOCK_SIZE)) {
            DEBUG_SERVER(cout COUT_SHOW_ID << " HSTCPServer::readInitPos: WARNING: less bytes in the array than expected for "<<
                         client->toString()<<": block size."<<endl;);
            socket->flush();
            return;
        }
        in >> blockSize;
    }

    if (socket->bytesAvailable() < blockSize) {
        DEBUG_SERVER(cout COUT_SHOW_ID << "HSTCPServer::readInitPos: WARNING: less bytes in the array than expected for "<<
                     client->toString()<<": blocksize="<<blockSize<<", available:"<<socket->bytesAvailable() <<endl;);
        socket->flush();
        return;
    }

    //socket->flush();
    //AG150120: message type
    MESSAGE_TYPE_SIZE messageType;
    in >> messageType;

    DEBUG_SERVER(cout COUT_SHOW_ID <<" HSTCPServer::agentReadyRead "<< client->toString()<<endl;);

    //check if game still running
    if(_gameStatus > HSGlobalData::GAME_STATE_RUNNING) {
        //the game is over so reset the game
        cout COUT_SHOW_ID  << "WARNING: game already ended, but still receving from client"<<endl;

        QByteArray buf = socket->readAll();
        QString str=buf;

        DEBUG_SERVER(cout COUT_SHOW_ID << "WARNING Server reads from client and IGNORES: "<<str.toStdString()<<endl;);

        if (socket->write("Server: game is not on yet !\r\n")==-1) {
            DEBUG_SERVER(cout COUT_SHOW_ID << "Error on write to socket"<<endl;);
        }

        socket->waitForBytesWritten(1000);
        socket->flush();
        return;
    }

    DEBUG_SERVER(cout COUT_SHOW_ID <<" - message: ";);
    switch ((MessageType)messageType) {
        case MT_InitPos: {
            DEBUG_SERVER(cout<<"InitPos"<<endl;);
            readInitPoses(in);

            //AG150112: check if all set
            if (_numPlayersWInitPos == _numPlayers) {
                //if all are set, start game
                startGame();
            } else
                assert(_numPlayersWInitPos < _numPlayers); //ag150511: check if not passed
        }
        break;
        case MT_Action: {
            DEBUG_SERVER(cout<<"Action"<<endl;);

            bool someHadFlag = readNewActions(in);

            if (someHadFlag) {
                cout COUT_SHOW_ID <<" HSTCPServer::agentReadyRead: WARNING: there is a client which already had updated it's location, ignoring"<<endl;
            }
        }
        break;
        case MT_SeekerGoals:
            DEBUG_SERVER(cout<<"SeekerGoals"<<endl;);
            //AG150508: only used by the two robot version as sent to IROS
            readRobotGoalPoses(in , client);
            break;
        case MT_SeekerHB:
            DEBUG_SERVER(cout<<"SeekerHB"<<endl;);
            readRobotHB(in, client);
            break;
        case MT_Message:
            DEBUG_SERVER(cout<<"Message"<<endl;);
            //readAndForwardMessage(in, playerInfo);
            throw CException(_HERE_,"TODO: implement this for multi robot case"); //AG150511: not used
            break;
        case MT_StopServer:
            DEBUG_SERVER(cout<<"StopServer"<<endl;);
            cout COUT_SHOW_ID << "Server stop requested by client "<<client->toString()<<endl;
            ondisconnect();
            break;
        default:
            cout COUT_SHOW_ID << "HSTCPServer::agentReadyRead: Error - Unknown message type ("<<(int)messageType<<") for client"
                              <<client->toString()<<endl;
            break;
    }

    checkIfBothSentAndSend();
}


void HSTCPServer::startGame() {
    usleep(10000);
    DEBUG_SERVER(cout<<"Starting game!"<<endl;);

    assert(_playerVec.size() == _params->numPlayersReq);
    assert(_hiderPlayer!=NULL);
    assert(_seeker1Player!=NULL);

    //set both of the players //AG150513: depricated use of set
    //_hiderPlayer->set = _seeker.set = _seeker2.set = true;
    _gameStatus = HSGlobalData::GAME_STATE_RUNNING;

    //AG140605: check noise params
    if (_params->simObsNoiseStd>_gmap->rowCount()/2.0 || _params->simObsNoiseStd>_gmap->colCount()/2.0) {
        cout << "WARNING: obs. noise std. is very high: "<<_params->simObsNoiseStd<<endl;
    }
    if (_params->simObsNoiseStd<0) throw CException(_HERE_, "Sim. obs. noise std. has to be positive");
    if (_params->simObsFalseNegProb<0 || _params->simObsFalseNegProb>1) throw CException(_HERE_, "Sim. obs. false neg. prob has to be between 0 and 1");
    if (_params->simObsFalsePosProb<0 || _params->simObsFalsePosProb>1) throw CException(_HERE_, "Sim. obs. false pos. prob has to be between 0 and 1");

    //AG140605: set noise std
    decltype(_gausObsNoiseDistr.param()) new_range_gausDist (0, _params->simObsNoiseStd);
    _gausObsNoiseDistr.param(new_range_gausDist);

    DEBUG_SERVER(cout<<"Init auto walkers: "<<flush;);
    //AG140506: init the auto walkers
    initAutoWalkers();
    DEBUG_SERVER(cout<<"ok"<<endl<<"Init logs: "<<flush;);

    //write the first line to the file
    initializeLogs();
    DEBUG_SERVER(cout<<"ok"<<endl<<"Sending update to players..."<<endl;);

    //send first positions
    sendUpdateToPlayers();
}



void HSTCPServer::ondisconnect() {
    //AG120523: start new server instance

    DEBUG_SERVER_VERB(cout COUT_SHOW_ID  << "onDisconnect: a client disconnected" << endl;);

    closeServer(0,true);
}


int HSTCPServer::checkStatus() {
    if (_params->stopAtWin) {
        //ag140130: include check for continuous
        bool winSeeker = false;

        //AG150511: could be later extended for multi agents
        PlayerInfoServer* seekerPlayer = NULL;
        if (_playerVec[0]->isSeeker()) {
            seekerPlayer = _playerVec[0];
        } else if (_playerVec[1]->isSeeker()) {
            seekerPlayer = _playerVec[1];
        } else {
            throw CException(_HERE_,"there are 2 players, but none of them is seeker");
        }

        if (_params->gameType==HSGlobalData::GAME_HIDE_AND_SEEK) { //AG150511: limit to hide-and-seek only
            assert(_params->maxNumberOfPlayers()==2);
            assert(_playerVec.size()==2);

            if (_config->getWinIfCrossed()) {
                winSeeker = seekerPlayer->currentPos.equalsInt(_hiderPlayer->previousPos)
                        && seekerPlayer->previousPos.equalsInt(_hiderPlayer->currentPos); // check crossed
            }
            if (!winSeeker) {
                if (_params->useContinuousPos) {
                    winSeeker = (_gmap->distanceEuc(seekerPlayer->currentPos, _hiderPlayer->currentPos)<=_params->winDist);
                } else if (_params->winDist==0) {
                    winSeeker = seekerPlayer->currentPos.equalsInt(_hiderPlayer->currentPos);
                } else {
                   winSeeker = _gmap->distance(seekerPlayer->currentPos,_hiderPlayer->currentPos)<=_params->winDist;
                }
            }
        } else if (_params->gameType==HSGlobalData::GAME_FIND_AND_FOLLOW) {
            // 'win' if seeker close to hider, and visible
            winSeeker =  ( _gmap->isVisible(seekerPlayer->currentPos, _hiderPlayer->currentPos, true,false)
                           && _gmap->distanceEuc(seekerPlayer->currentPos,_hiderPlayer->currentPos)<=_params->winDist );
        } else if (_params->gameType==HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB
                    || _params->gameType==HSGlobalData::GAME_FIND_AND_FOLLOW_MULTI_ROB) {
            //check if 1 is seeker is close
            for (const PlayerInfoServer* pInfo : _playerVec) {
                if (pInfo->isSeeker()) { //AG160216: check if it is a seeker
                    //check if the seeker is close to the hider
                    winSeeker =  ( _gmap->isVisible(pInfo->currentPos, _hiderPlayer->currentPos, true,false)
                                   && _gmap->distanceEuc(pInfo->currentPos,_hiderPlayer->currentPos)<=_params->winDist );
                    if (winSeeker)
                        break; //found
                }
            }

        }

        if (winSeeker) {
             //seeker won
             DEBUG_SERVER_VERB(cout COUT_SHOW_ID   << "Game ended: seeker won!"<<endl;);
             _gameStatus = HSGlobalData::GAME_STATE_SEEKER_WON;

        } else {
            bool winHider = false;

            if (_params->gameType==HSGlobalData::GAME_HIDE_AND_SEEK) {
                if (_params->useContinuousPos) {
                    winHider = _gmap->distanceEuc(_hiderPlayer->currentPos,_gmap->getBase())<=_params->winDist;
                } else {
                    winHider = _hiderPlayer->currentPos.equalsInt(_gmap->getBase());
                }
                if (winHider) {
                    //hider won
                    DEBUG_SERVER_VERB(cout COUT_SHOW_ID   << "Game ended: hider won!"<<endl;);
                    _gameStatus = HSGlobalData::GAME_STATE_HIDER_WON;
                }
            }

            if (!winHider) {
                if (_params->maxNumActions>0 && (_hiderPlayer->numberActions >= _params->maxNumActions)) { //TODO: put a max time

                    for(PlayerInfoServer* player : _playerVec) {
                        if (player->numberActions != _hiderPlayer->numberActions) throw CException(_HERE_,"hider and a seeker have different amount of actions: "+
                                                                                                 QString::number(_hiderPlayer->numberActions).toStdString()+" and "+
                                                                                                 QString::number(player->numberActions).toStdString());
                    }

                    //tie
                    DEBUG_SERVER_VERB(cout COUT_SHOW_ID   << "Game ended in a tie."<<endl;);
                    _gameStatus = HSGlobalData::GAME_STATE_TIE;

                } else {
                    _gameStatus = HSGlobalData::GAME_STATE_RUNNING;
                }
            }
        }
    }

    if (_gameStatus==HSGlobalData::GAME_STATE_RUNNING && _params->stopAfterNumSteps>0 && _hiderPlayer->numberActions>=(int)_params->stopAfterNumSteps) {
        //AG140520: stop because of max num actions
        _gameStatus = HSGlobalData::GAME_STATE_TIE;
        DEBUG_SERVER_VERB(cout COUT_SHOW_ID   << "Game ended after fixed number of steps: "<<_params->stopAfterNumSteps<<endl;);
    }

    return _gameStatus;
}

void HSTCPServer::initializeLogs() {
    //AG150118: if no log
    if (_gameLog==NULL) return;

    int gameID = _gameLog->startGame(getMapName(), _gmap, _params, _playerVec);

    DEBUG_SERVER(cout COUT_SHOW_ID << "Game ID: "<<gameID<<endl;);

}

long HSTCPServer::writeStepToLog() {
    if (_gameLog==NULL) return -1;

    long logLineID = _gameLog->addGameStep(_hiderPlayer->numberActions, _sentLastActionTime, _gameStatus, _playerVec, _hiderPlayer);

    if (logLineID <0)
        cout << "HSTCPServer::sendUpdateToPlayers - WARNING: logging failed (id="<<logLineID<<")"<<endl;

    return logLineID;
}

void HSTCPServer::newGame() {
    if(_gameStatus == HSGlobalData::GAME_STATE_NOT_STARTED)
        return;//the game is already new!

    sleep(2);

    closeServer(0,false);
}

void HSTCPServer::initPlayers() {
    assert(_gmap!=NULL);

    //AG150430: clear vector
    for(PlayerInfoServer* info: _playerVec) {
        delete info;
    }
    _playerVec.clear();
    _hiderPlayer = NULL;
    _numPlayersWInitPos = 0;
}

QString HSTCPServer::getMapName() {
    QString name;
    if (_params->mapID==HSGlobalData::MAP_PASSED_BY_NETWORK) { //AG130724: passed map by network
        name = "$" + QString::fromStdString(_gmap->getName());
    } else {
        name = HSGlobalData::MAPS[_params->mapID];
    }
    return name;
}

void HSTCPServer::writeGameParamsToStream(QDataStream &out, QByteArray &byteArray, ClientSocket* client) {

    out.setVersion(HSGlobalData::DATASTREAM_VERSION);

    //block size
    out << (BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_GameParams;
    //game type
    out << (quint8)_params->gameType; // _params->gameType;
    //continuous
    //cout << "server SENDING: usercont="<<_params->useContinuousPos<<",as char="<<(int)((quint8)(_params->useContinuousPos?1:0))<<endl;
    out << (quint8)(_params->useContinuousPos?1:0);
    //AG160414: exp. name
    out << QString::fromStdString(_params->expName);
    //max actions
    out << (quint16)_params->maxNumActions;
    //win dist
    out << _params->winDist;
    //min dist
    out << _params->minDistToObstacle;
    //the int of the map (so that both players know which map it is)
    out << (quint16)_params->mapID;
    //map
    _gmap->writeMaptoStream(out);
    //AG160524: distance map file name
    out << QString::fromStdString(_params->mapDistanceMatrixFile);

    //AG140531: observ. noise std.dev.
    out << _params->simObsNoiseStd;
    //AG140605: false negative/positive
    out << _params->simObsFalseNegProb;
    out << _params->simObsFalsePosProb;
    //AG140606: send noise or not
    //out << sendNoisePos;

    //AG160216: sim not visib. dist
    out << _params->simNotVisibDist;

    //AG160505: use dyn. obst for visib check
    out << _params->useDynObstForVisibCheck;

    //AG150511: changes for multi players
    //total number of players
    out << (quint8)_playerVec.size();
    //number of players in the client
    out << (quint8)client->getNumPlayers();
    //meta info for each player in the client
    for (int i=0; i<client->getNumPlayers(); i++) {
        PlayerInfoServer* recPlayer = client->getPlayer(i);
        assert(recPlayer->id>=0); //could be -1
        out << (quint8) recPlayer->id;
        out << (qint8)recPlayer->playerType;
        out << recPlayer->username;
        out << recPlayer->useObsProb;
    }

    //now list other players (numPlayers - client.getNumPlayers())
    for (PlayerInfoServer* player : _playerVec) {
        if (!client->hasPlayerID(player->id)) {
            out << (quint8)player->id;
            out << (qint8)player->playerType;
            out << player->username;
            out << player->useObsProb;
        }
    }

    //put size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(byteArray.size() - sizeof(BLOCK_SIZE));
}

void HSTCPServer::sendParamsToPlayers() {
    //send to players the info of the map.
    DEBUG_SERVER(cout COUT_SHOW_ID  << "sending parameters and map to clients:"<<endl;);

    //AG150708: first check player info params
    //DEBUG_SERVER(
    cout<<"All players and clients:"<<endl;
    size_t playerClntCnt = 0;
    map<int,PlayerInfoServer*> playerIDMap;
    int cl=0;
    for(ClientSocket* client : _clientSocketVec) {
        cout <<"Client #"<<cl<<": "<<client->toString()<<endl;
        for(int i=0;i<client->getNumPlayers(); i++){
            PlayerInfoServer* pi = client->getPlayer(i);
            assert(pi!=NULL);
            cout <<" - "<<i<<") "<<pi->toString()<<endl;
            if (playerIDMap[pi->id]==NULL) {
                playerIDMap[pi->id] = pi;
            } else {
                cout << "ERROR in client "<<client->toString()<<" id "<<pi->id<<" is already occupied by "<<playerIDMap[pi->id]->toString()
                     << " instead of "<<pi->toString()<<endl;
                throw CException(_HERE_,"an id is used double");
            }
            playerClntCnt++;
        }
        cl++;
    }
    assert(_playerVec.size()==playerClntCnt);
    assert(playerIDMap.size()==playerClntCnt);
    //check refs
    cout <<"Listing all players: "<<endl;
    int pl=0;
    for(PlayerInfoServer* pInfo : _playerVec) {
        assert(pInfo!=NULL);
        cout<<" - "<<pl<<") "<<pInfo->toString()<<endl;
        PlayerInfoServer* pi = playerIDMap[pInfo->id];
        assert(pi!=NULL);
        if (pInfo!=pi) {
            cout <<"WARNING: the playerinfo in client: "<<pi->toString()<<" is not refered through same pointer as "<<pInfo->toString()<<endl;
            if (pInfo->id!=pi->id) throw CException(_HERE_, "the playerinfo in list and client do not have the same id");
        }
        pl++;
    }

    normalizeObsProb();

    //AG150511: send parameters to all clients
    for(ClientSocket* client : _clientSocketVec) {
        //write data to matrix
        QByteArray blockToClient;
        QDataStream outClient(&blockToClient, QIODevice::WriteOnly);
        writeGameParamsToStream(outClient, blockToClient, client);
        //send data
        client->getSocket()->write(blockToClient);
        client->getSocket()->flush();
    }

    for(PlayerInfoServer* player: _playerVec) {
        player->flag = false;
        player->multiHasHBPoses = false;
        player->multiHasGoalPoses = false;
    }
}

void HSTCPServer::normalizeObsProb() {
    //AG150708: here normalize the probs
    //first calc sum of prob.
    double obsProbSum = 0;
    unsigned int numSeekers = 0;
    for(const PlayerInfoServer* pi : _playerVec) {
        if (pi->isSeeker()) {
            obsProbSum += pi->useObsProb;
            numSeekers++;
        }
    }
    if (obsProbSum==0) {
        cout << "WARNING: the obs prob sum is 0! Now using uniform probability"<<endl;
    }
    assert(numSeekers>0);
    //normalize
    for(PlayerInfoServer* pi : _playerVec) {
        if (pi->isSeeker()) {
            if (obsProbSum==0) {
                pi->useObsProb = 1.0/numSeekers;
            } else {
                pi->useObsProb = pi->useObsProb/obsProbSum;
            }
        }
    }
}


void HSTCPServer::readAndForwardMessage(QDataStream &in, PlayerInfoServer& sender) {
    throw CException(_HERE_, "this code has to be adapted to multi agents");
}

void HSTCPServer::readRobotGoalPoses(QDataStream &in, ClientSocket* client) {
    //assert(_params->gameHas2Seekers());
    //assert(!sender.multiHasGoalPoses);

    //AG150512: for all players recieve the data
    quint8 numPlayers;
    in >> numPlayers;

    //read per player
    for(quint8 i=0; i<numPlayers; i++) {
        //id
        quint8 id;
        in >> id;

        assert(id<_playerVec.size());
        assert(client->hasPlayerID((int)id));

        PlayerInfoServer* player = _playerVec[id];
        //has poses
        if (player->multiHasGoalPoses) {
            cout COUT_SHOW_ID << " HSTCPServer::readRobotGoalPoses: ERROR "<<player->toString()<<" has already sent the goal poses"<<endl;
        }

        quint8 numGoals; //, numBels;
        in >> numGoals;
        //assert(numGoals>0 && numGoals<=2);

        //init vectors
        player->multiGoalIDVec.resize(numGoals);
        player->multiGoalBPosesVec.resize(numGoals);
        //player->multiGoalBeliefVec.resize(numGoals);

        //now read and store goals
        for (quint8 i=0; i<numGoals; i++) {
            //id goal player
            quint8 idGoalP;
            in >> idGoalP;
            assert(idGoalP<_playerVec.size());

            //set id
            player->multiGoalIDVec[i] = idGoalP;

            //read goal
            player->multiGoalBPosesVec[i].readPosFromStream(in,_params->useContinuousPos);
            //belief
            //in >> player->multiGoalBeliefVec[i];

            //AG160602: update time info received
            player->timestamp = QDateTime::currentDateTime();
        }


        player->multiHasGoalPoses = true;
    }
}

void HSTCPServer::readRobotHB(QDataStream &in, ClientSocket* client) {
    assert(_params->seekerSendsMultiplePoses());
    //assert(!sender.multiHasGoalPoses);

    //read numb. players
    quint8 numPlayers;
    in >> numPlayers;
    DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"read hbs:"<<endl;);
    //read hb for each player
    for(quint8 i=0; i<numPlayers;i ++) {
        quint8 id;
        in >> id;
        assert(id<_playerVec.size());

        //player
        PlayerInfoServer* player = _playerVec[id];
        //has poses
        if (player->multiHasHBPoses) {
            cout COUT_SHOW_ID << " HSTCPServer::readRobotHB: ERROR "<<player->toString()<<" has already sent the HB poses"<<endl;
        }


        //number of belief points for this player
        quint8 numHBs;
        in >> numHBs;
        assert(numHBs>0);
        DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<" - "<<player->toString()<<" numhbs="<<(int)numHBs<<": "<<flush;);
        //read the poses and belief
        //note: these positions are 'highest belief' points, and are not 'connected' to the players
        player->multiHBPosVec.resize(numHBs);
        //player->multiHBBeliefVec.resize(numBels);
        for (size_t i=0; i<(size_t)numHBs; i++) {
            //sender.multiGoalPosesVec[i].readPosFromStream(in,_params->useContinuousPos);
            player->multiHBPosVec[i].readPosFromStream(in,_params->useContinuousPos);
            //in >> player->multiHBBeliefVec[i];
            DEBUG_SERVER_VERB(cout << player->multiHBPosVec[i].toString()<<"; ";);
            assert(player->multiHBPosVec[i].isSet());
            assert(player->multiHBPosVec[i].b>=0);
        }
        DEBUG_SERVER_VERB(cout<<endl;);

        player->multiHasHBPoses = true;

        //AG160602: update time info received
        player->timestamp = QDateTime::currentDateTime();
    }
}



void HSTCPServer::writeRobotGoalsToStream(QDataStream &out, QByteArray &byteArray) { //, PlayerInfo& p2Info) {
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);

    //block size
    out << (BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_SeekerGoals;

    //AG150512: send all goals of seekers to all seekers
    out << (quint8)(_playerVec.size()-1);

    //loop all players
    for (PlayerInfoServer* playerInfo: _playerVec) {
        if (playerInfo->isSeeker()) {
            assert(playerInfo->multiHasGoalPoses);
            assert(playerInfo->multiGoalBPosesVec.size()==playerInfo->multiGoalIDVec.size());
            //assert(playerInfo->multiGoalBPosesVec.size()==playerInfo->multiGoalBeliefVec.size());

            assert(playerInfo->id>=0);
            out << (quint8)playerInfo->id;

            //num. goals
            out << (quint8)playerInfo->multiGoalBPosesVec.size();

            assert(playerInfo->multiGoalIDVec[0]==playerInfo->id);
            assert(playerInfo->multiGoalBPosesVec.size()==1 || playerInfo->multiGoalIDVec[1]!=playerInfo->id);

            //AG150512: write all goals (and beliefs)
            for (size_t i=0; i<playerInfo->multiGoalBPosesVec.size(); i++) {
                //index of goal pos is the player's ID
                out << (quint8)playerInfo->multiGoalIDVec[i];
                //pos
                playerInfo->multiGoalBPosesVec[i].writePostoStream(out,_params->useContinuousPos);
                //belief
                //out << playerInfo->multiGoalBeliefVec[i];
            }

        }
    }

    //put size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(byteArray.size() - sizeof(BLOCK_SIZE));
}

void HSTCPServer::sendRobotGoals() {
    assert(_params->seekerSendsMultiplePoses()); // gameHas2Seekers());
    //assert(_seeker.multiHasGoalPoses && _seeker2.multiHasGoalPoses);
        //checked in writeRobotGoalsToStream

    //AG150512: send to all
    //first write data to block
    QByteArray blockToPlayer;
    QDataStream outPlayer(&blockToPlayer, QIODevice::WriteOnly);

    writeRobotGoalsToStream(outPlayer, blockToPlayer);

    //now send to all clients
    for(ClientSocket* client : _clientSocketVec) {
        if (client->isSeeker()) {
            client->getSocket()->write(blockToPlayer);
            client->getSocket()->flush();
        }
    }

    //reset vars
    for (PlayerInfoServer* player: _playerVec) {
        player->multiHasGoalPoses = false;
    }
}

void HSTCPServer::writeRobotHBToStream(QDataStream &out, QByteArray &byteArray) {
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);

    //block size
    out << (BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_SeekerHB;

    //AG150512: send all goals to all seeker clients
    out << (quint8)(_playerVec.size()-1);
    DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"hb poses to clients:"<<endl;);
    //loop all players
    for (PlayerInfoServer* player: _playerVec) {
        if (player->isSeeker()) {
            assert(player->multiHasHBPoses);
            assert(!player->multiHBPosVec.empty());

            out << (quint8)player->id;
            DEBUG_SERVER_VERB(cout COUT_SHOW_ID << " - "<<player->toString()<<": "<<player->multiHBPosVec.size()<<endl;);
            //num. goals
            out << (quint8)player->multiHBPosVec.size();

            //AG150512: write all goals (and beliefs)
            for (size_t i=0; i<player->multiHBPosVec.size(); i++) {
                //pos
                player->multiHBPosVec[i].writePostoStream(out,_params->useContinuousPos);
                //belief
                //out << player->multiHBBeliefVec[i];
            }
        }
    }

    //put size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(byteArray.size() - sizeof(BLOCK_SIZE));
}

void HSTCPServer::sendRobotHB() {
    //assert(_params->seekerSendsMultiplePoses()); // gameHas2Seekers());
    //assert(_seeker.multiHasGoalPoses && _seeker2.multiHasGoalPoses);
        //checked in writeRobotGoalsToStream

    //AG150512: send to all
    //first write data to block
    QByteArray blockToPlayer;
    QDataStream outPlayer(&blockToPlayer, QIODevice::WriteOnly);

    writeRobotHBToStream(outPlayer, blockToPlayer);

    //now send to all clients
    for(ClientSocket* client : _clientSocketVec) {
        if (client->isSeeker()) {
            client->getSocket()->write(blockToPlayer);
            client->getSocket()->flush();
        }
    }

    //reset vars
    for (PlayerInfoServer* player: _playerVec) {
        player->multiHasHBPoses = false;
    }
}

bool HSTCPServer::startNewServerInstance() {
    //set arguments
    QStringList arg;
    arg << _config->getXMLFile();
    QString path=_config->getServerProg();

    cout  COUT_SHOW_ID << "Starting new server instance: "<<path.toStdString()<<" | params: "<<arg.join(",").toStdString()<<endl;

    bool ok = QProcess::startDetached(path,arg);
    if(!ok)
        cout COUT_SHOW_ID <<"FAILED to start new server"<<endl;

    return ok;
}

void HSTCPServer::callRandhider(int hiderType, QString actionFile, int randomPosDistToBaseHider) {
    QStringList arg;
    //create param list
    arg<<_ip<<QString::number(_port)<<QString::number(hiderType)<<QString::number(randomPosDistToBaseHider);
    //AG120903: add args for hider type action list
    if (hiderType==HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST
            || hiderType==HSGlobalData::OPPONENT_TYPE_HIDER_FILE) arg<< getFilePath(_config->getRecordedActionsPath(), actionFile);

    QString path=_config->getAutoHiderProg();
    DEBUG_SERVER( cout << "Starting random hider: " << path.toStdString() << " | params: " << arg.join(",").toStdString()<<endl; );

    bool ok = QProcess::startDetached(path,arg);
    DEBUG_SERVER_VERB(
        if(ok)
            cout<<"The auto-hider process was started succesfully"<<endl;
        else
            cout<<"The auto-hider process was NOT started"<<endl;
    );
}

void HSTCPServer::createAutoWalker(int autoWalkerType, int n) {
    assert(_walkerVector.empty()); //AG140426
    assert(_params!=NULL);

    if (n>0) {
        DEBUG_SERVER(cout COUT_SHOW_ID << "Auto-walker: (#"<<n<<") ";);
        switch (autoWalkerType) {
        case HSGlobalData::AUTOWALKER_NONE:
            DEBUG_SERVER(cout<<"none"<<endl;);
            break;
        case HSGlobalData::AUTOWALKER_RANDOM: {
            DEBUG_SERVER(cout<<"Random"<<endl;);
            RandomHider* randomHider = new RandomHider(_params, n);
            _walkerVector.push_back(randomHider);
            break;
        }
        case HSGlobalData::AUTOWALKER_RANDOM_GOAL: {
            DEBUG_SERVER(cout<<"Random Goal"<<endl;);
            RandomWalker* randomWalker = new RandomWalker(_params, n);
            _walkerVector.push_back(randomWalker);
            break;
        }
        case HSGlobalData::AUTOWALKER_SFM: {
            DEBUG_SERVER(cout<<"Social Force Model"<<endl;);
            SFMWalkers* sfmWalkers = new SFMWalkers(_params, n);
            _walkerVector.push_back(sfmWalkers);
            break;
        }
        case HSGlobalData::AUTOWALKER_FILE: {
            //AG140605: test if file exists, otherwise relative to mapaction path
            QString autoWalkerPosFileQStr = getFilePath(_config->getRecordedActionsPath(), QString::fromStdString(_params->autoWalkerPosFile));
            DEBUG_SERVER(cout<<"File: "<</*_params->autoWalkerPosFile*/autoWalkerPosFileQStr.toStdString()<<endl;);
            assert(!_params->autoWalkerPosFile.empty());
            FromListHider* fromListHider = new FromListHider(_params, autoWalkerPosFileQStr.toStdString());
            //AG140608: set number of autowalkers such that stored in DB
            _params->autoWalkerN = fromListHider->getNumberOfHiders();
            _walkerVector.push_back(fromListHider);
            break;
        }
        case HSGlobalData::AUTOWALKER_FIXED_POS: {
            DEBUG_SERVER(cout<<"Fixed Pos"<<endl;);
            RandomFixedHider* randomFixedHider = new RandomFixedHider(_params, n);
            _walkerVector.push_back(randomFixedHider);
        }
        default:
            DEBUG_SERVER(cout<<"unknown"<<endl;);
            cout << "WARNING: UNKNOWN walker type!!"<<endl;
            break;
        }
    }
}

void HSTCPServer::initAutoWalkers() {
    if (_walkerVector.size()>0) {
        //move the walkers, keeping in mind the current position of seeker and hider
        for(AutoWalker* walker: _walkerVector) {
            DEBUG_SERVER(cout COUT_SHOW_ID << "Init of autowalker "<<walker->getName()<<": "<<flush;);

            //AG150708: init with one other player, maybe in future we need all
            walker->playerInfo.useObsProb = 1;
            walker->initBelief(_gmap, _seeker1Player);


            DEBUG_SERVER(cout << "ok"<<endl;);
        }
    }
}

void HSTCPServer::moveAutoWalkers(vector<IDPos>& nextPosAutoWalkers) {
    if (_walkerVector.size()>0) {
        //move the walkers, keeping in mind the current position of seeker and hider
        for(AutoWalker* walker: _walkerVector) {
            DEBUG_SERVER(cout COUT_SHOW_ID << "Next step of autowalker "<<walker->getName()<<": "<<flush;);
            //AG150708: WARNING _playerVec[0] could be the same as hiderPlayer !!
            vector<IDPos> posVec = walker->getAllNextPos(_seeker1Player->currentPos, _hiderPlayer->currentPos);
            nextPosAutoWalkers.insert(nextPosAutoWalkers.end(), posVec.begin(), posVec.end());
            DEBUG_SERVER(cout << "ok, "<<posVec.size()<<" poses"<<endl;);
        }
    }
}

GMap* HSTCPServer::getGMap() {
    return _gmap;
}

//ag111201
void HSTCPServer::setServer(HSServer* server) {
    _server = server;
}

void HSTCPServer::setip(QString ip) {
    _ip = ip;
}

void HSTCPServer::setport(int p) {
    _port=p;
}

void HSTCPServer::closeServer(int exitStatus, bool startNewServer) {
    //close db
    DEBUG_SERVER(cout<<"Closing server ... "<<flush;);
    //this server
    close();

    //AG150513: all client connections
    for (ClientSocket* client : _clientSocketVec) {
        client->close();
    }

    if (_gameLog!=NULL && _gameLog->isOpen()) {
        DEBUG_SERVER(cout<<"ok"<<endl<<"Closing log db ... "<<flush;);
        _gameLog->stopGame();
        _gameLog->close();
        DEBUG_SERVER(cout<<"ok"<<endl;);
    }

    if (startNewServer) {
        startNewServerInstance();
    }
    DEBUG_SERVER(cout<<"Now exiting with return code "<<exitStatus<<endl;);
    QCoreApplication::exit(exitStatus);
}

//TODO CHANGE ALL EXIT -> this closeServer
// TODO: also in clients:  -> exit -> qcoreapp (DO TEST FIRST)


void HSTCPServer::timerTestUpdate() {
    _timerTestC++;
    cout COUT_SHOW_ID << "timer="<<_timerTestC<<", port="<<_port<<", is listening="<<
                         isListening()<<", open sockets:";

    int n = 0;

    //AG150513: check all clients
    for (ClientSocket* client : _clientSocketVec) {
        if (client->getSocket()->isOpen()) {
            cout << " "<<client->toString();
            n++;
        }
    }

    if (n==0) {
        cout << "NONE"<<endl;
    } else {
        cout <<endl;
    }
}

void HSTCPServer::clientDisconnected(ClientSocket *client) {
    DEBUG_SERVER(cout<<"Client disconnected: "<<client->toString()<<endl<< "Closing server in 2 s"<<endl);

    sleep(2);
    closeServer(0,true);
}

void HSTCPServer::acceptError(ClientSocket *client, QAbstractSocket::SocketError s) {
    DEBUG_SERVER(cout COUT_SHOW_ID << "ERROR - Socket error from "<<client->toString()
                                    << ": " <<  (int)s<<endl
                                    << "Aborting all."<<endl;);

    abort();

    //abort all sockets
    for (ClientSocket* client : _clientSocketVec) {
        client->getSocket()->abort();
    }

    //AG160205: not using QCoreApplication::exit() since it seems to wait for other thread and leaves program running
    exit(-2);
}

void HSTCPServer::acceptError(QAbstractSocket::SocketError s) {
    DEBUG_SERVER(cout COUT_SHOW_ID << "ERROR - Socket error from server: "<< (int)s<<endl<< "Aborting all."<<endl;);

    abort();

    //abort all sockets
    for (ClientSocket* client : _clientSocketVec) {
        client->getSocket()->abort();
    }

    //AG160205: not using QCoreApplication::exit() since it seems to wait for other thread and leaves program running
    exit(-2);
}

