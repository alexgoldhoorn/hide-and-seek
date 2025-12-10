
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>

#include <QProcess>
#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include "Server/hstcpserver.h"

#include "hsglobaldata.h"

#include <pthread.h>
#include <unistd.h> //ag130813: added for sleep in Ubuntu 13.04

#include "hsconfig.h"

#include "hsglobaldata.h"
#include "exceptions.h"
#include "Server/hsserverclientcommunication.h"

#include "AutoHider/sfmwalkers.h"
#include "AutoHider/randomhider.h"
#include "AutoHider/randomwalker.h"
#include "AutoHider/fromlisthider.h"
#include "AutoHider/randomfixedhider.h"

using namespace std;

HSTCPServer::HSTCPServer(HSServerConfig* config, HSGameLog* gameLog, QObject *parent)
    : QTcpServer(parent), _config(config), _gameLog(gameLog), _uniformProbDistr(0,1), _gausObsNoiseDistr(0,1), _randomGenerator(_randomDevice())
{
    DEBUG_SERVER_VERB(cout COUT_SHOW_ID << "Starting server..."<<endl;);

    _gmap = new GMap(_params);
    _gameStatus = HSGlobalData::GAME_STATE_NOT_STARTED;
    _numConn = 0;
    _hiderSocket = NULL;
    _seekerSocket = NULL;
    _seeker2Socket = NULL;
    
    _params = NULL;
    _seekerBeliefScore = _seeker2BeliefScore = -1;
    _seekerReward = _seeker2Reward = -1;

    //connect to receiving new client connection
    connect(this, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    //AG150224: handle error
    connect(this, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));

    //AG150223: timer to debug issue of not closing servers and see when there are still servers running in the background
    connect(&_timerTest, SIGNAL(timeout()), this, SLOT(timerTestUpdate()));
    _timerTestC = 0;
    _timerTest.start(30000);
}

HSTCPServer::~HSTCPServer() {
    for(AutoWalker* walker: _walkerVector) {
        delete walker;
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

    QTcpSocket* connection = this->nextPendingConnection();
    assert(connection!=NULL);
    DEBUG_SERVER(cout COUT_SHOW_ID <<"new connection request: "<<connection->socketDescriptor()<<endl;);

    if(connection->isOpen()) {
        //wait to get data
        while (!connection->waitForReadyRead(6000)) {
            DEBUG_SERVER_VERB(cout<<"."<<flush;);
        }

        //read
        BLOCK_SIZE blockSize=0;
        QDataStream in(connection);
        in.setVersion(HSGlobalData::DATASTREAM_VERSION);


        if (blockSize == 0) {
            if (connection->bytesAvailable() < (int)sizeof(BLOCK_SIZE)) {
                DEBUG_SERVER(cout COUT_SHOW_ID <<"HSTCPServer::handleNewConnection: less bytes in the array than expected: size."<<endl;);
                connection->flush();
                return; //wait for more (?)
            }
            //block size
            in >> blockSize;
        }


        if (connection->bytesAvailable() < blockSize) {
            DEBUG_SERVER(cout COUT_SHOW_ID <<"HSTCPServer::handleNewConnection: less bytes in the array than expected 1. blocksize="
                         <<blockSize<<", available:"<<connection->bytesAvailable() <<endl;);
            connection->flush();
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

        //AG150120: read the message type (although here shoulde be con request)
        MESSAGE_TYPE_SIZE messageType;
        in >> messageType;

        switch ((MessageType)messageType) {
            case MT_StopServer:
                cout  COUT_SHOW_ID << "Server stop requested"<<endl;
                /*close();
                exit(0);*/
                closeServer(0,false);
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
        quint8 gameType, useContinuous, myType;
        quint16 mapID, randomPosDistToBaseHider, autoWalkerN, numSteps;
        qint16 oppType, autoWalkerType;
        QString username;
        QString actionFile ="";//ag120903
        QString autoWalkerPosFile = "";
        bool stopAtWin;

        //AG140212: game type
        in >> gameType;

        //check connection number
        //AG141124: for find-and-follow 2 rob allow an extra player
        if(_numConn > 2 || (_params!=NULL && !_params->gameHas2Seekers() && _numConn > 1)) {
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
        in >> username;

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

        connection->flush();

        //the params that will be changed this function run
        SeekerHSParams* curParams = NULL;

        //AG141124: generalized connections
        if(_numConn==0) {
            //init players
            initPlayers();
        }

        DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"Connection established with player "<<(_numConn+1)<<" - ";);

        cout << ">- seeker.set="<<_seeker.set<<", hider.set="<<_hider.set<<", seeker2.set="<<_seeker2.set<<"---"<<endl;

        switch(myType) {
        case HSGlobalData::PLAYER_TYPE_SEEKER:
            if (_seeker.set) {
                if (_seeker2.set) {
                    cout << "ERROR: second seeker is trying to connect, but was already set"<<endl;
                    return;
                } else if (!_params->gameHas2Seekers()) {
                    cout << "ERROR: expecting a hider instead of a second seeker"<<endl;
                    return;
                }
                DEBUG_SERVER_VERB(cout <<"seeker 2"<<endl;);
                _seeker2Socket = connection;
                _seeker2.set = true;
                _seeker2.setUserName(username);
                _seeker2.metaInfo = metaInfo;
                _seeker2.comments = comments;
                //_params = &_seeker.params;
                curParams = &_seeker2.params;
                _seeker2.playerType = HSGlobalData::P_Seeker2;

                //AG150110: set connection here
                connect(_seeker2Socket, SIGNAL(readyRead()), this, SLOT(seeker2ReadyRead()));
                //AG150223: handle disconnection
                connect(_seeker2Socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
                //AG150224: handle error
                connect(_seeker2Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));
            } else {
                DEBUG_SERVER_VERB(cout <<"seeker"<<endl;);
                _seekerSocket = connection;
                _seeker.set = true;
                _seeker.setUserName(username);
                _seeker.metaInfo = metaInfo;
                _seeker.comments = comments;
                //_params = &_seeker.params;
                curParams = &_seeker.params;
                _seeker.playerType = HSGlobalData::P_Seeker1;

                //AG150110: set connection here
                connect(_seekerSocket, SIGNAL(readyRead()), this, SLOT(seekerReadyRead()));
                //AG150223: handle disconnection
                connect(_seekerSocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
                //AG150224: handle error
                connect(_seekerSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));
            }
            break;
        case HSGlobalData::PLAYER_TYPE_HIDER:
            if (_hider.set) {
                cout << "ERROR: hider is trying to connect, but was already set"<<endl;
                return;
            }
            DEBUG_SERVER_VERB(cout <<"hider"<<endl;);
            _hiderSocket = connection;
            _hider.set = true;
            _hider.setUserName(username);
            _hider.metaInfo = metaInfo;
            _hider.comments = comments;
            //_params = &_hider.params;
            curParams = &_hider.params;            
            _hider.playerType=HSGlobalData::P_Hider;

            //AG150112: set connection
            connect(_hiderSocket, SIGNAL(readyRead()), this, SLOT(hiderReadyRead()));
            //AG150223: handle disconnection
            connect(_hiderSocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
            //AG150224: handle error
            connect(_hiderSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));
            break;
        default:
            cout << "ERROR: unknown player type: "<<myType<<endl;
            //exit(1);
            closeServer(1,false);
            break;
        }

        if(_numConn==0) {
            _params = curParams;

            //AG140509: here the params are chosen, set it for GMap
            _gmap->setParams(_params);

        } else if ((_params->gameHas2Seekers() && _numConn == 2) ||
                   (!_params->gameHas2Seekers() && _numConn == 1)) { //last connection

            //close this server and start new one
            disconnect(this, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
            this->close();

            //ag120523:
            startNewServerInstance();
        }

        DEBUG_SERVER(
        cout <<"-------[Connection #"<<_numConn<<"]-------"<<endl
             <<"Game type: "<<(int)gameType<<endl
             <<"Name: "<<username.toStdString()<<endl
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
             <<"is global params="<<(_params==curParams)<<endl
             <<"stop after win="<<stopAtWin<<", after "<<numSteps<<" steps"<<endl
             <<"-------------"<<endl;
        );

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

        //ag141125: if it is the last connection... start the game
        if ((_params->gameHas2Seekers() && _numConn == 2) ||
                (!_params->gameHas2Seekers() && _numConn == 1)) {

            //we have two (or 3) connected players

            _gameStatus = HSGlobalData::GAME_STATE_RUNNING;
            //set false, wait for initial position of both
            _seeker.set = _seeker2.set = _hider.set = false;

            DEBUG_SERVER_VERB(
                cout COUT_SHOW_ID << "Starting the game, seeker "<<_seeker.params.userName;
                if (_params->gameHas2Seekers())
                    cout << "& "<<_seeker2.params.userName;
                cout<<" vs. hider "<<_hider.params.userName<<endl;
            );

            //send map to players
            sendParamsToPlayers();

        } else if (_numConn==0) { // for first connection

            //AG NOTE: map is fixed here, since the first player decides the map!

            //AG140608: set params such that it gets the rigth params (continuous/not..)
            _gmap->setParams(curParams);

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
                    //exit(-1);
                    closeServer(-1,false);
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


        } //if numConn==1

        //increase connection number
        _numConn++;
    } else {
        DEBUG_SERVER_VERB(cout COUT_SHOW_ID <<"Connection closed beforehand by client."<<endl;);
    }

    cout << "<- seeker.set="<<_seeker.set<<", hider.set="<<_hider.set<<", seeker2.set="<<_seeker2.set<<"---"<<endl;
}


void HSTCPServer::readInitPos(QDataStream& in, PlayerInfo *info) {
    //AG121112: set prev pos
    info->previousPos = info->currentPos;

    info->currentPos.readPosFromStream(in, _params->useContinuousPos);

    //AG140409: belief score not passed since could not have been calculated yet (missing opponent pos)

    //socket->flush();
}

void HSTCPServer::readNewAction(QDataStream& in, PlayerInfo *info, double* beliefScore, double* seekerReward) {
    //retrieve info
    qint16 action; //AG140410: changed form uint -> int
    //quint16 row, col;
    //double rowD, colD;

    in >> action;

    Pos pos;
    pos.readPosFromStream(in, _params->useContinuousPos);

    DEBUG_SERVER(cout COUT_SHOW_ID << info->toString() <<" Server reads: "<<info->numberActions +1<<" action="<<(int)action<<", "<<pos.toString(););

    //AG140409: belief score (NOTE: this is of the previous position and belief)
    double beliefScoreRec;
    in >> beliefScoreRec;
    if (beliefScore!=NULL) {
        *beliefScore = beliefScoreRec;
    }
    //AG140612: seeker reward
    double reward;
    in >> reward;
    if (seekerReward!=NULL) {
        *seekerReward = reward;
    }

    //AG150216: read multi chosen pos
    if (_params->gameHas2Seekers() && info->isSeeker()) {
        info->multiChosenPos.readPosFromStream(in,_params->useContinuousPos);
    }

    if (!info->flag) {
        //not yet received for this turn
        info->lastAction = (int)action; //action[_hider.numa] = (int)a;
        info->numberActions++;
        info->flag = true;
        //AG121112 set previous
        info->previousPos = info->currentPos;
        //set hider current        
        info->currentPos = pos;
        info->timestamp = QDateTime::currentDateTime();
        //socket->flush();

    } else {
        //already received, so wait for next player to finish
        DEBUG_SERVER(cout  COUT_SHOW_ID << info->toString() << " Ignoring - wait for opponent turn"<<endl;);
    }
}

void HSTCPServer::writeUpdateToStream(QDataStream &out, QByteArray& byteArray, const Pos& pos,
                                      vector<IDPos>& nextPosAutoWalkers, bool sendPosWNoise, Pos* posWNoise, Pos* pos2, Pos* posWNoise2) {

    out.setVersion(HSGlobalData::DATASTREAM_VERSION);

    out << (BLOCK_SIZE)0; //block size
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_Update;
    out << (quint16)1; //validation byte
    out << (quint16)_gameStatus; //game status

    pos.writePostoStream(out,_params->useContinuousPos);

    //AG140606: pos w noise
    if (sendPosWNoise) posWNoise->writePostoStream(out,_params->useContinuousPos);

    //AG150112: send other position
    if (_params->gameHas2Seekers() && pos2!=NULL) {
        pos2->writePostoStream(out,_params->useContinuousPos);
        if (sendPosWNoise) posWNoise2->writePostoStream(out,_params->useContinuousPos);
    }

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

    vector<IDPos> nextPosAutoWalkers;
    moveAutoWalkers(nextPosAutoWalkers);

    //-- data to hider -- //AG140605: without noise
    QByteArray blockToHider;
    QDataStream outHider(&blockToHider, QIODevice::WriteOnly);

    //-- data to seeker --
    //noisy hider pos to seeker 1
    Pos hiderPosWNoise = applyNoise(_seeker.currentPos, _hider.currentPos);

    QByteArray blockToSeeker;
    QDataStream outSeeker(&blockToSeeker, QIODevice::WriteOnly);

    //AG150203: pos of hider with noise for seeker 2
    Pos hiderPosWNoise2;
    Pos* hiderPosWNoise2P = NULL;


    //AG150112: handle case of having 2 seekers
    if (_params->gameHas2Seekers()) {
        //-- data to seeker --
        //TODO: x set different noise to both players, AND send each of them to the other
        //      add noise to other seeker's pos
        //noisy hider pos to seeker 2
        hiderPosWNoise2 = applyNoise(_seeker2.currentPos, _hider.currentPos);
        hiderPosWNoise2P = &hiderPosWNoise2;

        //to hider
        writeUpdateToStream(outHider, blockToHider, _seeker.currentPos, nextPosAutoWalkers, false, NULL,
                            &_seeker2.currentPos, NULL);
        //to seeker (1)
        writeUpdateToStream(outSeeker, blockToSeeker, _hider.currentPos, nextPosAutoWalkers, true, &hiderPosWNoise,
                            &_seeker2.currentPos, &hiderPosWNoise2);

        //to seeker (2)
        QByteArray blockToSeeker2;
        QDataStream outSeeker2(&blockToSeeker2, QIODevice::WriteOnly);
        writeUpdateToStream(outSeeker2, blockToSeeker2, _hider.currentPos, nextPosAutoWalkers, true, &hiderPosWNoise2,
                            &_seeker.currentPos, &hiderPosWNoise);

        //send
        _seeker2Socket->write(blockToSeeker2);
        _seeker2Socket->flush();

    } else {
        //to hider
        writeUpdateToStream(outHider, blockToHider, _seeker.currentPos, nextPosAutoWalkers, false);
        //to seeker (1)
        writeUpdateToStream(outSeeker, blockToSeeker, _hider.currentPos, nextPosAutoWalkers, true, &hiderPosWNoise);
    }

    //send
    assert(_seekerSocket!=NULL);
    assert(_hiderSocket!=NULL);
    assert(_seekerSocket!=_hiderSocket);
    //cout <<"Sending update to hider, size="<<blockToHider.size()<<endl;
    _hiderSocket->write(blockToHider);
    _hiderSocket->flush();
    //cout <<"Sending update to seeker, size="<<blockToSeeker.size()<<endl;
    _seekerSocket->write(blockToSeeker);
    _seekerSocket->flush();

    int logLineID = writeStepToLog(&hiderPosWNoise, hiderPosWNoise2P); //log the actions in the files.
    if (logLineID <0)
        cout << "HSTCPServer::sendUpdateToPlayers - WARNING: logging failed (id="<<logLineID<<")"<<endl;


    _hider.flag = _seeker.flag = _seeker2.flag = false;
}

Pos HSTCPServer::applyNoise(Pos seekerPos, Pos hiderPos) {
    DEBUG_SERVER(cout<<"Adding noise to "<<hiderPos.toString()<<": ";);
    assert(seekerPos.isSet());
    assert(hiderPos.isSet());

    //AG150204: DO take into account dynamical obstacles, when they are obstructing..it should not be visible
    bool isVisib = _gmap->isVisible(seekerPos,hiderPos,true);
    Pos pos(hiderPos);

    if (isVisib) {
        if (_params->simObsFalseNegProb>0) {            //TODO:THIS PROBABLY WON'T WORK, BECAUSE WE EXPECT A CORRECT POS IN SEEKER(?)
            //check whether to add a false negative
            double p = _uniformProbDistr(_randomGenerator);
            if (p<=_params->simObsFalseNegProb) {
                DEBUG_SERVER(cout<<"false neg.";);
                //set false negative
                pos.clear();
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
            }
        }

        if (pos.isSet()) {
            assert(_gmap->isPosInMap(pos) && !_gmap->isObstacle(pos));
            //try to add noise
            uint c=0;
            Pos pos2;
            do {
                pos2.set(pos.rowDouble() + _gausObsNoiseDistr(_randomGenerator),
                        pos.colDouble() + _gausObsNoiseDistr(_randomGenerator) );
                c++;
                if (c>=100) {
                    cout << "HSTCPServer::applyNoise: WARNING: no valid noise found to get a correct pos, returning original"<<endl;
                    pos2 = pos;
                }
            } while(!_gmap->isPosInMap(pos2) || _gmap->isObstacle(pos2));
            pos = pos2;
        }
    } else {
        pos.clear();
        DEBUG_SERVER(cout<<" (not visib.)";);
    }

    DEBUG_SERVER(cout <<" -> "<<pos.toString()<<endl;);
    return pos;
}

//AG150112: check if ALL are ready and send
void HSTCPServer::checkIfBothSentAndSend() {
    bool use2Robots = (_params->gameHas2Seekers());

    //AG150202: check if using 2 seekers and need to send goals
    if (use2Robots && _seeker.multiHasGoalPoses && _seeker2.multiHasGoalPoses) {
        sendRobotGoals();
    } else if ( _seeker.flag && _hider.flag && (!use2Robots || _seeker2.flag)) {//check if all players ready sent

        if (_hider.numberActions==_seeker.numberActions && (!use2Robots || _seeker2.numberActions==_seeker.numberActions)) {
            //both players sent an action
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

        } else {
            cout COUT_SHOW_ID  << "ERROR: not both players have sent all the actions, seeker: " << (_seeker.flag?"sent":"NOT sent")
                 << " last action, sent " << _seeker.numberActions << " in total; hider: " << (_hider.flag?"sent":"NOT sent")
                 << " last action, sent " << _hider.numberActions <<"; ";
            if (use2Robots) cout << "seeker 2: "<<(_seeker2.flag?"sent":"NOT sent")<<" last action, sent "<<_seeker2.numberActions;
            cout<<endl;
        }
    }
}

void HSTCPServer::hiderReadyRead() {
    agentReadyRead(_hiderSocket, _hider);
}

void HSTCPServer::seekerReadyRead() {
    agentReadyRead(_seekerSocket, _seeker, &_seekerBeliefScore, &_seekerReward);
}

void HSTCPServer::seeker2ReadyRead() {
    agentReadyRead(_seeker2Socket, _seeker2, &_seeker2BeliefScore, &_seeker2Reward);
}

//AG140112: now a readyRead function for each connection,
//          for more agents could be 1, whereby the agent sends it's id/name
//          and to be more secure it could send a by the server generated ID
//AG150112: made a general function for all agents
void HSTCPServer::agentReadyRead(QTcpSocket* socket, PlayerInfo &playerInfo, double *seekerBeliefScore, double* seekerReward) {
    BLOCK_SIZE blockSize=0;
    QDataStream in(socket);
    in.setVersion(HSGlobalData::DATASTREAM_VERSION);

    if (blockSize == 0) {
        if (socket->bytesAvailable() < (int)sizeof(BLOCK_SIZE)) {
            DEBUG_SERVER(cout COUT_SHOW_ID << playerInfo.toString() <<" HSTCPServer::readInitPos: less bytes in the array than expected: size."<<endl;);
            socket->flush();
            return;
        }
        in >> blockSize;
    }

    if (socket->bytesAvailable() < blockSize) {
        DEBUG_SERVER(cout COUT_SHOW_ID << playerInfo.toString() <<" HSTCPServer::readInitPos: less bytes in the array than expected 1. blocksize="<<blockSize
                     <<", available:"<<socket->bytesAvailable() <<endl;);
        socket->flush();
        return;
    }

    //socket->flush();
    //AG150120: message type
    MESSAGE_TYPE_SIZE messageType;
    in >> messageType;

    DEBUG_SERVER(cout COUT_SHOW_ID << playerInfo.toString() <<" HSTCPServer::agentReadyRead "
                    << (playerInfo.playerType<0?"???":HSGlobalData::PLAYER_TYPE_NAMES[playerInfo.playerType].toStdString())
                    <<" ["<<(int)playerInfo.playerType<<"] ("<<playerInfo.username.toStdString()
                    <<", set="<<playerInfo.set<<", flag="<<playerInfo.flag<< ", status: "<<_gameStatus<<"), msgType="<<(int)messageType<<endl;);

    if(!playerInfo.set) {
        if ((MessageType)messageType!=MT_InitPos) {
            cout COUT_SHOW_ID << "HSTCPServer::agentReadyRead: ERROR - expected init pos for "<<playerInfo.username.toStdString()
                              <<", but message type="<<messageType<<endl;
            return;
        }
        //read his initial position
        readInitPos(in, &playerInfo);

        DEBUG_SERVER(cout COUT_SHOW_ID << playerInfo.toString() <<"  initial position: "<<playerInfo.currentPos.toString()<<endl;);

        //initial position set
        playerInfo.set = true;

        //AG150112: check if all set
        if (_hider.set && _seeker.set && (!_params->gameHas2Seekers() || _seeker2.set)) {
            //if all are set, start game
            startGame();
        }

        return; //AG131210: wait for both players to be ready

    } else if(_gameStatus > HSGlobalData::GAME_STATE_RUNNING) {
        //the game is over so reset the game
        cout COUT_SHOW_ID  << "WARNING: game already ended, but still receving from agent"<<endl;

        QByteArray buf = socket->readAll();
        QString str=buf;

        DEBUG_SERVER(cout COUT_SHOW_ID << playerInfo.toString() <<" server reads from seeker and ignores: "<<str.toStdString()<<endl;);

        if (socket->write("Server: game is not on yet !\r\n")==-1)
            DEBUG_SERVER(cout COUT_SHOW_ID << playerInfo.toString() <<" error on write socket"<<endl;);

        socket->waitForBytesWritten(1000);
        socket->flush();
        return;

    } else {
        switch ((MessageType)messageType) {
            case MT_Action: {
                bool flag = playerInfo.flag;

                readNewAction(in, &playerInfo, seekerBeliefScore, seekerReward);

                if (flag) {
                    //writeOppInfo(_hiderSocket,&_hider,"Wait for hider");
                    //DOESN't make sense... to write...
                    return; //wait for other player
                }
            }
            break;
            case MT_SeekerGoals:
                readRobotGoalPoses(in , playerInfo);
                break;
            /*case MT_ChosenSeekerGoal:
                readRobotChosenGoalPose(in, playerInfo);*/
            case MT_Message:
                readAndForwardMessage(in, playerInfo);
                break;
            case MT_StopServer:
                cout COUT_SHOW_ID << playerInfo.toString() <<" Server stop requested."<<endl;
                ondisconnect();
                break;
            default:
                cout COUT_SHOW_ID << playerInfo.toString() <<" HSTCPServer::agentReadyRead: Error - Unknown message type ("<<(int)messageType<<")"<<endl;
                break;
        }
    }

    checkIfBothSentAndSend();
}


void HSTCPServer::startGame() {    
    usleep(10000);
    DEBUG_SERVER(cout<<"Starting game!"<<endl;);

    //set both of the players
    _hider.set = _seeker.set = _seeker2.set = true;
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

    //cout <<" --- start game --- send update"<<endl;

    //write the first line to the file
    initializeLogs();
    DEBUG_SERVER(cout<<"ok"<<endl<<"Sending update to players..."<<endl;);

    //send first positions
    sendUpdateToPlayers();
}



void HSTCPServer::ondisconnect() {
    //AG120523: start new server instance

    DEBUG_SERVER_VERB(cout COUT_SHOW_ID  << "onDisconnect: a client disconnected" << endl;);
    /*startNewServerInstance();

    //AG120531: close (db/file/..)
    if (_gameLog!=NULL) _gameLog->close();*/

    //exit(0);
    closeServer(0,true);
}


int HSTCPServer:: checkStatus() {
    //AG150112: check if used 2 robots (could be centralized)
    bool use2Robots = (_params->gameHas2Seekers());

    if (_params->stopAtWin) {
        //ag140130: include check for continuous
        bool winSeeker = false;
        if (_config->getWinIfCrossed()) {
            winSeeker = _seeker.currentPos.equalsInt(_hider.previousPos)
                    && _seeker.previousPos.equalsInt(_hider.currentPos); // check crossed

            //AG150112: check for 2 robots
            if (use2Robots && !winSeeker)
                winSeeker = _seeker2.currentPos.equalsInt(_hider.previousPos)
                        && _seeker2.previousPos.equalsInt(_hider.currentPos); // check crossed for 2nd seeker
        }
        if (!winSeeker) {
            if (_params->useContinuousPos) {
                winSeeker = (_gmap->distanceEuc(_seeker.currentPos, _hider.currentPos)<=_params->winDist);
                //cout << "cont dist dist="<<_gmap->distanceEuc(_seeker.current, _hider.current)<<", windist="<<_params->winDist<<endl;

                //AG150112: check for 2 robots
                if (use2Robots && !winSeeker)
                    winSeeker = (_gmap->distanceEuc(_seeker2.currentPos, _hider.currentPos)<=_params->winDist);
            } else if (_params->winDist==0) {
                winSeeker = _seeker.currentPos.equalsInt(_hider.currentPos);

                //AG150112: check for 2 robots
                if (use2Robots && !winSeeker)
                    winSeeker = _seeker2.currentPos.equalsInt(_hider.currentPos);
            } else {
               winSeeker = _gmap->distance(_seeker.currentPos,_hider.currentPos)<=_params->winDist;

               //AG150112: check for 2 robots
               if (use2Robots && !winSeeker)
                   winSeeker = _gmap->distance(_seeker2.currentPos,_hider.currentPos)<=_params->winDist;
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
                    winHider = _gmap->distanceEuc(_hider.currentPos,_gmap->getBase())<=_params->winDist;
                } else {
                    winHider = _hider.currentPos.equalsInt(_gmap->getBase());
                }
                if (winHider) {
                    //hider won
                    DEBUG_SERVER_VERB(cout COUT_SHOW_ID   << "Game ended: hider won!"<<endl;);
                    _gameStatus = HSGlobalData::GAME_STATE_HIDER_WON;
                }
            }

            if (!winHider) {
                if (_params->maxNumActions>0 && (_seeker.numberActions >= _params->maxNumActions || _hider.numberActions >= _params->maxNumActions)) { //TODO: put a max time
                    assert(_seeker.numberActions == _hider.numberActions);
                    //tie
                    DEBUG_SERVER_VERB(cout COUT_SHOW_ID   << "Game ended in a tie."<<endl;);
                    _gameStatus = HSGlobalData::GAME_STATE_TIE;

                } else {
                    _gameStatus = HSGlobalData::GAME_STATE_RUNNING;
                }
            }
        }
    }

    if (_gameStatus==HSGlobalData::GAME_STATE_RUNNING && _params->stopAfterNumSteps>0 && _seeker.numberActions>=(int)_params->stopAfterNumSteps) {
        //AG140520: stop because of max num actions
        _gameStatus = HSGlobalData::GAME_STATE_TIE;
    }

    return _gameStatus;
}

void HSTCPServer::initializeLogs() {
    //AG150118: if no log
    if (_gameLog==NULL) return;

    _sentLastActionTime = QDateTime::currentDateTime();

    //AG150112: check if seeker2 is there
    PlayerInfo* seeker2 = NULL;
    if (_params->gameHas2Seekers()) {
        seeker2 = &_seeker2;
    }

    //AG120530: open log
    int gameID = _gameLog->startGame(getMapName(),_gmap, &_seeker, &_hider, seeker2, _params);

    DEBUG_SERVER(cout COUT_SHOW_ID << "Game ID: "<<gameID<<endl;);

    //AG140605: not sure if adding noise..
    /*int logLineID = writeStepToLog(NULL,NULL );
    if (logLineID <0)
        cout << "HSTCPServer::initializeLogs - WARNING: logging failed (id="<<logLineID<<")"<<endl;*/
}

int HSTCPServer::writeStepToLog(const Pos* hiderPosWNoiseSent, const Pos* hiderPosWNoiseSent2) {
    //AG150118: if no log used
    if (_gameLog==NULL) return -1;

    //AG140212: calc distance to decide 'score'
    double dsh = _gmap->distance(_hider.currentPos, _seeker.currentPos);
    double dshEuc = _gmap->distanceEuc(_hider.currentPos, _seeker.currentPos);
    //TOOD: to calculate, but might slow down, and since not yet important we don't calc them
    double dsb = -1;
    double dhb = -1;

    //AG150112: distance to seeker2
    double ds2h = -1;
    double ds2hEuc = -1;
    double dss2 = -1;
    double dss2Euc = -1;
    Pos const* seeker2Pos = NULL;
    bool vis_sh,vis_s2h,vis_ss2;
    vis_sh = _gmap->isVisible(_seeker.currentPos, _hider.currentPos, false); //TODO: also check with dyn. obst??
    vis_s2h = vis_ss2 = false;
    /*Pos* goalS1FromS1, goalS2FromS1;
    double goalS1FromS1Bel, goalS2FromS1Bel;
    Pos* goalS1FromS2, goalS2FromS2;
    double goalS1FromS2Bel, goalS2FromS2Bel;
    goalS1FromS1 = goalS2FromS1 = goalS1FromS2 = goalS2FromS2 = NULL;
    goalS1FromS1Bel = goalS2FromS1Bel = goalS1FromS2Bel = goalS2FromS2Bel = -1;*/

    //calculate stats for 2 seekers game
    if (_params->gameHas2Seekers()) {
        ds2h = _gmap->distance(_hider.currentPos, _seeker2.currentPos);
        ds2hEuc = _gmap->distanceEuc(_hider.currentPos, _seeker2.currentPos);
        dss2 = _gmap->distance(_seeker.currentPos, _seeker2.currentPos);
        dss2Euc = _gmap->distanceEuc(_seeker.currentPos, _seeker2.currentPos);
        seeker2Pos = &_seeker2.currentPos;
        vis_s2h = _gmap->isVisible(_seeker2.currentPos, _hider.currentPos, false); //TODO: also check with dyn. obst??
        vis_ss2 = _gmap->isVisible(_seeker.currentPos, _seeker2.currentPos, false); //TODO: also check with dyn. obst??


    }

    //AG120530: write to log
    int lineId = _gameLog->addGameStep(_hider.numberActions, _sentLastActionTime, _hider.timestamp, _seeker.timestamp, _seeker2.timestamp,
                          _hider.lastAction, _seeker.lastAction, _seeker2.lastAction,
                          _hider.currentPos, _seeker.currentPos, seeker2Pos,
                          _gameStatus, dsh, dsb, dhb, dshEuc, ds2h, ds2hEuc, dss2, dss2Euc,
                          vis_sh, vis_s2h, vis_ss2,
                          hiderPosWNoiseSent, hiderPosWNoiseSent2,
                          _seekerBeliefScore, _seeker2BeliefScore, _seekerReward, _seeker2Reward,
                          _seeker.multiGoalPosesVec, _seeker.multiGoalBeliefVec, _seeker.multiChosenPos,
                          _seeker2.multiGoalPosesVec, _seeker2.multiGoalBeliefVec, _seeker2.multiChosenPos
                        );

    _sentLastActionTime = QDateTime::currentDateTime(); //AG TODO: should maybe be in another place...

    return lineId;
}



void HSTCPServer::newGame() {
    if(_gameStatus == HSGlobalData::GAME_STATE_NOT_STARTED)
        return;//the game is already new!

    sleep(2);

    //disconnect sockets
    /*_hiderSocket->disconnect();
    _seekerSocket->disconnect();*/

    //AG120531: close (db/file/..)
    /*if (_gameLog!=NULL) _gameLog->close();*/

    //exit(0); //? TODO: reuse server (?)
    closeServer(0,false);
}


void HSTCPServer::initPlayers() {
    assert(_gmap!=NULL);

    _hider.clear();
    _seeker.clear();
    _seeker2.clear();
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

void HSTCPServer::writeGameParamsToStream(QDataStream &out, QByteArray &byteArray, char playerType, PlayerInfo *p2Info,
                                          PlayerInfo *p3Info, bool sendNoisePos) {

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
    //player type
    out << (quint8)playerType;
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
    //name of other agent(s)
    //AG150112: change to other agents
    out << p2Info->username;    
    QString p3UserName;
    if (p3Info!=NULL)
        p3UserName = p3Info->username;
    out << p3UserName;

    //AG140531: observ. noise std.dev.
    out << _params->simObsNoiseStd;
    //AG140605: false negative/positive
    out << _params->simObsFalseNegProb;
    out << _params->simObsFalsePosProb;
    //AG140606: send noise or not
    out << sendNoisePos;

    //put size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(byteArray.size() - sizeof(BLOCK_SIZE));
}

void HSTCPServer::sendParamsToPlayers() {
    //send to players the info of the map.
    DEBUG_SERVER(cout COUT_SHOW_ID  << "sending parameters and map to players"<<endl;);

    //-- data to hider --
    QByteArray blockToHider;
    QDataStream outHider(&blockToHider, QIODevice::WriteOnly);

    //-- data to seeker --
    QByteArray blockToSeeker;
    QDataStream outSeeker(&blockToSeeker, QIODevice::WriteOnly);

    //AG150112: params for seeker2
    if (_params->gameHas2Seekers()) {
        writeGameParamsToStream(outHider,blockToHider,HSGlobalData::PLAYER_TYPE_HIDER,&_seeker,&_seeker2,false);
        writeGameParamsToStream(outSeeker,blockToSeeker,HSGlobalData::PLAYER_TYPE_SEEKER,&_hider,&_seeker2,true);

        QByteArray blockToSeeker2;
        QDataStream outSeeker2(&blockToSeeker2, QIODevice::WriteOnly);
        writeGameParamsToStream(outSeeker2,blockToSeeker2,HSGlobalData::PLAYER_TYPE_SEEKER,&_hider,&_seeker,true);

        _seeker2Socket->write(blockToSeeker2);
        _seeker2Socket->flush();
    } else {
        writeGameParamsToStream(outHider,blockToHider,HSGlobalData::PLAYER_TYPE_HIDER,&_seeker,NULL,false);
        writeGameParamsToStream(outSeeker,blockToSeeker,HSGlobalData::PLAYER_TYPE_SEEKER,&_hider,NULL,true);
    }

    // -- send data --
    assert(_hiderSocket!=NULL);
    //cout <<"Sending params to hider, size="<<blockToHider.size()<<endl;
    _hiderSocket->write(blockToHider);//send to hider
    _hiderSocket->flush();
    assert(_seekerSocket!=NULL);
    //cout<<"Sending params to seeker, size="<<blockToSeeker.size()<<endl;
    _seekerSocket->write(blockToSeeker); //send to seeker
    _seekerSocket->flush();

    _hider.flag = _seeker.flag = _seeker2.flag = false;
    _seeker.multiHasGoalPoses = _seeker2.multiHasGoalPoses = false;
}

void HSTCPServer::readAndForwardMessage(QDataStream &in, PlayerInfo& sender) {
    //read from client
    QByteArray msg;
    quint8 to;
    in >> to;
    in >> msg; //CHECK if this is possible/correct

    DEBUG_SERVER(
        cout << "HSTCPServer::readMessage: message:"<<endl
             << "|From: "<<HSGlobalData::MESSAGESENDTO_NAMES[sender.playerType].toStdString() <<endl
             << "|To:   "<<HSGlobalData::MESSAGESENDTO_NAMES[to].toStdString() <<endl
             << "+---------------"<<endl
             << QString(msg).toStdString()<<endl
             << "+---------------<"<<endl
             << " Now sending ..."<<endl;
    );

    MessageSendTo msgSendTo = (MessageSendTo)to;

    if (msgSendTo!=MST_Server) {
        //prepare message for client(s)
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out.setVersion(HSGlobalData::DATASTREAM_VERSION);

        //block size
        out << (BLOCK_SIZE)0;
        //AG150120: message type
        out<<(MESSAGE_TYPE_SIZE)MT_Message;
        out<<to;
        out<<(quint8)sender.playerType; //from
        out<<msg;
        //put size
        out.device()->seek(0);
        out << (BLOCK_SIZE)(byteArray.size() - sizeof(BLOCK_SIZE));

        //now send to client(s)
        //hider
        if (msgSendTo==MST_All || msgSendTo==MST_AllClients || msgSendTo==MST_Hider) {
            _hiderSocket->write(byteArray);
            _hiderSocket->flush();
        }
        //seeker 1
        if (msgSendTo==MST_All || msgSendTo==MST_AllClients || msgSendTo==MST_AllSeekers || msgSendTo==MST_Seeker1) {
            _seekerSocket->write(byteArray);
            _seekerSocket->flush();
        }
        //seeker 2
        if (_seeker2Socket!=NULL && (msgSendTo==MST_All || msgSendTo==MST_AllClients || msgSendTo==MST_AllSeekers || msgSendTo==MST_Seeker2)) {
            _seeker2Socket->write(byteArray);
            _seeker2Socket->flush();
        }
    }

    if (msgSendTo==MST_Server || msgSendTo==MST_All) {
        //TODO handle here on server
    }
}

void HSTCPServer::readRobotGoalPoses(QDataStream &in, PlayerInfo &sender) {
    assert(_params->gameHas2Seekers());
    assert(!sender.multiHasGoalPoses);

    quint8 numGoals, numBels;
    in >> numGoals;
    assert(numGoals>0 && numGoals<=2);

    //read the goal poses
    sender.multiGoalPosesVec.resize(numGoals);
    for (size_t i=0; i<(size_t)numGoals; i++) {
        sender.multiGoalPosesVec[i].readPosFromStream(in,_params->useContinuousPos);
    }

    //get beliefs
    in >> numBels;
    assert(numBels==0 || numBels==numGoals);
    sender.multiGoalBeliefVec.resize(numBels);
    for (size_t i=0; i<(size_t)numBels; i++) {
        in >> sender.multiGoalBeliefVec[i];
    }

    sender.multiHasGoalPoses = true;
}

/*void HSTCPServer::readRobotChosenGoalPose(QDataStream &in, PlayerInfo &sender) {
    assert(_params->gameHas2Seekers());
    //assert(sender.multiHasGoalPoses);

    sender.multiChosenPos.readPosFromStream(in,_params->useContinuousPos);
}*/


void HSTCPServer::writeRobotGoalsToStream(QDataStream &out, QByteArray &byteArray, PlayerInfo& p2Info) {
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);

    //block size
    out << (BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_SeekerGoals;
    //num. goals
    out << (quint8)p2Info.multiGoalPosesVec.size();

    //write in reverse order, first the client's goal pos
    for(int i = (int)p2Info.multiGoalPosesVec.size()-1; i>=0; i--) {
        p2Info.multiGoalPosesVec[i].writePostoStream(out,_params->useContinuousPos);
    }

    //now the belief vector
    //num. beliefs
    out << (quint8)p2Info.multiGoalBeliefVec.size();

    //write in reverse order, first the client's goal pos
    for(int i = (int)p2Info.multiGoalBeliefVec.size()-1; i>=0; i--) {
        out << p2Info.multiGoalBeliefVec[i];
    }

    //put size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(byteArray.size() - sizeof(BLOCK_SIZE));
}

void HSTCPServer::sendRobotGoals() {
    assert(_params->gameHas2Seekers());
    assert(_seeker.multiHasGoalPoses && _seeker2.multiHasGoalPoses);

    //assert(_seeker.);
    QByteArray blockToSeeker1;
    QDataStream outSeeker1(&blockToSeeker1, QIODevice::WriteOnly);

    //-- data to seeker --
    QByteArray blockToSeeker2;
    QDataStream outSeeker2(&blockToSeeker2, QIODevice::WriteOnly);

    writeRobotGoalsToStream(outSeeker1, blockToSeeker1, _seeker2);
    writeRobotGoalsToStream(outSeeker2, blockToSeeker2, _seeker);

    // -- send data --
    assert(_seekerSocket!=NULL);
    _seekerSocket->write(blockToSeeker1); //send to seeker
    _seekerSocket->flush();
    assert(_seeker2Socket!=NULL);
    _seeker2Socket->write(blockToSeeker2); //send to seeker 2
    _seeker2Socket->flush();

    _seeker.multiHasGoalPoses = _seeker2.multiHasGoalPoses = false;
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
        //both hider and seeker pos should be set
        assert(_seeker.currentPos.isSet());
        assert(_hider.currentPos.isSet());

        //move the walkers, keeping in mind the current position of seeker and hider
        for(AutoWalker* walker: _walkerVector) {
            DEBUG_SERVER(cout COUT_SHOW_ID << "Init of autowalker "<<walker->getName()<<": "<<flush;);
            walker->initBelief(_gmap, _seeker.currentPos, _hider.currentPos, true);
            DEBUG_SERVER(cout << "ok"<<endl;);
        }
    }
}


void HSTCPServer::moveAutoWalkers(vector<IDPos>& nextPosAutoWalkers) {
    if (_walkerVector.size()>0) {
        //move the walkers, keeping in mind the current position of seeker and hider
        for(AutoWalker* walker: _walkerVector) {
            DEBUG_SERVER(cout COUT_SHOW_ID << "Next step of autowalker "<<walker->getName()<<": "<<flush;);
            vector<IDPos> posVec = walker->getAllNextPos(_seeker.currentPos, _hider.currentPos);
            nextPosAutoWalkers.insert(nextPosAutoWalkers.end(), posVec.begin(), posVec.end());
            DEBUG_SERVER(cout << "ok, "<<posVec.size()<<" poses"<<endl;);
        }
    }
}



GMap* HSTCPServer::getGMap() {
    return _gmap;
}

PlayerInfo* HSTCPServer::getHider() {
    return &_hider;
}

PlayerInfo* HSTCPServer::getSeeker() {
    return &_seeker;
}

//ag111201
void HSTCPServer::setServer(HSServer* server) {
    _server = server;
}

void HSTCPServer::setip(QString ip) {
    _ip = ip;
    //_params->serverIP = ip.toStdString();
}

void HSTCPServer::setport(int p) {
    _port=p;
    //_params->serverPort
}

void HSTCPServer::closeServer(int exitStatus, bool startNewServer) {
    //close db
    DEBUG_SERVER(cout<<"Closing server ... "<<flush;);
    close();
    if (_hiderSocket!=NULL && _hiderSocket->isOpen())
        _hiderSocket->close();
    if (_seekerSocket!=NULL && _seekerSocket->isOpen())
        _seekerSocket->close();
    if (_seeker2Socket!=NULL && _seeker2Socket->isOpen())
        _seeker2Socket->close();
    DEBUG_SERVER(cout<<"ok"<<endl<<"Closing log db ... "<<flush;);
    if (_gameLog!=NULL) {
        _gameLog->stopGame();
        _gameLog->close();
    }
    DEBUG_SERVER(cout<<"ok"<<endl;);
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
    if (_hiderSocket!=NULL && _hiderSocket->isOpen()) cout <<" hider";
    if (_seekerSocket!=NULL && _seekerSocket->isOpen()) cout <<" seeker";
    if (_seeker2Socket!=NULL && _seeker2Socket->isOpen()) cout <<" seeker2";
    cout<<", user names: h:"<<_hider.username.toStdString()<<",s:"
       <<_seeker.username.toStdString()<<",s2:"<<_seeker2.username.toStdString()<<endl;
}

void HSTCPServer::clientDisconnected() {
    DEBUG_SERVER(cout<<"Client disconnected - closing server in 2 s"<<endl);
    if (_hiderSocket!=NULL) disconnect(_hiderSocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    if (_seekerSocket!=NULL) disconnect(_seekerSocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    if (_seeker2Socket!=NULL) disconnect(_seeker2Socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    sleep(2);
    closeServer(0,true);
}

void HSTCPServer::acceptError(QAbstractSocket::SocketError s) {
    DEBUG_SERVER(cout COUT_SHOW_ID << "socket error - aborting all."<<endl;);//: "<< s.errorString() .toStdString()<<endl;);
    abort();
    _seekerSocket->abort();
    _seeker2Socket->abort();
    _hiderSocket->abort();
    exit(-2);
}
