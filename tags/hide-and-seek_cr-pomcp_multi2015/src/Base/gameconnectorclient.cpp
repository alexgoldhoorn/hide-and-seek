#include "Base/gameconnectorclient.h"

#include <cassert>
#include <iostream>

#include "Base/hsglobaldata.h"
#include "Base/hsconfig.h"

#include "Utils/generic.h"
#include "exceptions.h"
#include "mutex.h"

#ifndef SEEKERHS_H
#include "Base/seekerhs.h"
#endif

//AG140530 NOTE: for the block size use quint32 for all messages, see define BLOCK_SIZE in hsglobaldata.h

using namespace std;


GameConnectorClient::GameConnectorClient(SeekerHSParams* params, QString metaInfo, QString comments, AutoPlayer *autoPlayer, GMap *mapToSend
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                                        , SeekerHS* seekerHS
#endif
        ) {
    assert(params!=NULL);
    _params = params;

    init();

    SHS_IN_GC(_seekerHS = seekerHS;);
    _mapToSend = mapToSend;

    //AG150518: multi agent changes
    _autoPlayerVec.push_back(autoPlayer);
    _metaInfo = metaInfo;
    _comments = comments;
}

GameConnectorClient::GameConnectorClient(SeekerHSParams* params, QString metaInfo, QString comments, vector<AutoPlayer*> autoPlayerVec, GMap *mapToSend
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                                        , SeekerHS* seekerHS
#endif
        ) {
    assert(params!=NULL);
    _params = params;

    init();

    SHS_IN_GC(_seekerHS = seekerHS;);
    _mapToSend = mapToSend;
    _autoPlayerVec = autoPlayerVec;
    _metaInfo = metaInfo;
    _comments = comments;
}

GameConnectorClient::GameConnectorClient(SeekerHSParams* params, QString metaInfo, QString comments, Player* thisPlayer, GMap *mapToSend) {
    assert(params!=NULL);
    _params = params;

    init();

    _thisPlayer = thisPlayer;
    _mapToSend = mapToSend;
    _metaInfo = metaInfo;
    _comments = comments;
}


GameConnectorClient::~GameConnectorClient() {
    if (_tcpSocket!=NULL) delete _tcpSocket; //and close first (?)
    //if (_player!=NULL) delete _player;

    //AG150521: delete PlayerInfo objects created here, which
    //are only the ones for players that are not in this client (i.e. in the AutoPlayer.playerInfo)
    //first set to NULL the objects that are in the AutoPlayer
    for(AutoPlayer* autoPlayer : _autoPlayerVec) {
        assert(autoPlayer != NULL);
        int id = autoPlayer->playerInfo.id;
        //assert(id>=0); //ag150709: the id is to be set by the server, and may not (yet) have been set
        if (id>=0) {
            assert(id<(int)_playerInfoVec.size());
            _playerInfoVec[id] = NULL;
        }
    }

    //AG150525: delete player which contains a playerInfo
    if (_thisPlayer!=NULL) {
        //assert(_thisPlayer->playerInfo.id>=0); //ag150709: the id is to be set by the server, and may not (yet) have been set
        if (_thisPlayer->playerInfo.id>=0) {
            assert(_thisPlayer->playerInfo.id<(int)_playerInfoVec.size());
            _playerInfoVec[_thisPlayer->playerInfo.id] = NULL;
        }
    }

    //now delete the PlayerInfo objects which are left
    for(PlayerInfo* playerInfo : _playerInfoVec) {
        if (playerInfo!=NULL)
            delete playerInfo;
    }
}

void GameConnectorClient::init() {
    //_myInitPosSet = _oppInitPosSet = false;
    _initBeliefDone = false;
    _numMsgReceived = _numMsgSent = 0;
    _canDoAction = false;
    _tcpSocket = NULL;
    _serverParamsReceived = false;
    _exitOnDisconnect = false;
    _gmap = NULL;

    _tcpSocket = NULL;
    _thisPlayer = NULL;
    _hiderPlayer =  NULL;
    _seeker1Player = NULL;
    _seeker2Player = NULL;
}

bool GameConnectorClient::connectToServer() {
    assert(!_params->serverIP.empty() && _params->serverPort>0 /*&& _player!=NULL*/);

    //init vars
    _serverParamsReceived = false;
    _numMsgReceived = _numMsgSent = 0;
    _canDoAction = false;
    _gameStatus = HSGlobalData::GAME_STATE_NOT_STARTED;

    if (_tcpSocket!=NULL) {
        //first disconnect old socket
        if (_tcpSocket->isOpen()) _tcpSocket->close();
        delete _tcpSocket;
    }

    //create new socket
    _tcpSocket = new QTcpSocket(this);
    _tcpSocket->connectToHost(QString::fromStdString(_params->serverIP), _params->serverPort);
    if(_tcpSocket->waitForConnected(10000)) {
        /*DEBUG_CLIENT(*/ cout<<"Connected to server!"<<endl;//);

        //set disconnected
        connect(_tcpSocket, SIGNAL(disconnected()), this, SLOT(onServerDisconnect()));

        //send the username of the player along with the choises of the popup window.
        sendInfoToServer();

        /*DEBUG_CLIENT(*/cout << "Waiting for server "<<endl;//);

        //connect reading slot
        connect(_tcpSocket, SIGNAL(readyRead()), this, SLOT(serverReadyRead()));

        if (!_tcpSocket->isOpen())
            cout<<"error in socket connection"<<endl;

        return true;

    } else {
        cout << "Error, could not connect, try again."<<endl;

        return false;
    }
}

void GameConnectorClient::sendInfoToServer() {
    assert(_params!=NULL);

    //send the player and params to the server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    //block size
    out <<(BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_ClientRequestConn;

    //AG140212: first send game type
    out <<(quint8)_params->gameType;

    if (_mapToSend!=NULL) {
        DEBUG_CLIENT(cout << "GameConnectorClient::sendInfoToServer: sending map info: "<<_mapToSend->rowCount()<<"x"<<_mapToSend->colCount()<<endl;);
        //send map
        out << HSGlobalData::MAP_PASSED_BY_NETWORK;
        _mapToSend->writeMaptoStream(out);

    } else {
        out <<(quint16)_params->mapID;
    }

    //AG140531: send the distance matrix file if it is localhost
    QString mapDistMatFileStr;
    if (!_params->mapDistanceMatrixFile.empty() && _params->serverIP=="localhost") {
        mapDistMatFileStr = QString::fromStdString(_params->mapDistanceMatrixFile);
        DEBUG_CLIENT(cout<<"GameConnectorClient::sendInfoToServer: Sending distance map file name to server: "<<_params->mapDistanceMatrixFile<<endl;);
    }

    out << mapDistMatFileStr;

    //ag140130: continuous pos or not
    out << (quint8) (_params->useContinuousPos ? 1 : 0);

    //AG140131: always send params win dist
    out << (double)_params->winDist;

    //AG140506: obst dist
    out << (double)_params->minDistToObstacle;

    out <<(qint16)_params->opponentType;

    if (_params->opponentType==HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST
            || _params->opponentType==HSGlobalData::OPPONENT_TYPE_HIDER_FILE) {
        assert(!_params->oppActionFile.empty());

        out << QString::fromStdString(_params->oppActionFile);
        DEBUG_CLIENT(cout << "GameConnectorClient::sendInfoToServer: SENDING: opp="<<_params->oppActionFile<<"; actionFile="<<_params->oppActionFile<<endl;);
    }

    //ag131220: set randomPosdDistToHider
    out << (quint16)_params->randomPosDistToBaseHider;

    //AG140426: auto walker type and count
    out << (qint16)_params->autoWalkerType;
    out << (quint16)_params->autoWalkerN;
    //AG140605: autowalker pos file
    if (_params->autoWalkerType==HSGlobalData::AUTOWALKER_FILE) {
        out << QString::fromStdString(_params->autoWalkerPosFile);
    }

    //AG140608: send solver type
    out << (quint8)_params->solverType;

    //AG140520: stop after win, and num. of iterations to do
    out << _params->stopAtWin;
    if (!_params->stopAtWin || _params->maxNumActions<=0) {
        out << (quint16)_params->stopAfterNumSteps;
    } else {
        out << (quint16)_params->maxNumActions;
    }

    //AG150511: changed to qint8, and use enum
    if (_params->isSeeker)
        out <<(qint8) HSGlobalData::P_Seeker; // HSGlobalData::Player  PLAYER_TYPE_SEEKER;
    else
        out <<(qint8) HSGlobalData::P_Hider; // HSGlobalData::PLAYER_TYPE_HIDER;

    //AG150518: add number of players in client
    if (_autoPlayerVec.empty()) {
        out << (quint8)1; //only 1 player
    } else {
        out << (quint8)_autoPlayerVec.size();
    }

    //name
    out << QString::fromStdString(_params->userName); //_player->playerInfo.username; //QString::fromStdString(_player->getUsername());

    //AG140531: extra params to send
    out << _params->simObsNoiseStd;
    //AG140605: send noise
    out << _params->simObsFalseNegProb;
    out << _params->simObsFalsePosProb;

    //AG140531: extra meta info
    out << _metaInfo;
    //comments
    out << _comments;

    //AG150518: number of players required
    out << (quint8)_params->numPlayersReq;

    //AG150519: probability of choosing this client's obs
    out << _params->multiSeekerOwnObsChooseProb;

    //AG15151118: sees all
    out << (_params->solverType == SeekerHSParams::SOLVER_FOLLOWER_SEES_ALL);

    //block size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    /*cout << "SEND TO SERVER: block.size="<<block.size()<<", uint.sz="<<sizeof(quint32)<<", set size="<<(block.size() - sizeof(quint32))
         <<", as quint16:"<<((quint32) (block.size() - sizeof(quint32)) )<<endl;*/
    _tcpSocket->write(block);
    _tcpSocket->flush();

    DEBUG_CLIENT(cout<<"GameConnectorClient::sendInfoToServer: player sent his name : "<< _params->userName /*_player->getUsername()*/<<endl;);
}


void GameConnectorClient::readInitInfoFromServer(QDataStream& in) {
    DEBUG_CLIENT(cout << endl << "GameConnectorClient::readInitInfoFromServer: Reading data.."<<endl;);
    assert(!_serverParamsReceived);
    assert(_playerInfoVec.empty());
    assert(_hiderPlayer==NULL);

    //receive info from server
    quint8 useCont;
    quint16 mapID;
    //qint8 type; //AG150511: changed to qint8
    quint8 gameType;

    //AG140212: game type
    in >> gameType;
    _params->gameType = (char)gameType;

    //AG140130: continuous or not
    in >> useCont;
    _params->useContinuousPos = (useCont==(quint8)1);

    //max actions
    quint16 maxAct;
    in >> maxAct;
    _params->maxNumActions = (long)maxAct;

    //win dist
    in >> _params->winDist;

    //min dist to obstacle
    in >> _params->minDistToObstacle;

    //read map id
    in >> mapID;
    _params->mapID = (int)mapID;

    //ag131125: create new map
    if (_gmap!=NULL) {
        delete _gmap;
    }
    _gmap = new GMap(_params);
    _gmap->setUsingContActions(_params->useContinuousPos);
    //_player->setMap(_gmap);

    //read map
    _gmap->readMapFromStream(in);

    //load gmap distance matrix //AG140528 TODO: we should either send this through the network or some other manner
    if (_params->mapDistanceMatrixFile.length()>0) {
        DEBUG_CLIENT(cout<<"GameConnectorClient::readInitInfoFromServer: Loading distance matrix: "<<flush;);
        _gmap->readDistanceMatrixToFile(_params->mapDistanceMatrixFile.c_str());
        DEBUG_CLIENT(cout<<"ok"<<endl;);
    }

    //AG140531: sim obs. noise std.dev.
    in >> _params->simObsNoiseStd;
    //AG140605: send noise
    in >> _params->simObsFalseNegProb;
    in >> _params->simObsFalsePosProb;

    //AG150518
    //get all player details
    quint8 numPlayers;
    in >> numPlayers;

    //AG150719
    _params->numPlayersReq = numPlayers;

    //resize players info
    assert(numPlayers>1);

    quint8 numPlayersInClient;
    in >> numPlayersInClient;

    if (numPlayersInClient!=_params->numPlayersInClient || (!_autoPlayerVec.empty() && _autoPlayerVec.size()!=numPlayersInClient)) {
        throw CException(_HERE_, "the number of players in this client ("+QString::number(numPlayersInClient).toStdString()
                                    +"), and number of auto players is "+QString::number(_autoPlayerVec.size()).toStdString()
                                    +", but server sends: "+QString::number(numPlayersInClient).toStdString()+".");
    }
    assert(numPlayers>=numPlayersInClient);

    //now set the size of the player vector
    _playerInfoVec.resize(numPlayers,NULL);

    //read all own players
    quint8 pid,ptype;
    double usePlayerObsP;
    QString pname;

    assert(numPlayersInClient>0);

    for (quint8 i=0; i<numPlayersInClient; i++) {
        //get id
        in >>pid;
        assert(pid<_playerInfoVec.size());
        //get player info
        PlayerInfo* playerInfo;

        //AG150609: the playerInfo should be not set yet
        if (_playerInfoVec[pid]!=NULL) {
            throw CException(_HERE_,"an id for a player has been repeated: "+QString::number(pid).toStdString());
        }

        if (_autoPlayerVec.empty()) {
            //not using autoplayer, only 'manual'
            assert(_thisPlayer!=NULL);
            playerInfo = _playerInfoVec[pid] = &_thisPlayer->playerInfo;
        } else {
            //using autoplayers
            playerInfo = _playerInfoVec[pid] = &_autoPlayerVec[i]->playerInfo;
        }

        assert(playerInfo->id==-1);//should not yet been set
        playerInfo->id = (int)pid;
        //get type
        in >>ptype;
        playerInfo->playerType = (HSGlobalData::Player)ptype;
        //name
        in >> pname;
        playerInfo->username = pname; // setUserName(pname);
        //prob. of choosing this player's obs
        in >> usePlayerObsP;
        playerInfo->useObsProb = usePlayerObsP;
        //set not yet received pos
        playerInfo->posRead = false;

        //set player info autoplayer
        if (!_autoPlayerVec.empty()) {
            //_autoPlayerVec[i].setPlayerInfo(player);
            //add to map
            assert(getAutoPlayer(pid)==NULL);
            _autoPlayerMap[pid] = _autoPlayerVec[i];
        }

        //check hider
        if (ptype == HSGlobalData::P_Hider) {
            if (_hiderPlayer!=NULL) {
                throw CException(_HERE_, "a hider player already has been set, but a new one is received (in own)");
            }
            _hiderPlayer = playerInfo;
        } else if(_seeker1Player==NULL) {
            if (ptype != HSGlobalData::P_Seeker) {
                throw CException(_HERE_, "expected a seeker player");
            }
            _seeker1Player = playerInfo;
        }
    } // for players in client

    DEBUG_CLIENT(
        if (_hiderPlayer!=NULL && _autoPlayerVec.size()>1) throw CException(_HERE_, "the current client has a hider, but also other players, this is not (yet) allowed");
    );

    //all other players
    for (quint8 i=0; i<numPlayers - numPlayersInClient; i++) {
        //get id
        in >> pid;
        assert(pid<_playerInfoVec.size());

        //AG150609: the playerInfo should be not set yet
        if (_playerInfoVec[pid]!=NULL) {
            throw CException(_HERE_,"an id for a player has been repeated: "+QString::number(pid).toStdString());
        }

        //get player info
        PlayerInfo* playerInfo = _playerInfoVec[pid] = new PlayerInfo();
        assert(playerInfo->id==-1);//should not yet been set
        playerInfo->id = pid;
        //get type
        in >>ptype;
        playerInfo->playerType = (HSGlobalData::Player)ptype;
        //name
        in >> pname;
        playerInfo->username = pname; // setUserName(pname);
        //prob. of choosing this player's obs
        in >> usePlayerObsP;
        playerInfo->useObsProb = usePlayerObsP;
        //set not yet received pos
        playerInfo->posRead = false;

        //check hider
        if (ptype == HSGlobalData::P_Hider) {
            if (_hiderPlayer!=NULL) {
                throw CException(_HERE_, "a hider player already has been set, but a new one is received (in others)");
            }
            _hiderPlayer = playerInfo;
        } else {
            if (ptype != HSGlobalData::P_Seeker) {
                throw CException(_HERE_, "expected a seeker player");
            }
            if(_seeker1Player==NULL) {
                _seeker1Player = playerInfo;
            } else if(_seeker2Player==NULL) {
                _seeker2Player = playerInfo;
            }
        }
    } // for other players


    assert(_seeker1Player!=NULL);
    assert(_seeker1Player->id>=0);
    assert(_seeker1Player->isSeeker());
    assert(_hiderPlayer!=NULL);
    assert(!_hiderPlayer->isSeeker());
    assert(_seeker2Player==NULL || _seeker2Player->isSeeker());

    //debug check if all players have PlayerInfo, and if the player id is consistent
    DEBUG_CLIENT(
        /*for (AutoPlayer* autoPlayer : _autoPlayerVec) {
            if (autoPlayer->getPlayerInfo()==NULL) throw CException(_HERE_,"an autoplayer does not contain a playerInfo");
        }*/

        for (PlayerInfo* playerInfo : _playerInfoVec) {
            assert(playerInfo!=NULL);
            if (playerInfo->id<0) throw CException(_HERE_,"player "+playerInfo->toString(true)+" does not have an ID!");
            if (*playerInfo!=*_playerInfoVec[playerInfo->id]) throw CException(_HERE_,"player "+playerInfo->toString(true)
                    +" does not coincide with the player of same id in the vector: "+_playerInfoVec[playerInfo->id]->toString());
        }

        if (_hiderPlayer==NULL) throw CException(_HERE_,"no hider has been sent");
    );

    _numMsgReceived++;

    //AG150526: show who is playing
    cout << "Starting game, players from this client ";
    if (_autoPlayerVec.empty()) {
        //only this player
        cout<<"[manual]: ";
        assert(_thisPlayer!=NULL);
        cout<<_thisPlayer->playerInfo.toString();
    } else {
        //show autoplayers
        cout<<"[auto]:";
        for(AutoPlayer* autoPlayer : _autoPlayerVec) {
            cout <<" "<< autoPlayer->playerInfo.toString();
        }
    }

    //now show other players
    cout<<endl<<"Other players:";
    for (PlayerInfo* playerInfo : _playerInfoVec) {
        if (getAutoPlayer(playerInfo->id)==NULL && (_thisPlayer==NULL || _thisPlayer->playerInfo!=*playerInfo)) {
            //not a player of this client
            cout << " " << playerInfo->toString();
        }
    }
    cout << endl;

    if (!_tcpSocket->isOpen())
        cout<<"error in socket connection"<<endl;

    //AG150520: server params received
    _serverParamsReceived = true;

    emit serverParamsRead();

    //now send the new pos (if we have the player)
    if (_autoPlayerVec.size()>0) {
        assert(_gmap!=NULL);

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
        if (_seekerHS!=NULL) {
            _seekerHS->setMap(_gmap);
        } else {
#endif
            for(AutoPlayer* autoPlayer : _autoPlayerVec) {
                autoPlayer->setMap(_gmap);
            }
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
        }
#endif

        //set new init pos
        DEBUG_CLIENT(cout<<"GameConnectorClient::readInitInfoFromServer: Calculating init pos: "<<endl;);
        for(AutoPlayer* autoPlayer : _autoPlayerVec) {
            PlayerInfo* playerInfo = &autoPlayer->playerInfo;
            playerInfo->currentPos = autoPlayer->getInitPos();
            DEBUG_CLIENT(cout <<" - "<<playerInfo->toString(true)<<endl;);
        }
        //DEBUG_CLIENT(cout<<_player->getCurPos().toString()<<endl;);
        //now call send pos
        sendInitPos();
    }
}


void GameConnectorClient::serverReadyRead() {
    try {
        BLOCK_SIZE blockSize=0;
        QDataStream in(_tcpSocket);
        in.setVersion(HSGlobalData::DATASTREAM_VERSION);

        if (_tcpSocket->bytesAvailable() < (int)sizeof(BLOCK_SIZE)) {
            DEBUG_CLIENT(cout<<"GameConnectorClient::serverReadyRead: less bytes in the array than expected1."<<endl;);
            return;
        }
        in >> blockSize;

        if (_tcpSocket->bytesAvailable() < blockSize) {
            DEBUG_CLIENT(cout<<"GameConnectorClient::serverReadyRead: less bytes in the array than expected2. blocksize="<<blockSize<<endl;);
            return;
        } else if (blockSize==0) {
            DEBUG_CLIENT(cout<<"GameConnectorClient::serverReadyRead: unexpected: blockSize=0"<<endl;);
            return;
        }

        //AG150120: read message type
        MESSAGE_TYPE_SIZE messageType;
        in >> messageType;

        if (!_serverParamsReceived) {
            if ((MessageType)messageType==MT_GameParams) {
                readInitInfoFromServer(in);
            } else {
                cout << "ERROR: expected game parameters, but received another message type ("<<(int)messageType<<")"<<endl;
            }

        } else {
            //AG150120: check message type
            DEBUG_CLIENT(cout<<"GameConnectorClient::serverReadyRead: message type:";);
            switch ((MessageType)messageType) {
                case MT_Update:
                    DEBUG_CLIENT(cout<<"read server update"<<endl;);
                    readServerUpdate(in);
                    break;
                case MT_SeekerGoals:
                    DEBUG_CLIENT(cout<<"seeker goals"<<endl;);
                    assert(_params->isSeeker);
                    readSeeker2GoalsAndBeliefs(in);
                    break;
                case MT_SeekerHB:
                    DEBUG_CLIENT(cout<<"seeker hb"<<endl;);
                    assert(_params->isSeeker);
                    readSeekerHB(in);
                break;
                case MT_Message:
                    DEBUG_CLIENT(cout<<"read message"<<endl;);
                    readMessage(in);
                    break;
                default:
                    cout << "ERROR - unknown message type ("<<(int)messageType<<")"<<endl;
                    break;
            }
        }

    } catch(CException& ce) {
        cout << "GameConnectorClient::serverReadyRead: Exception: "<<ce.what()<<endl;
    }
}

void GameConnectorClient::readServerUpdate(QDataStream &in) {
    assert(_serverParamsReceived);

    quint16 validation, gameStatus,numDynObst;

    in >> validation; //validation byte
    if(validation==0) { //validation int
        //that means the previous action was not done from the server
        cout << "Error: received response 'invalid' from server"<<endl;
    }

    in >> gameStatus; //status byte

    _gameStatus = (int)gameStatus;

    //AG150519: changed for multi agents
    quint8 numPlayers;
    in >> numPlayers;

    //AG150609: now prepare each playerInfo for next step (set prev pos, and unset flags)
    for(PlayerInfo* playerInfo : _playerInfoVec) {
        playerInfo->prepareNextStep();
    }

    quint8 pid;
    //get all poses
    for (quint8 i=0; i<numPlayers; i++) {
        //get id
        in >> pid;
        assert(pid<_playerInfoVec.size());
        //get player
        PlayerInfo* playerInfo = _playerInfoVec[pid];

       //set pos and noisy pos
        playerInfo->currentPos.readPosFromStream(in, _params->useContinuousPos);
        playerInfo->currentPosWNoise.readPosFromStream(in, _params->useContinuousPos);
        //positions have been read if the 'no communication flag' (for debugging) has not been set
        //and if set, then still the own positions are read
        if (!_params->multiSeekerNoCommunication || (!_autoPlayerVec.empty() && getAutoPlayer(pid)!=NULL)) {
            playerInfo->posRead = true;
            if (!playerInfo->initPosSet)
                playerInfo->initPosSet=true;
        }
        //set other flags to false
        //player->multiChosenGoalPos.clear();
        //player->multiHasGoalPoses = player->multiHasHBPoses = false;
    }

    //get hider noisy pos for the seekers
    quint8 numPlayersS;
    in >> numPlayersS;

    for (quint8 i=0; i<numPlayersS; i++) {
        //get id
        in >> pid;
        assert(pid<_playerInfoVec.size());
        //set pos and noisy pos
        _playerInfoVec[pid]->hiderObsPosWNoise.readPosFromStream(in, _params->useContinuousPos);
        //ag150611: read prob. of trusting this obs
        in >> _playerInfoVec[pid]->useObsProb; // hiderObsTrustProb;

        //id should be a player for this client //AG150711: NO we should receive observations from all!
        //assert(_autoPlayerVec.empty() || getAutoPlayer(pid)!=NULL);
    }

    //AG140506: get all auto walkers/dynamic obst
    //first count
    in >> numDynObst;

    _dynObstPosVecMutex.enter();
    _dynObstPosVec.resize(numDynObst);
    //now read all
    for(IDPos& p: _dynObstPosVec) {
        p.readPosFromStream(in, _params->useContinuousPos);
    }
    //set dyn. obst to gmap
    _gmap->setDynamicObstacles(_dynObstPosVec);
    _dynObstPosVecMutex.exit();

    DEBUG_CLIENT(
        cout << "Dyn obstacles (#"<<numDynObst<<"):"<<endl;
        for(const IDPos & p : _gmap->getDynamicObstacles()) {
            cout<<" - "<<p.toString()<<endl;
        }
    );

    //increase number of received actions
    _numMsgReceived++;
    //the player can now send the next action
    _canDoAction = true;

    //emit
    emit serverUpdateReceived(_gameStatus); //TODO: should this be after init belief?

    DEBUG_CLIENT(
        cout << "GameConnectorClient::readServerUpdate: RECEIVED: status="<<WINSTATE_COUT(_gameStatus) <<", players: ";
        for(PlayerInfo* playerInfo : _playerInfoVec) {
            cout<<playerInfo->toString(true,true,true)<<", ";
        }
        cout << endl;
    );

    if(_gameStatus > HSGlobalData::GAME_STATE_RUNNING) {
        DEBUG_CLIENT_VERB(cout<<"GAME OVER!"<<endl;);
        //_tcpSocket->disconnect();
        disconnectFromServer();

    } else if (_autoPlayerVec.size()>0 //_autoPlayer!=NULL
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
               || _seekerHS!=NULL
#endif
               ) {
#ifdef OLD_CODE
        //send new pos action if its auto player
        Pos curPos(_player->getCurPos());
        IDPos oppPos( (_receiveNoisyPos ? _player->getHiderObsWNoise() : _player->getPlayer2Pos()), 0);
        //AG150218: always do the check because the oppPos.isSet() method could be inconsistent with 'posible' visibility due to added noise
        bool isVisib = oppPos.isSet();
        if (isVisib) isVisib = _gmap->isVisible(curPos,oppPos,true);

        //AG150224: if not visible (due to auto walkers for example)
        if (!isVisib) oppPos.clear();

        assert(isVisib==oppPos.isSet());

        //AG150202: opponent position seen by other player (i.e. hider's pos as seen by seeker 2)
        Pos oppPos2;
        Pos seeker2Pos = _player->getPlayer3Pos();

        if (_params->seekerSendsMultiplePoses() && _receiveNoisyPos) {
            //AG150204: assert that the pos is noisy (although maybe we want to disable this)
            //assert(_receiveNoisyPos);
            oppPos2 = _player->getHiderObs2WNoise();
        }
#endif

        //AG150519: if the init poses have not been received yet, call the init belief
        if(!_initBeliefDone) {
            //if its the seeker on the first msg then read the initial pos of the hider
            initBelief();
        }

        DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: Get new poses ";);
        if (_params->isSeeker) {
            DEBUG_CLIENT(cout << "seeker "<<endl;);
            calcNextPosSeeker();

        } else { // is not a seeker
            DEBUG_CLIENT(cout << "hider "<<endl;);
            calcNextPosHider();
        }


        //belief to image
        if (!_params->beliefImageFile.empty()
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                && _seekerHS==NULL
#endif
                ) {
            //AG150526: write belief file using first autoplayer
            //_autoPlayer->storeMapBeliefAsImage(_params->beliefImageFile,curPos, &seeker2Pos, oppPos, &oppPos2, _params->beliefImageCellWidth);
            _autoPlayerVec[0]->storeMapBeliefAsImage(_params->beliefImageFile, /*curPos, &seeker2Pos, oppPos, &oppPos2,*/ _params->beliefImageCellWidth);
        }

        DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: ";);

        if (_params->isSeeker) {
            switch(_params->solverType) {
                case SeekerHSParams::SOLVER_TWO_HB_EXPL: {
                    DEBUG_CLIENT(cout << "send multi robot goals"<<endl;);
                    sendRobotGoalsAndBeliefs();

                    break;
                }
                case SeekerHSParams::SOLVER_MULTI_HB_EXPL: {
                    DEBUG_CLIENT(cout << "send highest beliefs"<<endl;);
                    sendHBs();

                    break;
                }
                default: {
                    DEBUG_CLIENT(cout << "next robot poses"<<endl;);
                    sendPos();

                    break;
                }
            }
        } else {
            //if hider, send pos directly
            DEBUG_CLIENT(cout << "next hider poses"<<endl;);
            sendPos();
        }

    } // AutoPlayer != NULL

}


void GameConnectorClient::initBelief() {
    assert(!_initBeliefDone);

    //AG150522: code from readServerUpdate, split in function to reduce function size

    //if( !_oppInitPosSet ) { //if its the seeker on the first msg then read the initial pos of the hider
    if (_params->isSeeker) {
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
        if (_seekerHS!=NULL) {      //TODO: change init belief, and ONLY use seekersh (?)

            PosXY seekerPosXY;
            PosXY hiderObsPosXY;
            vector<PosXY> otherSeekerPosXYVec;
            vector<PosXY> otherSeekerHiderObsPosXYVec;
            getSeerkHSPosXYForAll(seekerPosXY, hiderObsPosXY, otherSeekerPosXYVec, otherSeekerHiderObsPosXYVec);

            //AG150202: check if two seekers
            if (_params->seekerSendsMultiplePoses()) {
                _seekerHS->initMultiSeeker(seekerPosXY, hiderObsPosXY, otherSeekerPosXYVec, otherSeekerHiderObsPosXYVec);

            } else {
                //_autoPlayer->initBelief(_gmap,curPos,oppPos,isVisib);
                //_seekerHS->init(curPos.toVector(),ov);
                _seekerHS->init(seekerPosXY, hiderObsPosXY);
            }
        } else
#endif
            //AG150202: check if two seekers and not checking without communcation
            if (_params->seekerSendsMultiplePoses() /*&& !_params->multiSeekerNoCommunication*/) {
                //init belief with 2 seekers
                //_autoPlayer->initBelief2(_gmap,curPos,oppPos,isVisib,seeker2Pos,oppPos2,_params->multiSeekerOwnObsChooseProb);

                //AG150520: init all beliefs
                DEBUG_CLIENT(cout<<"=== INIT BELIEF (multi) ==="<<endl;);
                for(AutoPlayer* autoPlayer : _autoPlayerVec) {
                    //assert(autoPlayer->getPlayerInfo()!=NULL);
                    DEBUG_CLIENT(cout<<"--- init b(mul): "+autoPlayer->playerInfo.toString(true,true,true)+" ---"<<endl;);
                    autoPlayer->initBeliefMulti(_gmap, _playerInfoVec, autoPlayer->playerInfo.id, _hiderPlayer->id);
                }

            } else {
                //_autoPlayer->initBelief(_gmap,curPos,oppPos,isVisib);
                //AG150520: init all beliefs
                DEBUG_CLIENT(cout<<"=== INIT BELIEF (single) ==="<<endl;);
                for(AutoPlayer* autoPlayer : _autoPlayerVec) {
                    //assert(autoPlayer->getPlayerInfo()!=NULL);
                    DEBUG_CLIENT(cout<<"--- init b(single): "+autoPlayer->playerInfo.toString(true,true,true)+" ---"<<endl;);
                    autoPlayer->initBelief(_gmap, _hiderPlayer);
                }
            }

    } else { // is a hider:
        //AG150520: for now we assume that the hider client only has 1 player
        assert(_autoPlayerVec.size()==1);

#ifdef USE_HIDER_AS_AUTOPLAYER
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
        if (_seekerHS!=NULL) {  //TODO: change init belief, and ONLY use seekersh (?)
            throw CException(_HERE_,"adapt to new multi agent structure");
            _seekerHS->init(oppPos.toVector(),curPos.toVector());
        } else
#endif

        if (_params->numPlayersReq>2) {
            _autoPlayerVec[0]->initBeliefMulti(_gmap, _playerInfoVec, _autoPlayerVec[0]->playerInfo.id, _hiderPlayer->id); //note: ids should be same
        } else {
            //_autoPlayer->initBelief(_gmap,oppPos,curPos,isVisib);
            _autoPlayerVec[0]->initBelief(_gmap, _seeker1Player);
        }
#endif
    }

    //AG150520: init belief done
    _initBeliefDone = true;
}

void GameConnectorClient::calcNextPosSeeker() {
    assert(_initBeliefDone);

    //AG150522: code from readServerUpdate, split in function to reduce function size

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    if (_seekerHS!=NULL) {

        PosXY seekerPosXY;
        PosXY hiderObsPosXY;
        vector<PosXY> otherSeekerPosXYVec;
        vector<PosXY> otherSeekerHiderObsPosXYVec;
        getSeerkHSPosXYForAll(seekerPosXY, hiderObsPosXY, otherSeekerPosXYVec, otherSeekerHiderObsPosXYVec);

        bool ok = false;
        int winState = -1;

        //AG150521: run based on type of solver
        switch(_params->solverType) {
            case SeekerHSParams::SOLVER_TWO_HB_EXPL: {
                PosXY goal1PosXY,goal2PosXY;
                if (otherSeekerPosXYVec.empty()) {
                    assert(otherSeekerHiderObsPosXYVec.empty());
                    _seekerHS->calcNextMultiRobotPoses2(seekerPosXY, hiderObsPosXY, NULL, NULL,
                        goal1PosXY, goal2PosXY, &winState);
                } else {
                    assert(otherSeekerHiderObsPosXYVec.size()==otherSeekerPosXYVec.size());
                    _seekerHS->calcNextMultiRobotPoses2(seekerPosXY, hiderObsPosXY, &otherSeekerPosXYVec[0], &otherSeekerHiderObsPosXYVec[0],
                        goal1PosXY, goal2PosXY, &winState);
                }

                assert(_seeker1Player->multiHasGoalPoses);
                assert(!_seeker1Player->multiGoalBPosesVec.empty());
                BPos goal1BPos = goal1PosXY.toBPos(_params);
                assert(_seeker1Player->multiGoalBPosesVec[0]==goal1BPos);
                if (goal2PosXY.isSet()) {
                    BPos goal2BPos = goal2PosXY.toBPos(_params);
                    assert(_seeker1Player->multiGoalBPosesVec[1]==goal2BPos);
                }

                break;
            }
            case SeekerHSParams::SOLVER_MULTI_HB_EXPL: {
                vector<PosXY> hbVec;
                if (otherSeekerPosXYVec.empty()) {
                    assert(otherSeekerHiderObsPosXYVec.empty());
                    _seekerHS->calcMultiHBs(seekerPosXY, hiderObsPosXY, NULL, NULL,
                        hbVec, &winState);
                } else {
                    assert(otherSeekerHiderObsPosXYVec.size()==otherSeekerPosXYVec.size());
                    _seekerHS->calcMultiHBs(seekerPosXY, hiderObsPosXY, &otherSeekerPosXYVec[0], &otherSeekerHiderObsPosXYVec[0],
                        hbVec, &winState);
                }

                assert(_seeker1Player->multiHasHBPoses);
                assert(_seeker1Player->multiHBPosVec.size()>0);
                assert(_seeker1Player->multiHBPosVec.size()==hbVec.size());
                //assert(playerInfo->multiHBPosVec.size()==playerInfo->multiHBBeliefVec.size());

                break;
            }
            default: {

                PosXY newSeekerPosXY;
                int winState=-1;
                int a = _seekerHS->getNextPos(seekerPosXY, hiderObsPosXY, newSeekerPosXY, &winState);

                assert(_seeker1Player->nextPos==newSeekerPosXY.toPos(_params));

                //DEBUG_CLIENT(cout << "GameConnectorClient::calcNextPosSeeker: get next robot pose"<<endl;);
                //calculate the next pos (also stored in PlayerInfo)
                //Pos newPos = autoPlayer->getNextPos(actionDone); //(curPos,oppPos,isVisib,actions,actionDone,_params->useNextNPos);
                        //_autoPlayer->getNextPosWithFilter(curPos,personVec,actions,lastA,_params->useNextNPos,dontExec);
                //_player->setCurPos(newPos);
                //DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: got next robot pos: "<<newPos.toString()<<endl;);
                break;
            }
        } // switch solverType

#ifdef OLD_CODE

        vector<double> nextPos;
        int winState = -1;

        /* //FOR TESTING 'getNextPoseForMultipleObs'
        vector<vector<double>> vecvec;
        if (isVisib) {
            vector<double> ov = oppPos.toVector();
            ov.push_back(1);
            vecvec.push_back(ov);
            for(int i=0;i<3;i++) {
                vector<double> vrand;

                vrand.push_back(randomDouble(-2,_gmap->rowCount()+2));
                vrand.push_back(randomDouble(-2,_gmap->colCount()+2));
                vrand.push_back(1);
                vecvec.push_back(vrand);
            }
        } else {
            //empty, no obs
        }

        _seekerHS->getNextPoseForMultipleObs(curPos.toVector(), vecvec, nextPos, &winState);
        */ //test
        try { //AG140307: ALL THIS is for debug of SeekerHS!

            //_seekerHS->getNextPose(curPos.toVector(), oppPos.toVector(), nextPos, &winState);
            //vector<double> ov;
            vector<vector<double>> ovVec;
            if (isVisib) {
                //ov = oppPos.toVector();
                vector<double> ovv = oppPos.toVector();
                ovv.push_back(0);
                ovVec.push_back(ovv);
            }

            double r, c;

            vector<double> curVec = curPos.toVector();
            curVec.push_back(0);

            if (debugAddNewTracksManually) {
                //reset seekerpos for debug
                cout << "seekerpos="<<curPos.toString()<<", reset: (-1=keep original)"<<endl;
                cout <<"row: ";
                cin >> r;
                if (r!=-1) {
                    cout <<"col: ";
                    cin >> c;
                    if (c!=-1) {
                        curVec[0] = r;
                        curVec[1] = c;
                    }
                }

                cout<<"oppPos="<<oppPos.toString()<<", add opp pos (-1=stop):"<<endl;
                //add opponent pos for debug
                do {
                    cout << "row: ";
                    cin >> r;
                    if (r!=-1) {
                        cout << "col: ";
                        cin >> c;
                    }
                    if (c!=-1 && r!=-1) {
                        vector<double> vec;
                        vec.push_back(r);
                        vec.push_back(c);
                        vec.push_back(0);
                        ovVec.push_back(vec);
                    }
                } while (r!=-1 && c!=-1);
            }


            //AG150210: first filter
            vector<double> curVecFil, ovVecFil;
            _seekerHS->filterMultipleObs(curVec, ovVec, curVecFil, ovVecFil);


            //AG150202: 2 seekers option
            if (_params->seekerSendsMultiplePoses()) {
                DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: get next robot poses (SeekerHS)"<<endl;);

                vector<double> goalVecMine, goalVecOther;
                double goalBelMine,goalBelOther;
                if (_params->multiSeekerNoCommunication) {
                    _seekerHS->getNextMultiSeekerPoses(curVecFil, ovVecFil, vector<double>(), vector<double>(), _params->useNextNPos,
                                                       goalVecMine, goalVecOther, goalBelMine, goalBelOther, NULL);
                } else {
                    _seekerHS->getNextMultiSeekerPoses(curVecFil, ovVecFil, seeker2Pos.toVector(), oppPos2.toVector(), _params->useNextNPos,
                                                       goalVecMine, goalVecOther, goalBelMine, goalBelOther, NULL);
                }

                assert(goalVecMine.size()>=2);
                //now convert to pos vectors (for use in GameConnector)
                _player->goalPosesVec.resize(1);
                _seekerHS->setPosFromVector(_player->goalPosesVec[0], goalVecMine);
                if (goalVecOther.size()>=2) {
                    _player->goalPosesVec.resize(2);
                    _seekerHS->setPosFromVector(_player->goalPosesVec[1], goalVecOther);

                    if (goalBelMine>=0) {
                        _player->goalPosesBelVec.resize(1);
                        _player->goalPosesBelVec[0] = goalBelMine;
                        if (goalBelOther>=0) {
                            _player->goalPosesBelVec.resize(2);
                            _player->goalPosesBelVec[1] = goalBelOther;
                        }
                    }
                }


                DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: " //<< (ok?"ok":"not ok")
                             <<" received "<<_player->goalPosesVec.size()
                             <<" robot poses (and "<< _player->goalPosesBelVec.size() <<" belief points), now sending to server. (SeekerHS)"<<endl;);

            } else {
                DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: get next robot pose"<<endl;);
                //Pos newPos = _autoPlayer->getNextPos(curPos,oppPos,isVisib,actions,lastA,_params->useNextNPos);
                        //_autoPlayer->getNextPosWithFilter(curPos,personVec,actions,lastA,_params->useNextNPos,dontExec);
                //_player->setCurPos(newPos);

                //_seekerHS->getNextMultiplePoses(curPos.toVector(), ov /* oppPos.toVector()*/, 1, nextPos, &winState);
                _seekerHS->getNextMultiplePosesForMultipleObs(curVec, ovVec, _params->useNextNPos, nextPos, &winState);

                //_seekerHS->setPosFromVector(newPos, nextPos);

                //DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: got next robot pos: "<<nextPos.toString()<<endl;);

                DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: Win state: "<<winState<<", next pos: "<<nextPos[0]
                                    <<","<<nextPos[1]<<endl;);

                _player->setCurPos(nextPos[0], nextPos[1]);
            }

            //chosen hider pos
            //vector<double> chosenHiderPos = _autoPlayer->get->getChosenHiderPos();
            IDPos chosenHiderPos = _autoPlayer->getLastHiderPos();
            cout << "Chosen hider pos: ";
            if (chosenHiderPos.isSet()) {
                cout << chosenHiderPos.toString()<<endl;
            } else {
                cout << "hidden"<<endl;
            }
        } catch(CException& ce) {
            cout << "Exception: "<<ce.what()<<endl;
        }

#endif

    } else { //No SeekerHS set ->
#endif
        //vector<int> actions;
        int actionDone = -1;
        /*if (_params->useDeducedAction) {
            lastA = _autoPlayer->deduceAction(_lastPos,curPos);
        }*/

        //first copy vector
        _dynObstPosVecMutex.enter();
        vector<IDPos> dynObstVec = _dynObstPosVec;
        _dynObstPosVecMutex.exit();

        //AG150813: if simulating no comm. -> set as not read
        if (_params->multiSeekerNoCommunication) {
            for(PlayerInfo* playerInfo : _playerInfoVec) {
                if (playerInfo->isSeeker())
                    playerInfo->posRead = false;
            }
        }

        //AG150520: for each player calculate the new action
        DEBUG_CLIENT(cout << "=== New Action ==="<<endl;);
        for (AutoPlayer* autoPlayer : _autoPlayerVec) {
            //player info
            PlayerInfo* playerInfo = &autoPlayer->playerInfo;

            //AG150813: set posread for this player (if no multi seeker pos)
            if (_params->multiSeekerNoCommunication)
                playerInfo->posRead = true;

            //init with no next pos, should be set by sendPos (if sent)
            assert(!playerInfo->nextPos.isSet());

            DEBUG_CLIENT(cout << "--- Act: "<<playerInfo->toString(true,true,true)<<" ---"<<endl;);

            //hider pos
            IDPos hiderPos(playerInfo->hiderObsPosWNoise,0); //convert to IDPos

            //calculate what the real last action was
            actionDone = autoPlayer->deduceAction();

            DEBUG_CLIENT(
                cout<<"Deduced action: ";
                if(actionDone<0)
                    cout <<"none ("<<actionDone<<")"<<endl;
                else
                    cout<<ACTION_COUT(actionDone)<<endl;
            );
            bool dontExec = false;

            //AG140512: simulate several tracks as potential hiders
            if (_params->simulateReceivingAllTracksAsPersons) {
                //copy dyn. obst vector as list of persons visible
                vector<IDPos> personVec = dynObstVec;

                //check if hider visible check)
                if (hiderPos.isSet()) {
                    //Add to list
                    personVec.push_back(hiderPos);
                }

                if (_params->simulateNotVisible) {
                    //simulate that tracks of 'persons' that are not visible due to obstacles, are not sent to the AutoPlayer
                    //AG140526: not necessary to also simulate lack of visibility due to dyn obst.
                    for(size_t i=0; i<personVec.size(); i++) {
                        if (!_gmap->isVisible(playerInfo->currentPos, personVec[i], false)) {
                            personVec.erase(personVec.begin()+i);
                            i--;
                        }
                    }
                }

                //AG140526: if we don't want to use dyn. obstacles in simulations for learning, then remove them here...
                //          NOTE: also if's in the simulator code at calling GMap.isVisible
                if (!_params->takeDynObstOcclusionIntoAccountWhenLearning) {
                    //remove them
                    _gmap->removeDynamicObstacles();
                }

                //now filter tracks and use the 'corrected'/filtered positions:
                Pos curPosOut;
                IDPos hiderPosOut;
                autoPlayer->checkAndFilterPoses(playerInfo->currentPos, personVec, curPosOut, hiderPosOut, dontExec);

                DEBUG_CLIENT(
                    cout<<"Filtered poses: new seeker pos: "<<curPosOut.toString()<<" (";
                    if (playerInfo->currentPos.equals(curPosOut))
                        cout <<"equal";
                    else
                        cout <<"was "<<playerInfo->currentPos.toString();
                    cout<<") hider: "<<hiderPos.toString()<<endl;
                );

            } else {
                // !simulateReceivingAllTracksAsPersons

                //AG140526: if we don't want to use dyn. obstacles in simulations for learning, then remove them here...
                //          NOTE: also if's in the simulator code at calling GMap.isVisible
                if (!_params->takeDynObstOcclusionIntoAccountWhenLearning) {
                    //remove them
                    _gmap->removeDynamicObstacles();
                }
            }

            DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: get next robot poses"<<endl;);

            bool ok = false;

            //AG150521: run based on type of solver
            switch(_params->solverType) {
                case SeekerHSParams::SOLVER_TWO_HB_EXPL: {
                    ok = autoPlayer->calcNextRobotPoses2(actionDone);

                    DEBUG_CLIENT(cout << "GameConnectorClient::calcNextPosSeeker: calcNextRobotPoses2: "<< (ok?"ok":"NOT ok")
                                 <<", generated "<<playerInfo->multiGoalBPosesVec.size()
                                 <<" robot poses, now sending to server."<<endl;);

                    assert(playerInfo->multiHasGoalPoses);
                    assert(playerInfo->multiGoalBPosesVec.size()>0);
                    //assert(playerInfo->multiGoalBeliefVec.size()<=playerInfo->multiGoalBPosesVec.size());

                    break;
                }
                case SeekerHSParams::SOLVER_MULTI_HB_EXPL: {
                    ok = autoPlayer->calcNextHBList(actionDone);

                    DEBUG_CLIENT(cout << "GameConnectorClient::calcNextPosSeeker: calcNextHBList: "<< (ok?"ok":"not ok")
                                 <<", generated "<<playerInfo->multiHBPosVec.size()
                                 <<" highest belief points, now sending to server."<<endl;);

                    assert(playerInfo->multiHasHBPoses);
                    assert(playerInfo->multiHBPosVec.size()>0);
                    //assert(playerInfo->multiHBPosVec.size()==playerInfo->multiHBBeliefVec.size());

                    break;
                }
                default: {
                    DEBUG_CLIENT(cout << "GameConnectorClient::calcNextPosSeeker: get next robot pose"<<endl;);
                    //calculate the next pos (also stored in PlayerInfo)
                    Pos newPos = autoPlayer->getNextPos(actionDone); //(curPos,oppPos,isVisib,actions,actionDone,_params->useNextNPos);
                            //_autoPlayer->getNextPosWithFilter(curPos,personVec,actions,lastA,_params->useNextNPos,dontExec);
                    //_player->setCurPos(newPos);
                    DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: got next robot pos: "<<newPos.toString()<<endl;);
                    break;
                }
            } // switch solverType

            //AG150813: unset posread for this player (if no multi seeker pos)
            if (_params->multiSeekerNoCommunication)
                playerInfo->posRead = false;

        } // for AutoPlayer

        //AG150813: if simulating no comm. -> set as not read
        if (_params->multiSeekerNoCommunication) {
            for(PlayerInfo* playerInfo : _playerInfoVec) {
                if (playerInfo->isSeeker())
                    playerInfo->posRead = true;
            }
        }

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    } // else - SeekerHS set
#endif
}

void GameConnectorClient::calcNextPosHider() {
    assert(_initBeliefDone);

    //AG150522: code from readServerUpdate, split in function to reduce function size
    //assuming it is only 1  (for now)
    assert(_autoPlayerVec.size()==1);

#ifdef USE_HIDER_AS_AUTOPLAYER

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    if (_seekerHS!=NULL) { //TODO
        throw CException(_HERE_,"adapt to new multi agent structure");

        vector<double> nextPos;
        int winState = -1;
        vector<double> ov;
        if (isVisib) {
            ov = oppPos.toVector();
            ov.push_back(0);
        }
        _seekerHS->getNextPose(oppPos.toVector(), ov, nextPos, &winState);
        DEBUG_CLIENT(cout << "Win state: "<<winState<<", next pos: "<<nextPos[0]<<","<<nextPos[1]<<endl;);
        _player->setCurPos(nextPos[0], nextPos[1]);
    } else {
#endif
        //a = _autoPlayer->getNextAction(oppPos,curPos,isVisib);
        /*vector<int> actions;
        IDPos curIDPos(curPos, 0);
        int lastA = -1;
        if (_params->useDeducedAction) {
            lastA = _autoPlayer->deduceAction(_lastPos,curPos);
        }
        _player->setCurPos(_autoPlayer->getNextPos(oppPos,curIDPos,isVisib,actions,lastA,_params->useNextNPos));*/

        //calculate the next pos (stored also in PlayerInfo)
        Pos nextPos = _autoPlayerVec[0]->getNextPos();

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    }
#endif
#endif

}


void GameConnectorClient::sendInitPos() {
    //send his initial position
    //Pos p = _player->getCurPos();

    DEBUG_CLIENT(cout << "GameConnectorClient::sendInitPos: Sending init pos: "<</*p.toString()<<*/endl;);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out <<(BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_InitPos;

    if (_autoPlayerVec.empty()) {
        assert(_thisPlayer!=NULL);
        out << (quint8)1;
        //init pos
        assert(_thisPlayer->playerInfo.id>=0);
        out<<(quint8)_thisPlayer->playerInfo.id;
        _thisPlayer->playerInfo.currentPos.writePostoStream(out,_params->useContinuousPos);
    } else {
        //only autoplayers
        //num players
        out<<(quint8)_autoPlayerVec.size();
        //for each player send the init pos
        for (AutoPlayer* autoPlayer : _autoPlayerVec) {
            assert(autoPlayer->playerInfo.id>=0);
            out<<(quint8)autoPlayer->playerInfo.id;
            autoPlayer->playerInfo.currentPos.writePostoStream(out,_params->useContinuousPos);
        }
    }

    //init position
    //p.writePostoStream(out, _params->useContinuousPos);

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);
    _tcpSocket->flush();
}

void GameConnectorClient::writePlayerUpdate(QDataStream &out, PlayerInfo *playerInfo, const Pos* fakePos) {
    if (!playerInfo->nextPos.isSet())
        throw CException(_HERE_,"Next pos for player "+playerInfo->toString()+" is not set");

    assert(playerInfo->id>=0);
    out<<(quint8)playerInfo->id;
    out<<(quint16)playerInfo->lastAction;

    //send pos
    if (fakePos==NULL) {
        playerInfo->nextPos.writePostoStream(out,_params->useContinuousPos);
    } else {
        //send fake for testing
        fakePos->writePostoStream(out,_params->useContinuousPos);
    }



    out << playerInfo->seekerBeliefScore;
    out << playerInfo->seekerReward;
    //if (_params->seekerSendsMultiplePoses() /*gameHas2Seekers()*/ && playerInfo->isSeeker()) {
    //AG150717: always sent chosen goal
        assert(playerInfo->chosenGoalPos.isSet());
        playerInfo->chosenGoalPos.writePostoStream(out,_params->useContinuousPos);
    //}

    playerInfo->numberActions++;
}

bool GameConnectorClient::sendPos(const Pos *fakePos) {
    if(!_canDoAction) { //the action was not done
        cout<<"GameConnectorClient::sendPos: Action Aborted because action not yet allowed."<<endl;
        return false;
    }

    //send action
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out << (BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_Action;

    //send action

    //AG150525: send player info
    if (_autoPlayerVec.empty()) {
        assert(_thisPlayer!=NULL);

        //1 player for this client
        out << (quint8)1;

        writePlayerUpdate(out, &_thisPlayer->playerInfo, fakePos);

    } else {
        //using the auto players

        assert(fakePos==NULL);//can't use fake pos

        //AG150522: send all positions
        out << (quint8)_autoPlayerVec.size();
        //for each player send the new pos
        for (AutoPlayer* autoPlayer : _autoPlayerVec) {
            //AG150804: check belief score
            if (autoPlayer->tracksBelief()) {
                assert(_hiderPlayer->currentPos.isSet());
                autoPlayer->playerInfo.seekerBeliefScore = autoPlayer->getBeliefScore(_hiderPlayer->currentPos);
            }
            //send player details
            writePlayerUpdate(out, &autoPlayer->playerInfo, NULL);
        }
    }

/*    //AG140410: use autoplayer if that one is used, otherwise use player
    int lastAction = -1;
    if (_autoPlayer==NULL) {
        lastAction = _player->getLastAction();
    } else {
        lastAction = _autoPlayer->getLastAction();
    }

    out <<(qint16)lastAction;

    if (obsPos!=NULL) { //AG130722: use other observation, ie user clicks observation
        DEBUG_CLIENT(cout << "GameConnectorClient::sendAction: sending obs: ("<<obsPos->toString()<< ")"<<endl;);

        obsPos->writePostoStream(out, _params->useContinuousPos);

    } else {
        Pos p = _player->getCurPos();
        DEBUG_CLIENT(cout << "GameConnectorClient::sendAction: sending pos: "<<p.toString()<<endl;);

        p.writePostoStream(out, _params->useContinuousPos);
    }

    //AG140409: belief score, NOTE: this is the score of the PREVIOUS belief!!
    double beliefScore = -1;
    if (_autoPlayer!=NULL && _autoPlayer->tracksBelief()) {
        //AG140606: belief score without noise
        beliefScore = _autoPlayer->getBeliefScore(_player->getPlayer2Pos());
        //cout << "SENDING Beliefscore "<<beliefScore<<" for pos "<<_player->getOppPos().toString()<<endl;
    }
    out << beliefScore;
    //AG140612: reward
    double reward = 0;
    if (_autoPlayer!=NULL) {
        reward = _autoPlayer->getReward();
    }
    out << reward;

    //AG150216: if 2 seekers, and isSeeker -> send chosen pos
    if (chosenPos!=NULL ) {
        chosenPos->writePostoStream(out,_params->useContinuousPos);
    }*/

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);//send to server
    _tcpSocket->flush();
    _numMsgSent++;
    _canDoAction = false;
    //_player->incrNumActions();

    DEBUG_CLIENT(cout << "--------------------------------" << endl;);

    //belief to image
    if (!_autoPlayerVec.empty() && !_params->beliefImageFile.empty()
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
            && _seekerHS==NULL
#endif
            ) {
        //AG150526: write belief file using first autoplayer
        //_autoPlayer->storeMapBeliefAsImage(_params->beliefImageFile,curPos, &seeker2Pos, oppPos, &oppPos2, _params->beliefImageCellWidth);
        _autoPlayerVec[0]->storeMapBeliefAsImage(_params->beliefImageFile, /*curPos, &seeker2Pos, oppPos, &oppPos2,*/ _params->beliefImageCellWidth);
    }


    return true;
}


void GameConnectorClient::sendRobotGoalsAndBeliefs() { //const std::vector<Pos> &goalPosesVec, const std::vector<double> &goalPosesBeliefVec) {
    if(!_canDoAction) { //the action was not done
        cout<<"GameConnectorClient::sendRobotGoalsAndBeliefs: Action Aborted because action not yet allowed."<<endl;
        throw CException(_HERE_, "Action Aborted because action not yet allowed");
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out << (BLOCK_SIZE)0;
    out<<(MESSAGE_TYPE_SIZE)MT_SeekerGoals;

    //AG150522: send all positions
    out << (quint8)_autoPlayerVec.size();
    //for each player send the new pos
    for (AutoPlayer* autoPlayer : _autoPlayerVec) {
        PlayerInfo* player = &autoPlayer->playerInfo;

        assert(player->multiHasGoalPoses);
        assert(player->multiGoalBPosesVec.size()>0 && player->multiGoalBPosesVec.size()<=2);
        assert(player->multiGoalBPosesVec.size()==player->multiGoalIDVec.size());
        //assert(player->multiGoalBeliefVec.size()==0 || player->multiGoalBeliefVec.size()==player->multiGoalBPosesVec.size());

        //send id
        out << (quint8)player->id;

        //AG150710: check that we have at least 1, at max 2
        assert( player->multiGoalBPosesVec.size()==2 ||  player->multiGoalBPosesVec.size()==1);

        assert(player->multiGoalIDVec[0]==player->id);
        assert(player->multiGoalBPosesVec.size()==1 || player->multiGoalIDVec[1]!=player->id);

        //goal poses
        out << (quint8) player->multiGoalBPosesVec.size();
        for (size_t i=0; i<player->multiGoalBPosesVec.size(); i++) {
            //AG150710: check if the id is legal (the id is the index in the global playerInfoVec)
            assert(player->multiGoalIDVec[i]>=0 && player->multiGoalIDVec[i]<(int)_playerInfoVec.size());

            //id (AG150605)
            out << (quint8)player->multiGoalIDVec[i];

            //pos
            player->multiGoalBPosesVec[i].writePostoStream(out,_params->useContinuousPos);
            //now belief
            /*if (i<player->multiGoalBeliefVec.size()) {
                out << player->multiGoalBeliefVec[i];
            } else {
                out << -1.0;
            }*/
        }

        //beliefs
        /*out << (quint8) player->goalPosesBelVec.size();
        for (double d : player->goalPosesBelVec) {
            out << d;
        }*/
    }

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);//send to server
    _tcpSocket->flush();
}

void GameConnectorClient::sendHBs() { //const std::vector<Pos> &goalPosesVec, const std::vector<double> &goalPosesBeliefVec) {
    if(!_canDoAction) { //the action was not done
        cout<<"GameConnectorClient::sendHBs: Action Aborted because action not yet allowed."<<endl;
        throw CException(_HERE_, "Action Aborted because action not yet allowed");
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out << (BLOCK_SIZE)0;
    out<<(MESSAGE_TYPE_SIZE)MT_SeekerHB;

    //AG150522: send all positions for all players
    out << (quint8)_autoPlayerVec.size();
    //for each player send the new pos
    for (AutoPlayer* autoPlayer : _autoPlayerVec) {
        PlayerInfo* player = &autoPlayer->playerInfo;

        assert(player->multiHasHBPoses);
        assert(!player->multiHBPosVec.empty());
        //assert(player->multiHBPosVec.size()==player->multiHBBeliefVec.size());

        //per player send id and HBs
        assert(player->id>=0);

        out << (quint8) player->id;
        out << (quint8) player->multiHBPosVec.size();
        for (size_t i=0; i<player->multiHBPosVec.size(); i++) {
            //pos
            player->multiHBPosVec[i].writePostoStream(out,_params->useContinuousPos);
        }
    }

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);//send to server
    _tcpSocket->flush();
}

void GameConnectorClient::readSeeker2GoalsAndBeliefs(QDataStream &in) {
    assert(_params->seekerSendsMultiplePoses());

    quint8 numPlayers, pid, numGoals, pidG;
    double b;
    in >> numPlayers;

    //read the goals and beliefs for all the player seekers
    assert((size_t)numPlayers==_playerInfoVec.size()-1); //AG150609: TODO - this could later be allowed if we emulate 'missing' data
    for(quint8 i=0; i<numPlayers; i++) {
        //get id
        in >> pid;
        assert(pid<_playerInfoVec.size());
        //get player
        PlayerInfo* playerInfo = _playerInfoVec[pid];
        //assert(player->posRead);
        assert(playerInfo->initPosSet);
        assert(playerInfo->isSeeker());

        //now read the goals (for each robot there should be a goal)
        in >> numGoals;
        assert(numGoals==numPlayers);
        assert(numGoals==2); //AG150610: this later could be different

        //prepare vector goals
        playerInfo->multiGoalBPosesVec.resize(numGoals);
        //playerInfo->multiGoalBeliefVec.resize(numGoals);
        playerInfo->multiGoalIDVec.resize(numGoals);
        //read the goal poses and beliefs, with the id being the index
        for(quint8 j=0; j<numGoals; j++) {
            //AG150605: read id
            in >> pidG;
            assert(pidG<_playerInfoVec.size());
            playerInfo->multiGoalIDVec[j] = (int)pidG;
            //goal bpos
            playerInfo->multiGoalBPosesVec[j].readPosFromStream(in, _params->useContinuousPos);
            //belief
            /*in >> b;
            playerInfo->multiGoalBeliefVec[j] = b;*/
        }

        //AG150610: assume that there are 2 goals, and the first one is for the robot itself
        assert(playerInfo->multiGoalIDVec[0]==playerInfo->id);
        assert(numGoals==1 || playerInfo->multiGoalIDVec[1]!=playerInfo->id);

        playerInfo->multiHasGoalPoses = true;
    }

    //now select the position
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    if (_seekerHS==NULL) {
#endif

        if (_params->multiSeekerNoCommunication) {
            //AG150813: set all others on false
            for(PlayerInfo* otherPlayer : _playerInfoVec) {
                if (otherPlayer->isSeeker()) {
                    otherPlayer->multiHasGoalPoses=false;
                }
            }
        }

        //loop all players to run pos selection
        for (AutoPlayer* autoPlayer : _autoPlayerVec) {
            PlayerInfo* player = &autoPlayer->playerInfo;

            assert(player->isSeeker());

            if (_params->multiSeekerNoCommunication) //AG1500813: set to true for this player
                player->multiHasGoalPoses = true;

            assert(player->multiHasGoalPoses);
            autoPlayer->selectRobotPosMulti();
            assert(player->nextPos.isSet());

            if (_params->multiSeekerNoCommunication) //AG1500813: set to false again for this player
                player->multiHasGoalPoses = false;
        }

        if (_params->multiSeekerNoCommunication) {
            //AG150813: set all others on true
            for(PlayerInfo* otherPlayer : _playerInfoVec) {
                if (otherPlayer->isSeeker()) {
                    otherPlayer->multiHasGoalPoses=false;
                }
            }
        }

        //AG150214: send chosen pos to server (for logging)
        //sendMultiSeekerChosenPos(chosenPos);

        //send action
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    } else {

        PosXY newSeekerPosXY;
        if (_params->multiSeekerNoCommunication) {
            _seekerHS->select2SeekerPose(NULL, NULL, newSeekerPosXY);
        } else {
            assert(_seeker2Player!=NULL);

            PosXY otherGoalThisPosXY, otherGoalOtherPosXY;

            if (_seeker2Player->multiHasGoalPoses) {
                assert(!_seeker2Player->multiGoalBPosesVec.empty());
                otherGoalOtherPosXY.fromBPos(_seeker2Player->multiGoalBPosesVec[0], _params); //note should be const index form TwoHBSeeker class
                if (_seeker2Player->multiGoalBPosesVec.size()>1) {
                    otherGoalThisPosXY.fromBPos(_seeker2Player->multiGoalBPosesVec[1], _params);
                }
            }

            _seekerHS->select2SeekerPose(&otherGoalThisPosXY, &otherGoalOtherPosXY, newSeekerPosXY);
        }

        Pos nextPos = newSeekerPosXY.toPos(_params);
        assert(nextPos==_seeker1Player->nextPos);
    }

#endif

    //AG150522: now send all poses
    if (_autoPlayerVec.size()>0)
        sendPos();

//--------
#ifdef OLD_CODE //this was the previous code
    assert(_autoPlayer!=NULL);
    assert(_params->seekerSendsMultiplePoses());

    Pos otherSeekerPos1, otherSeekerPos2;
    double otherSeekerPos1B, otherSeekerPos2B;

    quint8 numGoals, numBels;
    //number of goals sent by other seeker
    in >> numGoals;
    assert(numGoals>0 && numGoals<=2);

    //goals
    switch(numGoals) {
        case 1:
            //only of other seeker
            otherSeekerPos2.readPosFromStream(in, _params->useContinuousPos);
            break;
        case 2:
            //goal for this seeker
            otherSeekerPos1.readPosFromStream(in, _params->useContinuousPos);
            //goal of other seeker
            otherSeekerPos2.readPosFromStream(in, _params->useContinuousPos);
            break;
        //default:  //shouldn't occur
    }

    //get beliefs
    in >> numBels;
    assert(numBels==0 || numBels==numGoals);
    switch(numGoals) {
        case 1:
            //only of other seeker
            in >> otherSeekerPos2B;
            break;
        case 2:
            //belief of goal for this seeker
            in >> otherSeekerPos1B;
            //belief of goal of other seeker
            in >> otherSeekerPos2B;
            break;
        //default:  //shouldn't occur
    }

    //now select robot pos
/*#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    if (_seekerHS!=NULL) {*/ //TODO SeekerHS

    //chosen pos for if using 2 seekers, and in sim.
    Pos* chosenPos = NULL;

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    if (_seekerHS==NULL) {
#endif

        chosenPos = new Pos();
        Pos newPos = _autoPlayer->selectRobotPos2(&otherSeekerPos1, &otherSeekerPos2, otherSeekerPos1B, otherSeekerPos2B, _params->useNextNPos, chosenPos);
        _player->setCurPos(newPos);

        //AG150214: send chosen pos to server (for logging)
        //sendMultiSeekerChosenPos(chosenPos);

        //send action
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    } else {
        vector<double> newSeekerPosVec;
        if (_params->multiSeekerNoCommunication) {
            _seekerHS->selectMultiSeekerPose(vector<double>(), vector<double>(), -1, -1, _params->useNextNPos,
                                             newSeekerPosVec);
        } else {
            _seekerHS->selectMultiSeekerPose(otherSeekerPos1.toVector(), otherSeekerPos2.toVector(), otherSeekerPos1B,
                                             otherSeekerPos1B, _params->useNextNPos,newSeekerPosVec);
        }

        Pos p;
        _seekerHS->setPosFromVector(p, newSeekerPosVec);
        _player->setCurPos(p);
    }
#endif

    sendAction(chosenPos, NULL);
    if (chosenPos!=NULL)
        delete chosenPos;
#endif
}

void GameConnectorClient::readSeekerHB(QDataStream &in) {
    assert(_params->seekerSendsMultiplePoses());

    quint8 numPlayers, pid, numHBs;
    //double b;
    in >> numPlayers;
    DEBUG_CLIENT(cout<<"Receiving hb poses from server:"<<endl;);
    //read the goals and beliefs for all the player seekers
    assert((size_t)numPlayers==_playerInfoVec.size()-1);
    for(quint8 i=0; i<numPlayers; i++) {
        //get id
        in >> pid;
        assert(pid<_playerInfoVec.size());
        //get player
        PlayerInfo* player = _playerInfoVec[pid];
        //assert(player->posRead);
        assert(player->initPosSet || _params->multiSeekerNoCommunication);

        //now read the goals (for each robot there should be a goal)
        in >> numHBs;

        DEBUG_CLIENT(cout<<" - "<<player->toString()<<" numhb:"<<(int)numHBs<<": ";);

        assert(numHBs>0);
        //assert(numHBs==numPlayers);

        //prepare vector HBs
        player->multiHBPosVec.resize(numHBs);
        //player->multiHBBeliefVec.resize(numHBs);
        //read the goal poses and beliefs, with the id being the index
        for(quint8 j=0; j<numHBs; j++) {
            //goal pos
            player->multiHBPosVec[j].readPosFromStream(in, _params->useContinuousPos);
            DEBUG_CLIENT(cout<<player->multiHBPosVec[j].toString() <<"; "<<flush;);
            assert(player->multiHBPosVec[j].isSet());
            assert(player->multiHBPosVec[j].b>=0);
        }
        DEBUG_CLIENT(cout<<endl;);

        player->multiHasHBPoses = true;


    }

    //now select the position
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    if (_seekerHS==NULL) {
#endif
        if (_params->multiSeekerNoCommunication) {
            //AG150813: set all others on false
            for(PlayerInfo* otherPlayer : _playerInfoVec) {
                if (otherPlayer->isSeeker()) {
                    otherPlayer->multiHasHBPoses=false;
                }
            }
        }

        //loop through autoplayers
        for (AutoPlayer* autoPlayer : _autoPlayerVec) {
            PlayerInfo* player = &autoPlayer->playerInfo;

            if (_params->multiSeekerNoCommunication) //AG1500813: set to true for this player
                player->multiHasHBPoses = true;

            assert(player->multiHasHBPoses);

            autoPlayer->selectRobotPosMulti();

            if (_params->multiSeekerNoCommunication) //AG1500813: set to false again for this player
                player->multiHasHBPoses = false;

            assert(player->nextPos.isSet());
        }

        if (_params->multiSeekerNoCommunication) {
            //AG150813: set all others on false
            for(PlayerInfo* otherPlayer : _playerInfoVec) {
                if (otherPlayer->isSeeker()) {
                    otherPlayer->multiHasHBPoses=true;
                }
            }
        }

        //AG150214: send chosen pos to server (for logging)
        //sendMultiSeekerChosenPos(chosenPos);

        //send action
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    } else {

        PosXY newSeekerPosXY;
        vector<PosXY> otherHBVec;
        if (!_params->multiSeekerNoCommunication) {
            assert(_seeker2Player!=NULL);
            //create vector of hb poses
            if (_seeker2Player->multiHasHBPoses) {
                otherHBVec.resize(_seeker2Player->multiHBPosVec.size());
                for(size_t i=0; i<_seeker2Player->multiHBPosVec.size(); i++) {
                    otherHBVec[i].fromBPos(_seeker2Player->multiHBPosVec[i], _params);
                }
            }
        }

        _seekerHS->selectMultiSeekerPoseFromHB(otherHBVec, newSeekerPosXY);

        Pos nextPos = newSeekerPosXY.toPos(_params);
        assert(nextPos==_seeker1Player->nextPos);
    }

#endif

    /*sendAction(chosenPos, NULL);
    if (chosenPos!=NULL)
        delete chosenPos;*/

    //AG150522: now send all poses
    if (_autoPlayerVec.size()>0)
        sendPos();
}

bool GameConnectorClient::sendMessage(MessageSendTo sendTo, QByteArray &messageByteArray) { //TODO: update for multi
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out <<(BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_Message;

    out<<(quint8)sendTo;

    out<<messageByteArray;

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);
    _tcpSocket->flush();

    //TODO respond through here, or retrieve later by readMessage? (latest seems to be the easiest)
    return true;
}

void GameConnectorClient::sendStopRequest() { //TODO: update for multi
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out <<(BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_StopServer;
    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);
    _tcpSocket->flush();
}

void GameConnectorClient::readMessage(QDataStream &in) {
    quint8 from,to;
    QByteArray msg;
    in >> to;
    in >> from;
    in >> msg;

    //DEBUG_CLIENT(
        cout << "GameConnectorClient::readMessage: message:"<<endl
             << "|From: "<<HSGlobalData::MESSAGESENDTO_NAMES[from].toStdString()<<endl
             << "|To:   "<<HSGlobalData::MESSAGESENDTO_NAMES[to].toStdString()<<endl
             << "+---------------"<<endl
             << QString(msg).toStdString()<<endl
             << "+---------------<"<<endl;
    //);

    //TODO: read and send to client
    //NOTE!!!  through a special interface (header) that is to be used by gameconnectorclient AND for the real robot seekerhs through ROS
}

void GameConnectorClient::disconnectFromServer() {
    _tcpSocket->disconnect();
    onServerDisconnect();
}

void GameConnectorClient::onServerDisconnect() {
    cout << "Server disconnected!"<<endl;
    emit serverDisconnected();
    if (_exitOnDisconnect) {
        cout << "EXITING"<<endl;
        exit(0);
    }
}

void GameConnectorClient::moveToThreadVariables(QThread *thread) {
    cout<<"GameConnectorClient::moveToThreadVariables"<<endl;
    if (_tcpSocket!=NULL) {
        cout << "  move tcpSocket"<<endl;
        _tcpSocket->moveToThread( thread );
    }
}

bool GameConnectorClient::startGame() {
    return connectToServer();
}

SeekerHSParams* GameConnectorClient::getParams() {
    return _params;
}

bool GameConnectorClient::isSeeker() {
    return _params->isSeeker;
}

int GameConnectorClient::getGameStatus() {
    return _gameStatus;
}

bool GameConnectorClient::canDoAction() {
    return _canDoAction;
}

bool GameConnectorClient::haveServerParamsBeenRead() {
    return _serverParamsReceived;
}

void GameConnectorClient::setExitOnDisconnect(bool exitOnDisconnect) {
    _exitOnDisconnect = exitOnDisconnect;
}

void GameConnectorClient::setParams(SeekerHSParams* params) {
    _params = params;
}

GMap* GameConnectorClient::getGMap() {
    return _gmap;
}

vector<PlayerInfo*> GameConnectorClient::getPlayerInfoVec() {
    return _playerInfoVec;
}

vector<IDPos> GameConnectorClient::getDynamicObstacleVector() {
    _dynObstPosVecMutex.enter();
    vector<IDPos> dynObstPosvec = _dynObstPosVec;
    _dynObstPosVecMutex.exit();
    return dynObstPosvec;
}

AutoPlayer* GameConnectorClient::getAutoPlayer(int id) {
    if (id<0 || _autoPlayerVec.empty()) {
        return NULL;
    } else {
        map<quint8,AutoPlayer*>::iterator it = _autoPlayerMap.find((quint8)id);
        if (it==_autoPlayerMap.end()) {
            return NULL;
        } else {
            return it->second;
        }
    }
}

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
void GameConnectorClient::getSeerkHSPosXYForAll(PosXY &thisSeekerPosXY, PosXY &thisSeekerHiderObsPosXY,
                                                std::vector<PosXY>& otherSeekerPosXYVec, std::vector<PosXY>& otherSeekerHiderObsPosXYVec) {
    //this seeker
    thisSeekerPosXY.fromPos(_seeker1Player->currentPos, _params);
    assert(_seeker1Player->currentPos.isSet());
    assert(thisSeekerPosXY.isSet());
    thisSeekerPosXY.id = _seeker1Player->id;
    assert(_seeker1Player->id>=0);
    assert(_seeker1Player->currentPos.isSet());
    assert(_seeker1Player->posRead);
    assert(_seeker1Player->useObsProb>=0 && _seeker1Player->useObsProb<=1.0);
    thisSeekerHiderObsPosXY.fromPos(_seeker1Player->hiderObsPosWNoise, _params, 0, _seeker1Player->useObsProb);
    assert(thisSeekerHiderObsPosXY.b == _seeker1Player->useObsProb);
    thisSeekerHiderObsPosXY.id = _hiderPlayer->id;
    //other players
    for(PlayerInfo* pInfo : _playerInfoVec) {
        if (pInfo->isSeeker() && *pInfo!=*_seeker1Player) {
            //seeker pos
            PosXY posxy;
            posxy.fromPos(pInfo->currentPos,_params);
            posxy.id = pInfo->id;
            assert(pInfo->id>=0);
            otherSeekerPosXYVec.push_back(posxy);
            //hider obs
            PosXY hiderObsPosXY;
            assert(pInfo->useObsProb>=0 && pInfo->useObsProb<=1.0);
            hiderObsPosXY.fromPos(pInfo->hiderObsPosWNoise,_params,0,pInfo->useObsProb);
            assert(pInfo->useObsProb==hiderObsPosXY.b);
            hiderObsPosXY.id = _hiderPlayer->id;
            otherSeekerHiderObsPosXYVec.push_back(hiderObsPosXY);
        }
    }
}
#endif
