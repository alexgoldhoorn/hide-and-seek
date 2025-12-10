#include "gameconnectorclient.h"

#include <cassert>
#include <iostream>

#include "hsglobaldata.h"

#include "Utils/generic.h"
#include "exceptions.h"
#include "mutex.h"

#ifndef SEEKERHS_H
#include "seekerhs.h"
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
    _player = NULL;
    _tcpSocket = NULL;

    init();

    SHS_IN_GC(_seekerHS = seekerHS;);
    _mapToSend = mapToSend;
    _autoPlayer = autoPlayer;
    _metaInfo = metaInfo;
    _comments = comments;
}


GameConnectorClient::~GameConnectorClient() {
    if (_tcpSocket!=NULL) delete _tcpSocket; //and close first (?)
    if (_player!=NULL) delete _player;
}

void GameConnectorClient::init() {
    if (_player!=NULL) { //should be set to NULL
        delete _player;
    }

    _player = new Player();
    if (_params!=NULL)
        _player->setUsername(_params->userName);
    _player->initPlayer();

    _myInitPosSet = _oppInitPosSet = false;
    _numMsgReceived = _numMsgSent = 0;
    _canDoAction = false;
    _tcpSocket = NULL;
    _serverParamsReceived = false;
    _exitOnDisconnect = false;
    _gmap = NULL;    
}

void GameConnectorClient::setParams(std::string ip, int port, string userName, int mapID, int oppType, bool isSeeker,
                                    AutoPlayer* autoPlayer, GMap* mapToSend) {

    init();

    //set params
    if (_params==NULL)
        _params = new SeekerHSParams();

    _params->serverIP = ip;
    _params->serverPort = port;
    _params->mapID = mapID;
    _params->opponentType = oppType;
    _params->isSeeker = isSeeker;
    _params->userName = userName;

    _mapToSend = mapToSend;
    _autoPlayer = autoPlayer;

    //init player
    _player->setUsername(userName);
}

bool GameConnectorClient::connectToServer() {
    assert(!_params->serverIP.empty() && _params->serverPort>0 && _player!=NULL);

    //init vars
    _myInitPosSet = _oppInitPosSet = false;
    _serverParamsReceived = false;
    _numMsgReceived = _numMsgSent = 0;
    _canDoAction = false;
    _gameStatus = HSGlobalData::GAME_STATE_NOT_STARTED;

    DEBUG_CLIENT(cout<<_player->getUsername()<<", welcome to the Hide & Seek game!"<<endl<<endl;);

    if (_tcpSocket!=NULL) {
        //first disconnect old socket
        if (_tcpSocket->isOpen()) _tcpSocket->close();
        delete _tcpSocket;
    }

    //create new socket
    _tcpSocket = new QTcpSocket(this);
    _tcpSocket->connectToHost(QString::fromStdString(_params->serverIP), _params->serverPort);
    if(_tcpSocket->waitForConnected(10000)) {  //TODO WHY DOENST WORK???????????? ONLY CHANGED FORM 3000 to 10000 ???"·$="·/($)="·/$)=("·/$)=/"()$&"·()$&
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

    if (_params->isSeeker)
        out <<(quint8) HSGlobalData::PLAYER_TYPE_SEEKER;
    else
        out <<(quint8) HSGlobalData::PLAYER_TYPE_HIDER;

    //name
    out << QString::fromStdString(_player->getUsername());

    //AG140531: extra params to send
    out << _params->simObsNoiseStd;
    //AG140605: send noise
    out << _params->simObsFalseNegProb;
    out << _params->simObsFalsePosProb;

    //AG140531: extra meta info
    out << _metaInfo;
    //comments
    out << _comments;

    //block size
    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    /*cout << "SEND TO SERVER: block.size="<<block.size()<<", uint.sz="<<sizeof(quint32)<<", set size="<<(block.size() - sizeof(quint32))
         <<", as quint16:"<<((quint32) (block.size() - sizeof(quint32)) )<<endl;*/
    _tcpSocket->write(block);
    _tcpSocket->flush();

    DEBUG_CLIENT(cout<<"GameConnectorClient::sendInfoToServer: player sent his name : "<<_player->getUsername()<<endl;);
}


void GameConnectorClient::readInitInfoFromServer(QDataStream& in) {
    DEBUG_CLIENT(cout << endl << "GameConnectorClient::readInitInfoFromServer: Reading data.."<<endl;);

    //receive info from server
    quint8 useCont;
    quint16 mapID;
    quint8 type;
    quint8 gameType;

    //AG140212: game type
    in >> gameType;
    _params->gameType = (char)gameType;

    //AG140130: continuous or not
    in >> useCont;
    _params->useContinuousPos = (useCont==(quint8)1);

    //own user type: seeker/hider
    in >> type;
    _player->setType((int)type);
    _params->isSeeker = (type == HSGlobalData::PLAYER_TYPE_SEEKER);

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
    _player->setMap(_gmap);

    //read map
    _gmap->readMapFromStream(in);

    //load gmap distance matrix //AG140528 TODO: we should either send this through the network or some other manner
    if (_params->mapDistanceMatrixFile.length()>0) {
        DEBUG_CLIENT(cout<<"GameConnectorClient::readInitInfoFromServer: Loading distance matrix: "<<flush;);
        _gmap->readDistanceMatrixToFile(_params->mapDistanceMatrixFile.c_str());
        DEBUG_CLIENT(cout<<"ok"<<endl;);
    }

    QString p2UserName;
    in>>p2UserName;
    _player->setPlayer2Username(p2UserName.toStdString());

    //AG150114: p3
    QString p3UserName;
    in>>p3UserName;
    _player->setPlayer3Username(p3UserName.toStdString());

    if(!_params->isSeeker) {//hider
        /*DEBUG_CLIENT_VERB(*/cout<<"You are the hider!"<<endl
                            <<"Choose an initial position by clicking on a cell."<<endl;//);
    } else { //seeker
        /*DEBUG_CLIENT_VERB(*/cout<<"You are the seeker!"<<endl
                            <<"Choose an initial position by clicking on a cell."<<endl;//);
    }

    //AG140531: sim obs. noise std.dev.
    in >> _params->simObsNoiseStd;
    //AG140605: send noise
    in >> _params->simObsFalseNegProb;
    in >> _params->simObsFalsePosProb;

    //AG140606: receive noisy pos
    in >> _receiveNoisyPos;

    DEBUG_CLIENT(cout << "GameConnectorClient::readInitInfoFromServer: Recieve noisy pos: "<<(_receiveNoisyPos?"yes":"no")<<endl;);

    _numMsgReceived++;

    /*DEBUG_CLIENT_VERB(*/cout<<"You are playing against "<<_player->getPlayer2Username()<<endl;//);

    if (!_tcpSocket->isOpen())
        cout<<"error in socket connection"<<endl;

    emit serverParamsRead();

    //AG1405__: it failed here for really big maps (bcn upc map full)

    //now send the new pos (if we have the player)
    if (_autoPlayer != NULL) {
        assert(_gmap!=NULL);

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
        if (_seekerHS!=NULL) {
            _seekerHS->setMap(_gmap);
        } else {
#endif
            _autoPlayer->setMap(_gmap);
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
        }
#endif

        //set new init pos
        DEBUG_CLIENT(cout<<"GameConnectorClient::readInitInfoFromServer: Calculating init pos: "<<flush;);
        _player->setCurPos(_autoPlayer->getInitPos());
        DEBUG_CLIENT(cout<<_player->getCurPos().toString()<<endl;);
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
        //cout << "serverReadyRead: blocksize="<<blockSize<<endl;

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
                _serverParamsReceived = true;
            } else {
                cout << "ERROR: expected game parameters, but received another message type ("<<(int)messageType<<")"<<endl;
            }

        } else {
            if( !_oppInitPosSet )  { //if its the seeker on the first msg then read the initial pos of the hider
                if (!_myInitPosSet) {
                    //QMessageBox::information(this,"Error","Received opponent position, but still not sent own position.")
                    cout << "ERROR: Received opponent position, but still not sent own position."<<endl;

                    return;
                }
            }

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

void GameConnectorClient::readServerUpdate(QDataStream &in) {       //TODO: HERE the observation, can be sent from server
    quint16 validation, gameStatus,numDynObst;

    in >> validation; //validation byte
    if(validation==0) { //validation int
        //that means the previous action was not done from the server
        cout << "Error: received response 'invalid' from server"<<endl;
    }

    in >> gameStatus; //status byte

    _gameStatus = (int)gameStatus;

    //read your opponent's current position (w/o noise)
    //when the current player is a seeker, then this is the hider, otherwise seeker 1
    Pos pos2;
    pos2.readPosFromStream(in, _params->useContinuousPos);

    //AG140606: read opponent's position with noise and possibly empty
    Pos posWNoise;
    if (_receiveNoisyPos)
        posWNoise.readPosFromStream(in, _params->useContinuousPos);

    _player->setPlayer2Pos(pos2);
    _player->setHiderObsWNoise(posWNoise);

    //AG150114: third player possible
    //          Third player should never be the hider (controlled in HSTCPServer)
    if (_params->gameHas2Seekers()) {
        Pos pos3;
        pos3.readPosFromStream(in, _params->useContinuousPos);

        Pos seeker2HiderObsWNoise;
        if (_receiveNoisyPos)
            seeker2HiderObsWNoise.readPosFromStream(in, _params->useContinuousPos);

        _player->setPlayer3Pos(pos3);
        _player->setHiderObs2WNoise(seeker2HiderObsWNoise);
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
    _gmap->setDynamicObstacles(_dynObstPosVec);
    _dynObstPosVecMutex.exit();

    _numMsgReceived++;
    _canDoAction = true;

    //emit
    emit serverUpdateReceived(pos2.row(),pos2.col(),_gameStatus);

    DEBUG_CLIENT( cout << "GameConnectorClient::readServerUpdate: RECEIVED: opp="<<_player->getPlayer2Pos().toString()
                  <<", and noise pos: "<<_player->getHiderObsWNoise().toString()<<", status="<<WINSTATE_COUT(_gameStatus)<<endl; );

    if(_gameStatus > HSGlobalData::GAME_STATE_RUNNING) {
        DEBUG_CLIENT_VERB(cout<<"GAME OVER!"<<endl;);
        //_tcpSocket->disconnect();
        disconnectFromServer();

    } else if (_autoPlayer!=NULL
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
               || _seekerHS!=NULL
#endif
               ) {

        //send new pos action if its auto player
        Pos curPos(_player->getCurPos());
        IDPos oppPos( (_receiveNoisyPos ? _player->getHiderObsWNoise() : _player->getPlayer2Pos()), 0);
        //AG150218: always do the check because the oppPos.isSet() method could be inconsistent with 'posible' visibility due to added noise
        bool isVisib = oppPos.isSet();
        if (isVisib) isVisib = _gmap->isVisible(curPos,oppPos,true);

        //AG150224: if not visible (due to auto walkers for example)
        if (!isVisib) oppPos.clear();

        //cout <<"isVisib="<<isVisib<<" oppos="<<oppPos.toString()<<" s1="<<curPos.toString()<<" s2="<<_player->getPlayer3Pos().toString()<<endl;//////////
        assert(isVisib==oppPos.isSet());

        /*if (_receiveNoisyPos) { //AG140606: check noisy pos, and if so, already can be 'not set'
            isVisib = oppPos.isSet();
        } else {
            isVisib = _gmap->isVisible(curPos,oppPos,true); //AG140526: here always use dyn.obst to decide if visible
        }*/

        //AG150202: opponent position seen by other player (i.e. hider's pos as seen by seeker 2)
        Pos oppPos2;
        Pos seeker2Pos = _player->getPlayer3Pos();

        if (_params->gameHas2Seekers() && _receiveNoisyPos) {
            //AG150204: assert that the pos is noisy (although maybe we want to disable this)
            //assert(_receiveNoisyPos);
            oppPos2 = _player->getHiderObs2WNoise();

            //assume that is not visible of not seen.. but could be noise
            /*if (_gmap->isVisible(_player->getPlayer3Pos(), _player->getPlayer2Pos(),true)) {
            }*/
        }

        if( !_oppInitPosSet ) { //if its the seeker on the first msg then read the initial pos of the hider
            if (_params->isSeeker) {
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                if (_seekerHS!=NULL) {
                    vector<double> ov;
                    if (isVisib) {
                        ov = oppPos.toVector();
                        ov.push_back(0);
                    }
                    //AG150202: check if two seekers
                    if (_params->gameHas2Seekers()) {
                        /*if (!seeker2Pos.isSet()) {
                            throw CException(_HERE_, "We need the seeker 2 pos, but is not set");
                        }*/
                        vector<double> seeker2v;
                        if (!_params->multiSeekerNoCommunication && seeker2Pos.isSet()) {
                            seeker2v = seeker2Pos.toVector();
                        }
                        //opp 2 vector
                        vector<double> ov2;
                        if (!_params->multiSeekerNoCommunication && oppPos2.isSet()) {
                            ov2 = oppPos2.toVector();
                            ov2.push_back(0);
                        }

                        //init belief with 2 seekers
                        _autoPlayer->initBelief2(_gmap,curPos,oppPos,isVisib,seeker2Pos,oppPos2,_params->multiSeekerOwnObsChooseProb);

                        _seekerHS->initMultiSeekerObs(curPos.toVector(), ov, seeker2v, ov2 );

                    } else {
                        //_autoPlayer->initBelief(_gmap,curPos,oppPos,isVisib);
                        _seekerHS->init(curPos.toVector(),ov);
                    }
                } else
#endif
                    //AG150202: check if two seekers and not checking without communcation
                    if (_params->gameHas2Seekers() && !_params->multiSeekerNoCommunication) {
                        if (!seeker2Pos.isSet()) {
                            throw CException(_HERE_, "We need the seeker 2 pos, but is not set");
                        }
                        //init belief with 2 seekers
                        _autoPlayer->initBelief2(_gmap,curPos,oppPos,isVisib,seeker2Pos,oppPos2,_params->multiSeekerOwnObsChooseProb);

                    } else {
                        _autoPlayer->initBelief(_gmap,curPos,oppPos,isVisib);
                    }

            } else { // is a hider:
                //throw CException(_HERE_, "WARNING: this is an automated hider, UPDATE the GameConnectorClient!");
#ifdef USE_HIDER_AS_AUTOPLAYER
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                if (_seekerHS!=NULL) {
                    _seekerHS->init(oppPos.toVector(),curPos.toVector());
                } else
#endif

                    _autoPlayer->initBelief(_gmap,oppPos,curPos,isVisib);
#endif
            }
        } // !_oppInitPosSet

        DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: Get action for AutoPlayer ";);
        if (_params->isSeeker) {
            DEBUG_CLIENT(cout << "seeker "<<endl;);

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
            if (_seekerHS!=NULL) {
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
                    if (_params->gameHas2Seekers()) {
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



            } else { //No SeekerHS set ->
#endif
                vector<int> actions;
                int lastA = -1;
                if (_params->useDeducedAction) {
                    lastA = _autoPlayer->deduceAction(_lastPos,curPos);
                }

                DEBUG_CLIENT(
                    cout<<"GameConnectorClient::readServerUpdate: Deduced action: ";
                    if(lastA<0)
                        cout <<"none ("<<lastA<<")"<<endl;
                    else
                        cout<<ACTION_COUT(lastA)<<endl;
                );
                bool dontExec = false;

                //AG140512: sim that more hiders are entered
                if (_params->simulateReceivingAllTracksAsPersons) {
                    //first create vector
                    _dynObstPosVecMutex.enter();
                    vector<IDPos> personVec = _dynObstPosVec;
                    _dynObstPosVecMutex.exit();

                    //add hider
                    //IDPos iposHider(oppPos,0);
                    if (oppPos.isSet() && isVisib /*_gmap->isVisible(curPos, oppPos, true)*/) {
                        personVec.push_back(oppPos);
                    }

                    if (_params->simulateNotVisible) {
                        //simulate that tracks of 'persons' that are not visible due to obstacles, are not sent to the AutoPlayer
                        //AG140526: not necessary to also simulate lack of visibility due to dyn obst.
                        for(size_t i=0; i<personVec.size(); i++) {
                            if (!_gmap->isVisible(curPos, personVec[i], false)) {
                                personVec.erase(personVec.begin()+i);
                                i--;
                            }
                        }
                    }


                    /*cout << "visible persons(auto/hider):"<<endl;
                    for (IDPos& p : personVec)
                        cout << " - "<<p.toString()<<endl;*/


                    //AG140526: if we don't want to use dyn. obstacles in simulations for learning, then remove them here...
                    //          NOTE: also if's in the simulator code at calling GMap.isVisible
                    if (!_params->takeDynObstOcclusionIntoAccountWhenLearning) {
                        //remove them
                        _gmap->removeDynamicObstacles();
                    }

                    //newPos = _autoPlayer->getNextPosWithFilter(curPos,personVec,actions,lastA,_params->useNextNPos,dontExec);

                    //AG150202: do here only the check and filter
                    Pos curPosOut;
                    IDPos oppPosOut;
                    _autoPlayer->checkAndFilterPoses(curPos, personVec, curPosOut, oppPosOut, dontExec);
                    assert(curPosOut.isSet());
                    //now set it as current
                    curPos = curPosOut;
                    oppPos = oppPosOut;
                    //TODO: receive other's filtered obs??

                } else {
                    // !simulateReceivingAllTracksAsPersons

                    //AG140526: if we don't want to use dyn. obstacles in simulations for learning, then remove them here...
                    //          NOTE: also if's in the simulator code at calling GMap.isVisible
                    if (!_params->takeDynObstOcclusionIntoAccountWhenLearning) {
                        //remove them
                        _gmap->removeDynamicObstacles();
                    }

                    //newPos = _autoPlayer->getNextPos(curPos,oppPos,isVisib,actions,lastA,_params->useNextNPos);
                }

                //AG150202: 2 seekers option
                if (_params->gameHas2Seekers()) {
                    DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: get next robot poses"<<endl;);                    

                    bool ok = false;

                    if (_params->multiSeekerNoCommunication) {
                        ok = _autoPlayer->getNextRobotPoses2(curPos,oppPos, isVisib, NULL, NULL,
                                                         actions, _player->goalPosesVec, lastA, _params->useNextNPos, &_player->goalPosesBelVec);
                    } else {
                        ok = _autoPlayer->getNextRobotPoses2(curPos,oppPos, isVisib, &seeker2Pos, &oppPos2,
                                                         actions, _player->goalPosesVec, lastA, _params->useNextNPos, &_player->goalPosesBelVec);
                    }

                    DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: "<< (ok?"ok":"not ok")
                                 <<", received "<<_player->goalPosesVec.size()
                                 <<" robot poses (and "<< _player->goalPosesBelVec.size() <<" belief points), now sending to server."<<endl;);

                } else {
                    DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: get next robot pose"<<endl;);
                    Pos newPos = _autoPlayer->getNextPos(curPos,oppPos,isVisib,actions,lastA,_params->useNextNPos);
                            //_autoPlayer->getNextPosWithFilter(curPos,personVec,actions,lastA,_params->useNextNPos,dontExec);
                    _player->setCurPos(newPos);
                    DEBUG_CLIENT(cout << "GameConnectorClient::readServerUpdate: got next robot pos: "<<newPos.toString()<<endl;);
                }

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
            }
#endif
        } else { // is not a seeker
            //throw CException(_HERE_, "WARNING: this is an automated hider, UPDATE the GameConnectorClient!");
#ifdef USE_HIDER_AS_AUTOPLAYER
            DEBUG_CLIENT(cout << "hider "<<endl;);
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
            if (_seekerHS!=NULL) {
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
                vector<int> actions;
                IDPos curIDPos(curPos, 0);
                int lastA = -1;
                if (_params->useDeducedAction) {
                    lastA = _autoPlayer->deduceAction(_lastPos,curPos);
                }
                _player->setCurPos(_autoPlayer->getNextPos(oppPos,curIDPos,isVisib,actions,lastA,_params->useNextNPos));
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
            }
#endif
#endif
        }
        //do action
/*        if (
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                _seekerHS==NULL &&
#endif
                !_player->move(a)) {
            //only if seekerHS not set, use autoplayer's action to move
            cout << "WARNING: could not execute action of auto player: "<< ACTION_COUT(a) <<endl;

            if (_params->useContinuousPos) {
                //AG140305: fix by changing start pos
                Pos curPos = _player->getCurPos();
                //set to middle of cell
                curPos.convertValuesToInt();
                curPos.add(0.5,0.5);
                //now do action
                _player->setCurPos(curPos);
                if (!_player->move(a)) {
                    cout << "ERROR: could not move player, even after fix"<<endl;
                } else {
                    cout <<"fixed movement by starting from center"<<endl;
                }
            }

            cout << "ERROR: could not execute action of auto player: "<< ACTION_COUT(a) <<endl;
        }
        DEBUG_CLIENT(if (a>=0) cout<<" -> moving "<<ACTION_COUT(a)<<endl;);

        DEBUG_CLIENT(cout<<"Calculated pos: "<<_player->getCurPos().toString()<<endl;);
*/


        //belief to image
        if (!_params->beliefImageFile.empty()
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                && _seekerHS==NULL
#endif
                ) {
            _autoPlayer->storeMapBeliefAsImage(_params->beliefImageFile,curPos, &seeker2Pos, oppPos, &oppPos2, _params->beliefImageCellWidth);
        }

        //AG140403: set pos
        _lastPos = curPos;

        //AG150202: only send action when not using multiple seekers, otherwise wait for response
        if (_params->gameHas2Seekers() && _params->isSeeker) {
            sendRobotGoalsAndBeliefs();
        } else {
            sendAction(NULL,NULL);
        }

    } // AutoPlayer != NULL


    if (!_oppInitPosSet) {
        _oppInitPosSet = true;
    }
}


void GameConnectorClient::sendInitPos() {
    //send his initial position
    Pos p = _player->getCurPos();
    DEBUG_CLIENT(cout << "GameConnectorClient::sendInitPos: Sending init pos: "<<p.toString()<<endl;);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out <<(BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_InitPos;

    //init position
    p.writePostoStream(out, _params->useContinuousPos);

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);
    _tcpSocket->flush();
    _myInitPosSet = true;
}


bool GameConnectorClient::sendAction(const Pos* chosenPos, const Pos* obsPos) {
    if(!_canDoAction) { //the action was not done
        cout<<"GameConnectorClient::sendAction: Action Aborted because action not yet allowed."<<endl;
        return false;
    }

    //send action
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out << (BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_Action;


    //AG140410: use autoplayer if that one is used, otherwise use player
    int lastAction = -1;
    if (_autoPlayer==NULL) {
        lastAction = _player->getLastAction();
    } else {
        lastAction = _autoPlayer->getLastAction();
    }
    //cout<<">>sending last aciton: "<<lastAction<<" as quint8="<<(qint16)lastAction<<endl;
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
    if (chosenPos!=NULL /*_params->isSeeker && _params->gameHas2Seekers()*/) {
        chosenPos->writePostoStream(out,_params->useContinuousPos);
    }

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);//send to server
    _tcpSocket->flush();
    _numMsgSent++;
    _canDoAction = false;
    _player->incrNumActions();

    DEBUG_CLIENT(cout << "--------------------------------" << endl;);

    return true;
}


void GameConnectorClient::sendRobotGoalsAndBeliefs() { //const std::vector<Pos> &goalPosesVec, const std::vector<double> &goalPosesBeliefVec) {
    if(!_canDoAction) { //the action was not done
        cout<<"GameConnectorClient::sendRobotGoalsAndBeliefs: Action Aborted because action not yet allowed."<<endl;
    }
    assert(_player->goalPosesVec.size()>0 && _player->goalPosesVec.size()<=2);
    assert(_player->goalPosesBelVec.size()==0 || _player->goalPosesBelVec.size()==_player->goalPosesVec.size());

    //send action
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out << (BLOCK_SIZE)0;
    out<<(MESSAGE_TYPE_SIZE)MT_SeekerGoals;

    //goal poses
    out << (quint8) _player->goalPosesVec.size();
    for (const Pos& p : _player->goalPosesVec) {
        p.writePostoStream(out,_params->useContinuousPos);
    }

    //beliefs
    out << (quint8) _player->goalPosesBelVec.size();
    for (double d : _player->goalPosesBelVec) {
        out << d;
    }

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);//send to server
    _tcpSocket->flush();
}

void GameConnectorClient::readSeeker2GoalsAndBeliefs(QDataStream &in) {
    assert(_autoPlayer!=NULL);
    assert(_params->gameHas2Seekers());

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
}

bool GameConnectorClient::sendMessage(MessageSendTo sendTo, QByteArray &messageByteArray) {
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

void GameConnectorClient::sendStopRequest() {
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

/*void GameConnectorClient::sendMultiSeekerChosenPos(const Pos &chosenPos) {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    out <<(BLOCK_SIZE)0;
    //AG150120: message type
    out<<(MESSAGE_TYPE_SIZE)MT_ChosenSeekerGoal;

    chosenPos.writePostoStream(out, _params->useContinuousPos);

    out.device()->seek(0);
    out << (BLOCK_SIZE)(block.size() - sizeof(BLOCK_SIZE));
    _tcpSocket->write(block);
    _tcpSocket->flush();
}*/


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

bool GameConnectorClient::isMyInitPosSet() {
    return _myInitPosSet;
}

bool GameConnectorClient::isOppPosSet() {
    return _oppInitPosSet;
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

Player* GameConnectorClient::getPlayer() {
    return _player;
}

vector<IDPos> GameConnectorClient::getDynamicObstacleVector() {
    _dynObstPosVecMutex.enter();
    vector<IDPos> dynObstPosvec = _dynObstPosVec;
    _dynObstPosVecMutex.exit();
    return dynObstPosvec;
}
