
// WHY DOES IT NOT WORK??????
// CHECK: COMPARE WITH OLD server.ag ??? -> check differences.....


#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>

#include <QProcess>
#include <QDir>

#include "hstcpserver.h"

#include "../hsglobaldata.h"

#ifndef AUTOHIDER_OLD
#include "../AutoHider/randomhider.h"
#endif

HSTCPServer::HSTCPServer(HSServerConfig* config, HSGameLog* gameLog, QObject *parent)
    : QTcpServer(parent)
{
    cout << "Starting HSTCPserver"<<endl;
    //cout<<"A world is constructed"<<endl;
    _gmap = new GMap();
    status=-1;
    _hider =new PlayerInfo;
    _seeker =new PlayerInfo;
    //initplayers();
    _hider->set=0;
    _seeker->set=0;
    _numcon=0;
    _hidersock=new QTcpSocket(this);
    _seekersock=new QTcpSocket(this);
    _config = config;
    _gameLog = gameLog;
    _winDist = 0; //ag130404: default value
#ifndef AUTOHIDER_OLD
    _autoHider = NULL;
#endif

    //AG check: is this not a problem??? having the same method being slot and signal??
    connect(this, SIGNAL(newConnection()), this, SLOT(newConnection()));
}


//QString HSTCPServer::PATH = QString(QDir::currentPath ());


QString getFilePath(QString path, QString file) {
    if (path.right(1).compare(QDir::separator())==0) {
        return path + file;
    } else {
        return path + QDir::separator() + file;
    }
}

void HSTCPServer::newConnection() {

    cout<<"numcon = "<<_numcon<<endl;
    if(_numcon%2==0 && _numcon!=0) {
        cout<<"sto prwto if numcon: "<<_numcon<<endl;
        return;
    }
    if(status==0)  {
        cout<<"The game is on with different players. Try again later."<<endl;
        return;
    }
    else if(status>0)  { //the previous game is over...
        cout<<"Previous game is over. You ´re next"<<endl;
        newgame();


    }
    QTcpSocket* connection = this->nextPendingConnection();
    cout<<"new connection request! "<<connection->socketDescriptor()<<" number of connections: "<<_numcon<<endl;

    if(connection->isOpen())
    {

        cout<<"Number of connections = "<<_numcon<<endl;
        while (!connection->waitForReadyRead(6000))
        {

        }
        //read
        quint16 blockSize=0;
        QDataStream in(connection);
        in.setVersion(QDataStream::Qt_4_0);
        if (blockSize == 0) {
            if (connection->bytesAvailable() < (int)sizeof(quint16)) {
                cout<<"less bytes in the array than expected1."<<endl;
                connection->flush();
                return;

            }
            in >> blockSize;
        }
        if (connection->bytesAvailable() < blockSize) {
            cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
            return;
        }


        quint16 m, o, t;
        QString username;
        QString actionFile ="";//ag120903

        //read map id
        in >> m;

        //ag130723: receive map from client
        if (m==HSGlobalData::MAP_PASSED_BY_NETWORK) {
            cout << "PASSED MAP"<<endl;
            //rows,cols
            quint16 mrows,mcols,orow,ocol;
            quint32 mobst;
            in >> mrows;
            in >> mcols; cout << "row,col: "<<mrows<<","<<mcols<<endl;
            //reserve map space
            _gmap->createMap(mrows,mcols,true);
            //base
            in >> orow;
            in >> ocol; cout <<"base: "<<orow<<","<<ocol<<endl;
            _gmap->setbase(orow,ocol);

            //obstacles
            in >> mobst; cout <<"obst:"<<mobst<<endl;
            for(unsigned int i=0; i<mobst; i++) {
                in >> orow;
                in >> ocol;
                //add to map
                _gmap->addObstacle(orow,ocol);
            }

            cout << "Passed map:"<<endl;
            _gmap->printMap();
        }

        //read opponent type
        in >> o;        

        if ((int)o==HSGlobalData::SERVER_WIN_DIST_CODE) {
            //ag130404: first read win distance
            quint16 wd;
            in >> wd;
            _winDist = (int)wd;

            cout << "Win dist: "<<_winDist<<endl;

            //read opponent
            in >> o;
        }

        //AG120903: opponent type, if is list, expecting name of action list (file)
        if ((int)o==HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST) {
            in >> actionFile;
            cout << "Action file: "<<actionFile.toStdString()<<endl;
        }
        in >> t; //1:seeker, 0:hider
        in >> username;
        connection->flush();
        cout<<endl;
        cout<<"the infomsg sent from the server was received as: "<<(int)m<<"/"<<(int)o<<"/"<<(int)t<<"!"<<endl;
        char c;
        if (_numcon==2) {
            cout<<"max connections established"<<endl;
            _numcon++; //AG: should this be incremented??
            return;
        }

        if(_numcon==0) { //first connection
            if((int)t==1) { //if the first PlayerInfo chose to be the seeker
                c='s';
                _seekersock=connection;
                cout<<"P1 - Connection established with seeker"<<endl;
                _seeker->set=TRUE;
                _hider->set=FALSE;
            } else {//if the first PlayerInfo chose to be the hider
                c='h';
                cout<<"P1 - Connection established with hider"<<endl;
                _hidersock=connection;
                _hider->set=TRUE;
                _seeker->set=FALSE;
            }
            //set _filename
            //>AG120515: use prev set path
            /*QStringList path = _config->getMapPath();
            path = PATH.split("/");
            cout << PATH.toStdString()<<endl;
            return;
            if(path.value(path.size()-1)=="MacOS") {
                path.removeLast();
                path.removeLast();
                path.removeLast();
            }
            path.removeLast();
            path.removeLast();

            _filename=path.join("/");*/ //<AG
        } else if(_numcon==1) { //second connection
            if(_seeker->set) { //the seeker is already defined
                c='h';
                cout<<"P2 - Connection established with hider"<<endl;
                _hidersock=connection;
                _hider->set=1;
            } else { //the seeker is not defined
                c='s';
                _seekersock=connection;
                cout<<"P2 - Connection established with seeker"<<endl;
                _seeker->set=1;
            }
            this->close();

            //ag120523:
            startNewServerInstance();

        }
        _numcon++;

        mclient[connection] = c;

        if(mclient[connection]=='h') {
            _hider->username = username;
        }
        else {
            _seeker->username = username;
        }

        //ag note: isn't this done 2 times (for each player)??
        connect(_hidersock, SIGNAL(readyRead()), this, SLOT(hreadyRead()));
        connect(_seekersock, SIGNAL(readyRead()), this, SLOT(sreadyRead()));
        connect(connection, SIGNAL(disconnected()), this, SLOT(ondisconnect()));



#ifdef AUTOHIDER_OLD
        if(_numcon==2) { //ag: we have two connected players

            status=0;//game on
            _seeker->set=0;
            _hider->set=0;
            cout<< "the players are: "<<_hider->username.toStdString()<<" vs. "<<_seeker->username.toStdString()<<endl;
            sendmaptoPlayers();
            //initializeLogs(); //open the file and insert the first record
            //cout<<"first messages sent to both, from: "<<mclient[connection]<<endl;
        } else if (_numcon==1) {
            //cout<<"seekerset: "<<_seeker->set<<", hiderset: "<<_hider->set<<endl; //ag:
            cout << "newly connected player is: "<< (_seeker->set?"seeker":(_hider->set?"hider":"unknown"))<<endl;

            _map=(int)m; //the map choise is made by the first player who is also by default the seeker
            _opp=(int)o; //opponent choise
            // cout<<"the chosen map and opponent from the seeker are: "<<_map<<", "<<_opp<<endl;
            if(_opp != HSGlobalData::OPPONENT_TYPE_HUMAN) { //AG120904: also human type

                /*if (_opp==1) {
                    callrandhider(0);
                } else if (_opp==2) {
                    callrandhider(1);
                } else if (_opp==3) {
                    //call SeekroBot
                    callSeekroBot();
                }*/

                //AG120903
                if (_opp==HSGlobalData::OPPONENT_TYPE_SEEKER) {
                    callSeekroBot();
                } else {
                    callrandhider(_opp,actionFile);
                }


            }

            if (m!=HSGlobalData::MAP_PASSED_BY_NETWORK) { //AG130723
                //read map if not set before
                QString mapFile = getFilePath(_config->getMapPath(), HSGlobalData::MAPS[_map]);
                cout << "Loading map file: " << mapFile.toStdString()<<endl;

                _gmap->readMapFile(mapFile.toStdString().c_str());
            }

            //ag120511: set max actions dependend on map size:
            //max1: (#rows*#cols)/2
            //max2: (#rows+#cols)*1.5   <- implemented here
            _maxActions = (int)((_gmap->colCount()+_gmap->rowCount())*MAX_ACT_MULT_FACTOR);
            cout << "map size: "<<_gmap->rowCount()<<"x"<<_gmap->colCount()<<endl;
            _maxActions = min(_maxActions,MAXACTIONS);
            cout << "maximum actions: "<<_maxActions<<endl;

            //init players
            initplayers();
        }

#else
        if (_numcon==1) {
            //cout<<"seekerset: "<<_seeker->set<<", hiderset: "<<_hider->set<<endl; //ag:
            cout << "newly connected player is: "<< (_seeker->set?"seeker":(_hider->set?"hider":"unknown"))<<endl;

            _map=(int)m; //the map choise is made by the first player who is also by default the seeker
            _opp=(int)o; //opponent choise
            // cout<<"the chosen map and opponent from the seeker are: "<<_map<<", "<<_opp<<endl;


            QString mapFile = getFilePath(_config->getMapPath(), HSGlobalData::MAPS[_map]);
            cout << "Loading map file: " << mapFile.toStdString()<<endl;

            _gmap->readMapFile(mapFile.toStdString().c_str());

            //ag120511: set max actions dependend on map size:
            //max1: (#rows*#cols)/2
            //max2: (#rows+#cols)*1.5   <- implemented here
            _maxActions = (int)((_gmap->colCount()+_gmap->rowCount())*1.5);
            cout << "map size: "<<_gmap->rowCount()<<"x"<<_gmap->colCount()<<endl;
            _maxActions = min(_maxActions,MAXACTIONS);
            cout << "maximum actions: "<<_maxActions<<endl;

            //AG120608: moved opponent check after loading map, since it is using the auto hider
            if(_opp) {

                if (_opp==1) {
                    #ifndef AUTOHIDER_OLD
                        _autoHider = new RandomHider(_gmap);
                    #else
                        callrandhider(0);
                    #endif

                } else if (_opp==2) {
                    callrandhider(1);
                } else if (_opp==3) {
                    //call SeekroBot
                    callSeekroBot();
                }

                //AG120608: auto hider set -> skip new connection
                _numcon = 2;

            }

            //init players
            initplayers();
        }

        //AG120608: moved down for auto hider
        if(_numcon==2) { //ag: we have two connected players

            status=0;//game on
            _seeker->set=0;
            _hider->set=0;
            cout<< "the players are: "<<_hider->username.toStdString()<<" vs "<<_seeker->username.toStdString()<<endl;
            sendmaptoPlayers();
            //initializeLogs(); //open the file and insert the first record
            //cout<<"first messages sent to both, from: "<<mclient[connection]<<endl;
        }
#endif



    }
    else
        cout<<"no connection"<<endl;


    cout<<mclient[connection]<<": ending newconnection function..."<<endl;

}

//msg format: action, current position(x,y)
void HSTCPServer::hreadyRead() {

    cout<<"In hreadyRead "<<_hider->set<<"/"<<_hider->flag<<endl;

    if(!_hider->set) { //if the hider is not set yet
        //read his initial position and send it to seeker

        quint16 blockSize=0;
        QDataStream in(_hidersock);
        in.setVersion(QDataStream::Qt_4_0);
        if (blockSize == 0) {

            if (_hidersock->bytesAvailable() < (int)sizeof(quint16)) {
                cout<<"less bytes in the array than expected...xco."<<endl;
                _hidersock->flush();
                return;
            }
            in >> blockSize;
        }

        if (_hidersock->bytesAvailable() < blockSize) {
            cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
            _hidersock->flush();
            return;
        }

        quint16 x,y;
        in >> x;
        in >> y;
        Pos h;
        h.row=(int)x;
        h.col=(int)y;
        //AG121112: set prev pos
        _hider->previous = _hider->current;
        //set current
        _hider->current=h;
        _gmap->setHider(h);
        _hidersock->flush();
        cout<<"hider sent his initial pos: "<<h.toString()<<endl;
        //send to seeker

        sendinittoseeker();
        cout << "after sendtoseeker"<<endl;
        /*send ok to the hider
        QString ok ="ok";
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << (quint16)0;
        out << ok;
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        _hidersock->write(block);*/

        //set both of the players
        _hider->set=1;
        _seeker->set=1;
        status=0;
        _hider->win=0;
        _seeker->win=0;

        cout << "init logs:"<<flush;
        //write the first line to the file
        initializeLogs();
        cout <<endl;
        //repaintmap(); //ag111201
        //emit _hidersock->readyRead();
        //return;
    }
    //cout<<"in the hreadyread()"<<endl;
    cout << "status: "<<status<<endl;
    if(status!=0)
    {   //the game is over so reset the game (?if asked)
        //   newgame();

        QByteArray buf = _hidersock->readAll();
        QString str=buf;
        cout<<"server reads from hiderand ignores: "<<str.toStdString()<<endl;
        cout<<"game is not on yet !"<<endl;
        if (_hidersock->write("Server: game is not on yet !\r\n")==-1)
            cout<<"error on write hidersocket"<<endl;
        _hidersock->waitForBytesWritten(1000);
        return;//AGc: WHY IS THE FLUSH AFTER????
        _hidersock->flush();

    }
    if(_hider->flag)
    {
        cout << "hider flag .."<<endl;
        quint16 blockSize=0;
        QDataStream in(_hidersock);
        in.setVersion(QDataStream::Qt_4_0);
        if (blockSize == 0) {

            if (_hidersock->bytesAvailable() < (int)sizeof(quint16)) {
                cout<<"less bytes in the array than expected1."<<endl;
                _hidersock->flush();
                return;
            }
            in >> blockSize;
        }

        if (_hidersock->bytesAvailable() < blockSize) {
            cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
            _hidersock->flush();
            return;
        }
        //cout << "in: "<<flush;
        quint16 p,q,r;
        in >> p;
        in >> q;
        in >> r;
        //cout << p <<"/"<<q<<"/"<<r<<endl;

        cout<<"Server reads from H(ignores: flag=1): action-"<<(int)p<<", ("<<(int)q<<","<<(int)r<<") "<<endl;

        QString text;
        text="Server: wait for seeker's move";
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << (quint16)0;
        out <<(quint16)0;//validation byte
        out <<(quint16)_hider->current.row;
        out <<(quint16)_hider->current.col;
        out << text;
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        _hidersock->write(block);
        _hidersock->flush();

        //cout << "done hider part"<<endl;

        return;
    }

    //cout << "receiving .."<<endl;

    quint16 blockSize=0;
    QDataStream in(_hidersock);
    in.setVersion(QDataStream::Qt_4_0);
    if (blockSize == 0) {

        if (_hidersock->bytesAvailable() < (int)sizeof(quint16)) {
            cout<<"less bytes in the array than expected1."<<endl;
            return;
            _hidersock->flush();
        }
        in >> blockSize;
    }

    //cout <<"rec done"<<endl;

    if (_hidersock->bytesAvailable() < blockSize) {
        cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
        return;
        _hidersock->flush();
    }

    //cout <<"flushed, in:"<<flush;

    quint16 a, x,y;
    in >> a;
    in >> x;
    in >> y;
    cout<<"Server reads from H: "<<_hider->numa +1<<" action-"<<(int)a<<", ("<<(int)x<<","<<(int)y<<") "<<endl;

    _hider->action[_hider->numa] = (int)a;
    _hider->numa++;
    _hider->flag=1;
    //AG121112 set previous
    _hider->previous.row = _hider->current.row;
    _hider->previous.col = _hider->current.col;

    //set hider current
    _hider->current.row=(int)x;
    _hider->current.col=(int)y;
    _hider->timestamp= QDateTime::currentDateTime();
    _hidersock->flush();


    if ((_seeker->flag)&&(_hider->flag) && (_hider->numa==_seeker->numa)) {
        //since both players finished with their actions... check the status of the world(if someone wins or not!)

        status = checkstatus();
        if(status) {
            //AG120531: stop game
            _gameLog->stopGame();

            cout<<endl<<endl;
            cout<<"GAME OVER"<<endl<<endl;

        }
        //else s=0 game on


        //QString text="Server: sending positions!";
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << (quint16)0; //block size
        out <<(quint16)1; //validation byte
        out <<(quint16)status; //game status
        out << (quint16)_seeker->current.row;
        out << (quint16)_seeker->current.col;

        //cout<<text.toStdString()<<endl;
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        _hidersock->write(block);
        _hidersock->flush();

        block.clear();


        //out << (quint16)0;
        out <<(quint16)1;
        out <<(quint16)status; //game status
        out << (quint16)_hider->current.row;
        out << (quint16)_hider->current.col;
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        _seekersock->write(block);
        _seekersock->flush();

        //cout<<"position messages sent to both, from hider "<<endl;

        writetofile(); //log the actions in the files.
        _hider->flag=0;
        _seeker->flag=0;
        _gmap->setHider(_hider->current);
        _gmap->setSeeker(_seeker->current);
        //_w->repaintwidget(); //ag111201


        if(status)//disconnect both users and start over if wanted
            newgame();




    }
    else
    {
        //cout<<"next positions are not sent yet(h)!"<<endl;
    }




}


void HSTCPServer::sreadyRead() {

    //cout<<"in the sreadyread()"<<endl;
    _seekersock->flush();

    if(!_seeker->set) { //if seeker is not set yet print an error msg
        cout<<"seeker is not set yet"<<endl;
        return;
    }
    if(status!=0)
    {
        //newgame();

        return;

    }
    if(_seeker->flag) //the flag needs to be 0 in order to read next move.
    {
        quint16 blockSize=0;
        QDataStream in(_seekersock);
        in.setVersion(QDataStream::Qt_4_0);
        if (blockSize == 0) {

            if (_seekersock->bytesAvailable() < (int)sizeof(quint16)) {
                cout<<"less bytes in the array than expected1."<<endl;
                return;
                _seekersock->flush();
            }
            in >> blockSize;
        }
        if (_seekersock->bytesAvailable() < blockSize) {
            cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
            _seekersock->flush();
            return;
        }
        quint16 a, x, y;
        in >> a;
        in >> x;
        in >> y;
        _seekersock->flush();

        cout<<"Server reads from S(ignores: flag=1): action-"<<(int)a<<", ("<<(int)x<<","<<(int)y<<") "<<endl;


        QString text;
        text="Server: wait for hider's move";
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << (quint16)0;
        out <<(quint16)0;//validation
        out <<(quint16)_seeker->current.row;
        out <<(quint16)_seeker->current.col;
        out << text;
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        _seekersock->write(block);
        _seekersock->flush();
        return;
    }

    quint16 blockSize=0;
    QDataStream in(_seekersock);
    in.setVersion(QDataStream::Qt_4_0);
    if (blockSize == 0) {

        if (_seekersock->bytesAvailable() < (int)sizeof(quint16)) {
            cout<<"less bytes in the array than expected1."<<endl;
            _seekersock->flush();
            return;
        }
        in >> blockSize;
    }
    if (_seekersock->bytesAvailable() < blockSize) {
        cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
        _seekersock->flush();
        return;
    }
    quint16 a, x, y;
    in >> a;
    in >> x;
    in >> y;
    cout<<"Server reads from S: "<<_seeker->numa +1<<" action-"<<(int)a<<", ("<<(int)x<<","<<(int)y<<") "<<endl;

    _seeker->action[_seeker->numa] = (int)a;
    _seeker->numa++;
    _seeker->flag=1;
    //AG121112: set previous
    _seeker->previous.row = _seeker->current.row;
    _seeker->previous.col = _seeker->current.col;
    //current
    _seeker->current.row=(int)x;
    _seeker->current.col=(int)y;
    _seeker->timestamp = QDateTime::currentDateTime();
    _seekersock->flush();

    cout<<_hider->flag<<_seeker->flag<<_hider->numa<<_seeker->numa<<endl;

    #ifndef AUTOHIDER_OLD
    //AG120608: use auto hider to respond
    if (_autoHider!=NULL) {
        int nhrow,nhcol;
        //AG TODO CHECK if use of x/y row/col is ok!
        a = _autoHider->getNextAction(_seeker->current.y,_seeker->current.x,_hider->current.y,_hider->current.x,&nhrow,&nhcol);

        _hider->action[_hider->numa] = (int)a;
        _hider->numa++;
        _hider->flag=1;
        //AG121112: prev
        _hider->previous.x = _hider->current.x;
        _hider->previous.y = _hider->current.y;
        //current
        _hider->current.x=(int)nhcol;
        _hider->current.y=(int)nhrow;
        _hider->timestamp = QDateTime::currentDateTime();
    }
    #endif

    if ((_hider->flag) && (_seeker->flag) && (_hider->numa==_seeker->numa)) {

        status = checkstatus();
        if(status) {
            //AG120531: stop game
            _gameLog->stopGame();

            cout<<endl<<endl;
            cout<<"GAME OVER"<<endl<<endl;
        }        

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);

        //0out <<(quint16)1; //valid
        out.setVersion(QDataStream::Qt_4_0);
        out << (quint16)0;
        out <<(quint16)1; //valid
        out <<(quint16)status; //game status
        out << (quint16)_hider->current.row;
        out << (quint16)_hider->current.col;
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        _seekersock->write(block);
        _seekersock->flush();

        block.clear();

        #ifndef AUTOHIDER_OLD
        if (_autoHider==NULL) {
        #endif
            // out << (quint16)0;
            out <<(quint16)1;
            out <<(quint16)status; //game status
            out << (quint16)_seeker->current.row;
            out << (quint16)_seeker->current.col;
            out.device()->seek(0);
            out << (quint16)(block.size() - sizeof(quint16));
            _hidersock->write(block);
            _hidersock->flush();
        #ifndef AUTOHIDER_OLD
        }
        #endif


        //cout<<"position messages sent to both, from seeker "<<endl;
        writetofile(); //log the actions in the files.
        _gmap->setHider(_hider->current);
        _gmap->setSeeker(_seeker->current);
        _hider->flag=0;
        _seeker->flag=0;
        //_w->repaintwidget(); //ag111201

        if(status)
            newgame();

    }
    else
    {
        // cout<<"next positions are not sent yet(h)!"<<endl;
    }

}

void HSTCPServer::sendinittoseeker() {
    sleep(1);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;
    out << (quint16)_hider->current.row;
    out << (quint16)_hider->current.col;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    _seekersock->write(block);
    _seekersock->flush();
    cout<<"the server sent the hinitpos to the seeker"<<endl;

}

void HSTCPServer::ondisconnect() {
    //AG120523: start new server instance
    //ag note: IS IT OK TO PASS PARAM, is it different than other?????
    cout << " on disconnect" << endl;
    startNewServerInstance();

    /*
    _proc=new QProcess();
    QStringList arg;
    arg<<_ip<<QString(_port);
    QString path=_filename;


    //path.append("/Marzo/server.ag/HideSeekGame.app/Contents/MacOS/HideSeekGame"); //mac
    cout << "the PATH global is: "<<PATH.toStdString()<<endl;;
    path.append("/Marzo/server.ag/hsserver"); //linux
    cout<<"call another server in : "<<path.toStdString()<<endl;

    if(_proc->startDetached(path,arg))
        cout<<"the new server process was started succesfully"<<endl;
    else
        cout<<"the new server process was NOT started"<<endl;

        */

    //AG120531: close (db/file/..)
    _gameLog->close();

    exit(0);

}



void HSTCPServer::callrandhider(int hiderType, QString actionFile) {

    _proc=new QProcess();
    QStringList arg;
    arg<<_ip<<QString::number(_port)<<QString::number(hiderType);
    //AG120903: add args for hider type action list
    if (hiderType==HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST) arg<< getFilePath(_config->getRecordedActionsPath(), actionFile);

    QString path=_config->getAutoHiderProg();
    cout << "Starting random hider: " << path.toStdString() << " | params: " << arg.join(",").toStdString()<<endl;

    bool ok = _proc->startDetached(path,arg);
    if(ok)
        cout<<"The auto-hider process was started succesfully"<<endl;
    else
        cout<<"The auto-hider process was NOT started"<<endl;

    //return ok;

    /*QString argn;
    argn = argn.number(n);
    cout<<"you called hider no "<<argn.toStdString()<<endl;
    //argn=(QString)n;
    _proc=new QProcess();
    QStringList arg;
    arg<<_ip<<QString(_port)<<argn;
    QString path=_filename;

    //path.append("/Marzo/Clientcopy1/HideSeekGame.app/Contents/MacOS/HideSeekGame"); //for mac
    path.append("/Marzo/Clientcopy1/hsserver"); //for linux
    cout<<"call hider dummy in : "<<path.toStdString()<<endl;

    _proc->startDetached(path,arg);*/

}

void HSTCPServer::callSeekroBot() {
    //AG120523: todo: also make option in config file
    cout << "WARNING: Seeker Robot oponent is NOT available in this version!"<<endl;
/*
    QString pomdpfile, policyfile,map, optype, usrname;// argn;
    //bool* ok = 0;


    switch(_map) {
    case 0:
        pomdpfile ="map1.pomdpx";
        policyfile ="map1.policy";
        break;
    case 1:
        pomdpfile ="map2.pomdpx";
        policyfile ="map2.policy";
        break;
    case 2:
        pomdpfile ="map3.pomdpx";
        policyfile ="map3.policy";
        break;
    case 3:
        pomdpfile ="map4.pomdpx";
        policyfile ="map4.policy";
        break;
    case 4:
        pomdpfile ="map5.pomdpx";
        policyfile ="map5.policy";
        break;
    case 5:
        pomdpfile ="map6.pomdpx";
        policyfile ="map6.policy";
        break;
    case 6:
        pomdpfile ="map7.pomdpx";
        policyfile ="map7.policy";
        break;
    case 7:
        pomdpfile ="map8.pomdpx";
        policyfile ="map8.policy";
        break;
    case 8:
        pomdpfile ="map9.pomdpx";
        policyfile ="map9.policy";
        break;
    case 9:
        pomdpfile ="map10.pomdpx";
        policyfile ="map10.policy";
        break;

    }

    map = map.number(_map);
    optype = optype.number(0);
    usrname = "SeekroBot";

    _proc=new QProcess();
    QStringList arg;


    arg<<pomdpfile<<policyfile<<_ip<<QString(_port)<<map<<optype<<usrname;
    QString path=_filename;

    //path.append("/Marzo/Seekrosegtest/APPL-Simulator-test.app/Contents/MacOS/HideSeekGame"); //for mac
    path.append("/Marzo/SeekroBot/APPL-Simulator-test"); //for linux
    cout<<"call SeekroBot in : "<<path.toStdString()<<endl;
    cout<<"11. seekerset: "<<_seeker->set<<", hiderset: "<<_hider->set<<endl;
    _proc->startDetached(path,arg);
    cout<<"12. seekerset: "<<_seeker->set<<", hiderset: "<<_hider->set<<endl;
*/
}



int HSTCPServer:: checkstatus() {
    Pos hp,sp, base, hpp, spp;
    hp = _hider->current;
    sp = _seeker->current;
    hpp = _hider->previous;
    spp = _seeker->previous;
    base = _gmap->getBase();

    if(status==-1)
        return -1;
    if((hp.row==base.row && hp.col==base.col) && (sp.row!=base.row || sp.col!=base.col) ) {
        cout<<"!!!!!!!!!!!!!!!!!!!!!!!!HIDER WINS!!!!!!!!!!!!!!!!!!!!"<<endl<<endl;
        _hider->win = 1;
        _seeker->win = 2;
        status=2;
        return 2;//hider wins
    }
                                        //AG121112:added crossing criterium
    int dhs = _gmap->distance(sp,hp); //AG130404: use distance to hider
    if( dhs<=_winDist /*((hp.row==sp.row) &&(hp.col==sp.col))*/ || ((hp.row==spp.row) && (hp.col==spp.col) && (hpp.row==sp.row) && (hpp.col==sp.col)) ) {
        // if( (hp.y<=sp.y+1) &&(hp.y>=sp.y-1) ){
        cout<<"!!!!!!!!!!!!!!!!!!!!!!!!SEEKER WINS!!!!!!!!!!!!!!!!!!!!"<<endl<<endl;
        _hider->win = 2;
        _seeker->win = 1;
        status=1;
        return 1;//seeker wins
        //  }
    }        

    //if((_hider->numa > MAXACTIONS-1) || (_seeker-> numa>MAXACTIONS-1) )
    if((_hider->numa > _maxActions-1) || (_seeker-> numa>_maxActions-1) ) //AG120511: updated use of max actions
    {
        cout<<"IT IS A TIE!!!!!!!!!!!!!!"<<endl<<endl;
        _hider->win = 3;
        _seeker->win = 3;
        status=3;
        return 3;
    }
    _hider->win = 0;
    _seeker->win = 0; //game on
    status=0;
    return 0;//game on
}

/*void PlayerInfo::MovePlayer(HSTCPServer* world,int action) {
    if(action<1 || action>9) {
        cout << "the action chosen is not valid! " << endl;
    }
     Pos position;
     //cout << "the action chosen is  valid! " << action <<endl;
     position=this->GetPos();

    // the curent position of the PlayerInfo
void newgame();

    Pos n = world->mvPlayer(action,position);
    cout<<n.x<<n.y<<endl;
    if (n.x==-1)  {//if the movement was not done
        cout<< "PlayerInfo's move did not happen"<<endl;
    }
    else{
        //add the action in the actionlist of the PlayerInfo
        *_action=action;
        *_action++;
        _curpos.x = n.x;
        _curpos.y = n.y;
        _f= 1;
        _numa++;
        cout<<"the new position of the PlayerInfo is: ("<<_curpos.x<<", "<<_curpos.y<<")"<<endl;

        this->calculatevisible(); //calculate it again according to current position
        cout<<"the vis map was recalculated"<<this->getPMap()->getVisibility(0,3)<<endl;
    }
}*/



#ifdef OLD_CHRYSO_CODE

Pos  HSTCPServer::mvPlayer(int action, Pos a) {

    //cout<<"Inside world´s function mvPlayer!"<<action<<x<<y<<endl;
    int rows, cols;
    Pos n;
    rows = _gmap->rowCount();
    cols = _gmap->colCount();
    //first make sure that indeed the current pos of the PlayerInfo agrees with the world state
    /*if(_gmap->getItem(a.x,a.y)!= 8) {
        cout<<"The world has wrong position of the PlayerInfo"<<endl;
        n.x=n.y=-1;
        return n;
    }*/

    //cout<<"Inside world´s function mvPlayer!"<<_gmap->getItem(x,y)<<endl;
    //cout<<rows<<cols<<endl;

    switch(action) {
    case 1: {
        n.row=a.row-1;
        n.col=a.col;
        if(n.row<0) {
            cout<<"out of borders!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 1) {
            cout<<"there is a wall there!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 2) cout<<"back to the base..."<<endl;
        _gmap->xcoChangeMap(a,n);
    }
    break; // move north
    case 2: {
        n.row=a.row-1;
        n.col=a.col+1;
        if( (n.row<0) ||(n.col>=cols) ) {
            cout<<"out of borders!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 1) {
            cout<<"there is a wall there!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 2) cout<<"back to the base..."<<endl;
        _gmap->xcoChangeMap(a,n);
    }
    break; // move north-east
    case 3: {
        n.row=a.row;
        n.col=a.col+1;
        if(n.col>=cols) {
            cout<<"out of borders!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 1) {
            cout<<"there is a wall there!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 2) cout<<"back to the base..."<<endl;
        _gmap->xcoChangeMap(a,n);
    }
    break; // move east
    case 4: {
        n.row=a.row+1;
        n.col=a.col+1;
        if( (n.row>=rows) ||(n.col>=cols) ) {
            cout<<"out of borders!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 1) {
            cout<<"there is a wall there!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 2) cout<<"back to the base..."<<endl;
        _gmap->xcoChangeMap(a,n);
    }
    break; // move east-south
    case 5: {
        n.row=a.row+1;
        n.col=a.col;
        if( n.row>=rows ) {
            cout<<"out of borders!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 1) {
            cout<<"there is a wall there!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 2) cout<<"back to the base..."<<endl;
        _gmap->xcoChangeMap(a,n);
    }
    break; // move south
    case 6: {
        n.row=a.row+1;
        n.col=a.col-1;
        if( (n.row>=rows) ||(n.col<0) ) {
            cout<<"out of borders!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 1) {
            cout<<"there is a wall there!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 2) cout<<"back to the base..."<<endl;
        _gmap->xcoChangeMap(a,n);
    }
    break; // move south-west
    case 7: {
        n.row=a.row;
        n.col=a.col-1;
        if(n.col<0) {
            cout<<"out of borders!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 1) {
            cout<<"there is a wall there!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 2) cout<<"back to the base..."<<endl;
        _gmap->xcoChangeMap(a,n);
    }
    break; // move west
    case 8: {
        n.row=a.row-1;
        n.col=a.col-1;
        if( (n.row<0) ||(n.col<0) ) {
            cout<<"out of borders!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 1) {
            cout<<"there is a wall there!"<<endl;
            n.row=n.col=-1;
            break;
        }
        if(_gmap->getItem(n.row,n.col)== 2) cout<<"back to the base..."<<endl;
        _gmap->xcoChangeMap(a,n);
    }
    break; // move west-north
    case 0: {
        cout<<"the position of the PlayerInfo is: ("<<a.row<<", "<<a.col<<")"<<endl;
        n.row=a.row;
        n.col=a.col;
    }
    break; //don´t move
    default:
        cout << " (unknown command)";
    }
    cout << endl;
    return n;
}
#endif

int HSTCPServer::writetofile() {

    //cout<<"Write to File..."<<endl;

    if( !fd.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text) )
    {
        std::cout << "Failed to open file h." << std::endl;
        return -1;
    }
    QTextStream out(&fd);   // we will serialize the data into the file

    out << _hider->numa; //when a game starts the action number will be 0;
    out << '\t';
    //out << _hider->timestamp.toString();
    out << _hider->timestamp.toTime_t();
    out << '\t';
    out << _hider->action[_hider->numa-1]; //when a game starts the action will be -1;
    out << '\t';
    out << _hider->current.row; //current position of hider
    out << '\t';
    out << _hider->current.col;
    out << '\t';
    //out << _hider->timestamp.toString();
    out << _seeker->timestamp.toTime_t();
    out << '\t';
    out << _seeker->action[_seeker->numa-1]; //when a game starts the action will be -1;
    out << '\t';
    out << _seeker->current.row; //current position of hider
    out << '\t';
    out << _seeker->current.col;
    out << '\t';
    out << status<<endl;
    //cout << status<<status<<status<<status<<endl;


    fd.close();

    //AG120530: write to log
    _gameLog->addGameStep(_hider->numa, _hider->timestamp, _seeker->timestamp, _hider->action[_hider->numa-1], _seeker->action[_seeker->numa-1],
                          _hider->current.row, _hider->current.col,_seeker->current.row, _seeker->current.col, status);

    return 1;
}



void HSTCPServer::newgame() {
    if(status==-1)
        return;//the game is already new!
    //_gmap->initmap();
    //initplayers();

    sleep(6);

    #ifndef AUTOHIDER_OLD
    if (_autoHider==NULL)
    #endif
        _hidersock->disconnect();

    _seekersock->disconnect();

    //AG120531: close (db/file/..)
    _gameLog->close();

    exit(0);
}


void HSTCPServer::initplayers() {
    //cout<<"initplayers"<<endl;

    for(int i=0; i<MAXACTIONS -1; i++)
    {
        _hider->action[i]=-1;
        _seeker->action[i]=-1;
    }
    //cout<<"  init actions done"<<endl;

    int r=0,c=0;
    char tch;
    int tch1=0;
    int tch2=0;
    r = _gmap->rowCount();
    c = _gmap->colCount();

    //AG121112: init previous pos
    _hider->previous.row = _hider->previous.col = _seeker->previous.row = _seeker->previous.col = -1;


    //cout <<"  loop through map"<<endl;
    assert(_hider!=NULL);
    assert(_seeker!=NULL);
    assert(_gmap!=NULL);

    status=-1;
    for (int tr=0; tr<r; tr++) {
        for (int tc=0; tc<c; tc++) {
            tch=_gmap->getItem(tr,tc);

            if (tch==9) {                
                _hider->current.row=tr;
                _hider->current.col=tc;
                _hider->win=-1;
                _hider->numa=0;
                _hider->flag=1;

                tch1=1;
                 //cout<<"hider"<<endl;
            }
            if (tch==8) {
                _seeker->current.row=tr;
                _seeker->current.col=tc;
                _seeker->win=-1;
                _seeker->numa=0;
                _seeker->flag=1;
                tch2=1;
                //cout<<"seeker"<<endl;
            }
        }
    }

    if(tch1==0)
    {
         cout<<"hider not found so put him at 0,0"<<endl;
        Pos p;
        p.row=0;
        p.col=0;
        _hider->current.row=p.row;
        _hider->current.col=p.col;
        _gmap->setHider(p);
        _hider->win=-1;
        _hider->numa=0;
        _hider->flag=1;

    }
    if(tch2==0)
    {
        cout<<"seeker not found so put him next to the base"<<endl;
        Pos p;
        cout << " base:"<<flush;
        p=_gmap->getBase();
        cout <<p.toString()<<endl;
        //p.y+=1;//AG120313: seeker pos on the base always (instead of below)
        _gmap->setSeeker(p);
        cout << "seeker set "<<endl;
        _seeker->current.row=p.row;
        _seeker->current.col=p.col;
        _seeker->win=-1;
        _seeker->numa=0;
        _seeker->flag=1;
    }

    //cout <<"  done"<<endl;

}


QString HSTCPServer::getMapName(int mapID) {
    if (mapID==HSGlobalData::MAP_PASSED_BY_NETWORK) { //AG130724: passed map by network
        return QString("(sent_by_client)");
    } else {
        return HSGlobalData::MAPS[mapID];
    }
}

void HSTCPServer::initializeLogs() {

    //open the files with the proper names
    //writes the firstinfo of the game

    // cout<<endl<<endl;
    QString fn;
    //QString filename = PATH;

    QStringList path;
    //path = _filename.split("/");

    //AG130724: passed map by network
    fn = getMapName(_map);
    QString couple;

    couple=_hider->username;
    //ag111205
    couple.append("[");
    //AG120608
    QString hiderAdr;
    #ifndef AUTOHIDER_OLD
    if (_autoHider!=NULL)
        hiderAdr = "autohider";
    else
    #endif
        hiderAdr = _hidersock->peerAddress().toString();

    couple.append(hiderAdr); //NOTE: should check for NULL of peer addreess
    couple.append("]_vs_");
    couple.append(_seeker->username);
    //ag111205
    couple.append("[");
    couple.append(_seekersock->peerAddress().toString()); //NOTE: should check for NULL of peer addreess
    couple.append("]_");
    fn.prepend(couple);

    //cout<<fn.toStdString()<<endl;
    /* path.append("Marzo/logs");
    path.append(fn);
    */

    //AG120903: put right file name (file sep)
    //path << _config->getLogPath() << fn;

    fn=getFilePath(_config->getLogPath(),fn);
            //path.join("/");

    cout << "Initializing log to: " << fn.toStdString()<<endl;
    //cout<<fn.toStdString()<<endl;
    //Qstring fn should contain the full pathname of the file to beopened.. < HidernameVsSeekername_mapx.txt>

    fd.setFileName(fn);
    _hider->timestamp=_seeker->timestamp= QDateTime::currentDateTime();

    //AG120530: open log
    _gameLog->startGame(getMapName(_map),_gmap,_seeker->username,_hider->username,_maxActions);

    writetofile();
}



void HSTCPServer::sendmaptoPlayers() {
    //send to players the info of the map.
    cout << "send map to players"<<endl;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    Pos p;

    #ifndef AUTOHIDER_OLD
    if (_autoHider==NULL) { // ie the hider is not automatic
    #endif
        out << (quint16)0; //block size
        out << (quint16)_map; //the int of the map (so that both players know which map it is)
        out << (quint16)0; //hider
        out << (quint16)_gmap->rowCount();
        out << (quint16)_gmap->colCount();
        out << (quint32)_gmap->numObstacles(); // ag130304: renamed from getobstnum(); //ag130724: quint32 !!

        for(int i=0; i<_gmap->numObstacles(); i++) {
            p=_gmap->getObstacle(i);
            //cout << " o("<<p.row<<","<<p.col<<"),"<<flush;
            out << (quint16)p.row;
            out << (quint16)p.col;
        }
        p=_gmap->getBase();
        out << (quint16)p.row;
        out << (quint16)p.col;
        // cout<<p.x<<"/"<<p.y<<endl;
        p = _hider->current;
        out << (quint16)p.row; //hider
        out << (quint16)p.col;
        p = _seeker->current;
        out << (quint16)p.row;
        out << (quint16)p.col;//seeker
        out << _seeker->username;

        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        _hidersock->write(block);//send to hider
        _hidersock->flush();
        block.clear(); //clear block
    #ifndef AUTOHIDER_OLD
    }
    #endif

    out << (quint16)_map; //the int of the map (so that both players know which map it is)
    out << (quint16)1; //seeker
    out << (quint16)_gmap->rowCount();
    out << (quint16)_gmap->colCount();
    out << (quint32)_gmap->numObstacles(); //ag130724: quint32 !!
    for(int i=0; i<_gmap->numObstacles(); i++) {
        p=_gmap->getObstacle(i);
        out << (quint16)p.row;
        out << (quint16)p.col;
    }
    p=_gmap->getBase();
    out << (quint16)p.row;
    out << (quint16)p.col;
    p = _seeker->current;//seeker
    out << (quint16)p.row;
    out << (quint16)p.col;
    p = _hider->current;
    out << (quint16)p.row; //hider
    out << (quint16)p.col;
    out << _hider->username;

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    _seekersock->write(block); //send to seeker
    _seekersock->flush();

    _hider->flag=0;
    _seeker->flag=0;
}


bool HSTCPServer::startNewServerInstance() {
    _proc=new QProcess();
    QStringList arg;

    //WARNING: ERROR HERE!!!

    //AG120515: old path to prog
    /*arg<<_ip<<QString(_port);
    QString path=_filename;

    //path.append("/Marzo/server.ag//HideSeekGame.app/Contents/MacOS/HideSeekGame"); //mac
    cout << "the PATH global is: "<<PATH.toStdString()<<endl;;
    path.append("/Marzo/server.ag/hsserver"); //linux

    cout<<"call another server in : "<<path.toStdString()<<endl; */


    //TODO:: WHY DOES IT CRASH HERE ??? DUE _config ????

    /*cout << "starting new server instance"<<endl;
    cout << " config ==null: "<<(_config==NULL)<<endl;

    string s=_config->getXMLFile().toStdString();
    cout << "string"<<endl;
    cout << s<<endl;

    cout << " config.xmlfile: "<< _config->getXMLFile().toStdString()<<endl;*/

    //AG120525: problem solved of XML -> generated a 'new config' instaed of local config in HSServer

    arg << _config->getXMLFile();
    QString path=_config->getServerProg();

    cout << "Starting new server instance: "<<path.toStdString()<<" | params: "<<arg.join(",").toStdString()<<endl;

    bool ok = _proc->startDetached(path,arg);
    if(ok)
        cout<<"the new server process was started succesfully"<<endl;
    else
        cout<<"the new server process was NOT started"<<endl;

    return ok;
}


void raytrace(char **m, int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int x = x0;
    int y = y0;
    int n = 1 + dx + dy;
    int x_inc = (x1 > x0) ? 1 : -1;

    int y_inc = (y1 > y0) ? 1 : -1;
    int error = dx - dy;
    dx *= 2;
    dy *= 2;

    for (; n > 0; --n)
    {
        if( m[x][y] == 9 ) {
            m[x][y]= 1;
        }
        else if ( m[x][y] == 2 ) {
            m[x1][y1]= 0;
            return;
        }

        if (error > 0)
        {
            x += x_inc;

            error -= dy;
        }
        else
        {
            y += y_inc;
            error += dx;
        }
    }
}


