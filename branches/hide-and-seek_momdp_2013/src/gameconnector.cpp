
#include "gameconnector.h"


#include <QBool>
//#include <QtNetwork>

#include <iostream>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#include "hsglobaldata.h"
#include "hsconfig.h"

using namespace std;

GameConnector::GameConnector(string ip, int port, int type, AutoPlayer* autoplayer, SeekerHS* seekerHS, int mapID, int optype,
                             string username, int numofgames, string actionFileList, int winDist, GMap* gmap) {
        _port= port;
        _ip = QString::fromStdString(ip);
        _map = mapID;
        _optype = optype;
        _numofgames = numofgames;
        _actionFileList =  QString::fromStdString(actionFileList); //ag120904: action file list
        _winDist = winDist;
        _seekerHS = seekerHS;
        _player = new Player();

        init(type, _player, autoplayer, username, gmap);
}

GameConnector::~GameConnector(){    
}


void GameConnector::init(int type,  Player* pl, AutoPlayer* autoplayer, string username, GMap* gmap){
    //if (gmap==NULL) {
        _gmap = pl->getMap(); //use this map,.. it will write it from server even if it's already set..
    //} else {
    //    _gmap = gmap;
    //}
    _player = pl;
    _autoplayer = autoplayer; //ag130226:autoplayer //ag120112:momdp

    _player->sethidertype(type);
    DEBUG_CONNECTOR(cout<<"rh: the hidertype at the automated player is "<<type<<endl;);

	//setusername of the player, depending on the chosen type
    if(_player->getType()==1) {
        _player->setusername(username);
        DEBUG_CONNECTOR(cout<<"My name is "<<username<<endl;);
    } else if (_player->gethidertype()==1) {
        _player->setusername("SmartHider");
        DEBUG_CONNECTOR(cout<<"I am SmartHider: ";);
    } else if (_player->gethidertype()==0) {
        _player->setusername("RandomHider");
        DEBUG_CONNECTOR(cout<<"I am RandomHider: ";);
    }

    DEBUG_CONNECTOR(cout<<"rh: "<<_ip.toStdString()<<":"<<_port<<" : "<<_player->getusername()<<endl;);

    //tcpSocket = new QTcpSocket();
    tcpSocket.connectToHost(_ip,_port); //connect to the server
    if(tcpSocket.waitForConnected(3000))
    {
        cout<<"rh: connected!"<<endl;
        connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(ondisconnect()));

        //send the player name to the server.
        sendInfomsg(gmap);

        while (!tcpSocket.waitForReadyRead(6000))
        {
            cout <<"."<<flush;
        }
        //read the map info that server sends
        /**********MAP INFO MSG*************/
        quint16 blockSize=0;
        QDataStream in(&tcpSocket);
        in.setVersion(QDataStream::Qt_4_0);
        if (blockSize == 0) {
                if (tcpSocket.bytesAvailable() < (int)sizeof(quint16)){
                    tcpSocket.flush();
                    DEBUG_CONNECTOR(cout<<"rh: less bytes in the array than expected1."<<endl;);
                        return;
                }
                in >> blockSize;
        }
        if (tcpSocket.bytesAvailable() < blockSize){
            tcpSocket.flush();
            DEBUG_CONNECTOR(cout<<"rh: less bytes in the array than expected2. blocksize="<<blockSize<<endl;);
                return;
        }

        quint16 q,x,y,rows,cols;

        in >> q; //map _id
        _map = (int)q;

        DEBUG_CONNECTOR(cout<<"rh: playing map no: "<<endl;);
        in >> q; // player's type (random or smart)
        _player->setType((int)q);
        DEBUG_CONNECTOR(cout<<"rh: "<<_player->getType()<<"/"<<(int)q<<endl;);

        /*in >> q; //num of rows
        _gmap->setrows((int)q);
        cout<<"rh: "<<_gmap->rowCount()<<"/"<<(int)q<<endl;
        in >> i; //num of cols
        _gmap->setcols((int)i);
        cout<<"rh: "<<_gmap->colCount()<<"/"<<(int)i<<endl;
        //_gmap->createMap(q,i);*/
        //AG130227: create map with correct size
        in >> rows;
        in >> cols;
        cout << "map size: "<<rows<<"x"<<cols<<endl;
        _gmap->createMap(rows,cols);

        quint32 obstCount;
        in >> obstCount; // num of obstacles
        unsigned int count;
        cout<<"rh: Obstacles : "<<obstCount<<endl;
        for(count=0; count<obstCount;count++)
        {
            in >> x; //one by one all the obstacle locations (x=row, y=col)
            in >> y;
            _gmap->addObstacle(x,y);
            DEBUG_CONNECTOR(cout<<"rh: "<<"  "<<(int)x<<"/"<<(int)y<<endl;);
        }
        //base location
        in>>x;  //row
        in>>y; //col
        _gmap->setbase((int)x,(int)y);
        cout<<"rh: "<<"Base : "<<_gmap->getBase().row<<"/"<<_gmap->getBase().col<<endl;

        //ag130304: set fixed
        _gmap->setMapFixed();

        //player's position
        in>>x; //x=row, y=col
        in>>y;
        _player->setcurpos(x,y);
        cout<<"rh: "<<"my pos : "<<(int)x<<"/"<<(int)y<<endl;
        if (_player->getType())
            _gmap->setInitials((int)x,(int)y);
        else
            _gmap->setInitialh((int)x,(int)y);
        in>>x;
        in>>y;
        _player->setoppos(x,y);
        DEBUG_CONNECTOR(cout<<"rh: "<<"opponent's pos' : "<<(int)x<<"/"<<(int)y<<endl;);
        if (_player->getType())
            _gmap->setInitialh((int)x,(int)y);
        else
            _gmap->setInitials((int)x,(int)y);

        QString un;
        in>>un; //username
        Pos playerPos, opPlayerPos;
        tcpSocket.flush();
        playerPos=_gmap->getInitials();   //_player->Getcurpos();
        opPlayerPos=_gmap->getInitialh(); // _player->Getoppos();
        if(!_player->getType()) {//hider
            _gmap->setSeeker(opPlayerPos);
            cout<<"rh: "<<"You are the hider!"<<endl;
            cout<<"rh: "<<"Choose an initial position by clicking on a cell."<<endl;
            _player->setopusername(un.toStdString());
        }
        else {//seeker
           _gmap->setSeeker(playerPos);
           //_player->calculatevisible(); //ag130228: disabled
           cout<<"rh: "<<"You are the seeker!"<<endl;
           _player->setopusername(un.toStdString());
        }

        _player->addsmsg();
        //_player->setf(); //ready to send his action

         //send to server action taken and current position
        if(_player->getType()){//if its the seeker
            cout<<" read the initial pos of the hider plzzzzz"<<endl;

            receivehinitpos();
            opPlayerPos=_gmap->getInitialh(); // _player->Getoppos();
            //init belief //ag130320: put pos of hider and seeker
            bool visib = _gmap->isVisible(playerPos,opPlayerPos);


#ifdef SEEKERHS_CHECKS
            //extra check of the 'SeekerHS.checkValid*' functions

            if (_seekerHS!=NULL) {

                cout << "SEEKERHS_CHECKS for init"<<endl;
                //do check before init

                _gmap->printMap();;

                _seekerHS->setMap(_gmap);

                //fill vectors
                vector<double> sPosVec, hPosVec;
                sPosVec.resize(2);
                sPosVec[0] = playerPos.row;
                sPosVec[1] = playerPos.col;
                if (!visib) {
                    hPosVec.resize(0);
                } else {
                    hPosVec.resize(2);
                    hPosVec[0] = opPlayerPos.row;
                    hPosVec[1] = opPlayerPos.col;
                }

                cout << " real check"<<endl;

                //seeker pos
                bool checkSeekerPos = _seekerHS->checkValidNextSeekerPos(sPosVec,false);
                if (!checkSeekerPos) {
                    cout << "[check valid seeker pos FAILED]"<<endl;
                }

                //hider pos
                bool checkHiderPos = _seekerHS->checkValidNextHiderPos(hPosVec, sPosVec,false);
                if (!checkHiderPos) {
                    cout << "[check valid hider pos FAILED]"<<endl;
                }

                if (checkHiderPos && checkSeekerPos) {
                    cout << "[seeker+hider pos checks SUCCEEDED]"<<endl;
                }

                cout << "SEEKERHS_CHECKS for init DONE"<<endl;

                _seekerHS->init(sPosVec,hPosVec);

                cout << "SEEKERHS_INIT DONE"<<endl;

            } else {
                _autoplayer->initBelief(_player->getMap(),playerPos,opPlayerPos,visib);
            }
#else
            _autoplayer->initBelief(_player->getMap(),playerPos,opPlayerPos,visib);
#endif


            takenewseekeraction();
        }
        if (_player->gethidertype()==0){ //if this is the random player
            //tcpSocket->flush();
             _player->setf();
            setInitialPos();
            takenewaction();

        } else if (_player->gethidertype()==1)//if this is the smart player
        {
              _player->setf();
            //calculate the distance map
            setInitialPos();
            //tcpSocket->flush();
            _gmap->createPropDistPlanner();
            //get possible moves
            _player->getpossiblemoves();
            //score all pos moves
            _player->scoreneighb();
            int a = _player->chooseaction();
            //take action
            takenewaction(a);
        }


        //connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));

	//AG111202: test if ready for read..
        _stopped = false;
        cout << "ready read test..."<<endl;
        readyReadTest();
        cout << "done ready read"<<endl;
    }
    else
        cout<<"rh: "<<"not connected"<<endl;
}

void GameConnector::ondisconnect() {
    exit(0);
}


void GameConnector::setInitialPos(){
// it randomly places the hider in one of the invisible for the seeker cells

	Pos t;
        int ok=0;

    cout<<"before while in initial pos"<<endl;
    while(!ok){

        if(_map==0)//if it is the first map with no Invisible cells
        {
            srand((unsigned)time(0));
            int n=rand()%_gmap->rowCount();
            t.row=n;
            int k=rand()%_gmap->colCount();
            t.col=k;
            cout<< "rh: the initial pos of  hider is: ("<<t.row<<", "<<t.col<<")"<<endl;
            ok=1;
        }
        else{
             //_player->calculateInvisible();//calculates all the invisible cells //ag130228: disabled
             //and then chooses randomly one of them as an initial position

             srand((unsigned)time(0));

             /*
             int n=rand()%_player->get_n();
             t = _player->getInvisiblePoints(n);
             */ // AG130301: changed
             vector<Pos> invisPos = _player->getInvisiblePoints();
             int randI = rand() % invisPos.size();
             t = invisPos[randI];

             ok=1;
        }


         if(t.row==_gmap->getBase().row && t.col==_gmap->getBase().col)
            ok=0;
         if ( (t.row==_player->Getoppos().row) && (t.col==_player->Getoppos().col) )
            ok=0;
     }
    _player->setcurpos(t.row,t.col);
    _gmap->setHider(t);
    _gmap->setInitialh(t.row,t.col);

    //the hider sends his initial position

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out <<(quint16)0;
    out <<(quint16)t.row;
    out <<(quint16)t.col;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket.write(block);
    tcpSocket.flush();
    cout<<"rh: "<<"hider set his initial pos and sent it to server"<<endl;

    //_player->calculatevisible(); //ag130301: disabled

}


void GameConnector::sendInfomsg(GMap* gmap){
    //send the player name to the server.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out <<(quint16)0;

    //AG130723
    if (gmap!=NULL) {
        cout << "sending map info: "<<gmap->rowCount()<<"x"<<gmap->colCount()<<endl;
        //send map
        out << HSGlobalData::MAP_PASSED_BY_NETWORK;
        //rows,cols
        out << (quint16)gmap->rowCount();
        out << (quint16)gmap->colCount();
        //base
        Pos base = gmap->getBase();
        cout <<"base: "<<base.toString()<<endl<<"obstacle: "<<gmap->numObstacles()<<endl;
        out << (quint16)base.row;
        out << (quint16)base.col;
        //obstacles
        out << (quint32)gmap->numObstacles(); //NOTE: 32 bits!!
        for(int i=0; i<gmap->numObstacles();i++) {
            Pos obst = gmap->getObstacle(i);
            out << (quint16)obst.row;
            out << (quint16)obst.col;
        }
    } else {
        out <<(quint16)_map;
    }

    //ag130404: win dist
    if (_winDist>0) {
        out << (quint16)HSGlobalData::SERVER_WIN_DIST_CODE;
        out << (quint16)_winDist;
    }

    out <<(quint16)_optype;//opp irrelevant
    //AG120904: send name of action file list if that is the opponent
    if (_optype==HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST) {
        out << _actionFileList;
    }
    out <<(quint16)_player->getType(); //type of  player
    out << QString::fromStdString(_player->getusername());

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket.write(block);
    tcpSocket.flush();
    cout<<"rh: "<<"player sent his name : "<<_player->getusername()<<endl;

}

//AG111202: readReadyTest replaces the signal-slot solution (although that still should be the best solution)
void GameConnector::readyReadTest() {

    while (!_stopped) {
        cout <<"-"<<flush;
        //cout << "socketisopne: " <<tcpSocket.isOpen()<<endl;

        while (!tcpSocket.waitForReadyRead(1000) && !_stopped)
        {
           cout << '.'<<flush;

           tcpSocket.flush();
           sleep(1);
        }
        cout <<"-"<<flush;
        
        if (_stopped) return; //end
        
        cout << "[ready read]"<<endl;
        readyRead();
    }
}

void GameConnector::readyRead() {
//msg format:  blocksize,  0, curpos(x,y), errorstring
 //            or
 //             blocksize, 1, oppos(x,y)



    cout<<"rh:  Inside readyRead()!!!!!"<<endl;

    quint16 blockSize=0;
    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if (blockSize == 0) {
            if (tcpSocket.bytesAvailable() < (int)sizeof(quint16)){
               // cout<<"less bytes in the array than expected1."<<endl;
                tcpSocket.flush();
                    return;
            }
            in >> blockSize;
    }
    if (tcpSocket.bytesAvailable() < blockSize){
       cout<<"rh: "<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
       tcpSocket.flush();
        return;
    }

    quint16 q, s, x, y;
    QString str;
    in >> q;
    in >> s;
    if(s!=0){

        cout<<"rh: "<<(int)s<<"! GAME OVER !"<<(int)s<<endl;
        if(_player->getType()==1) {//if its the seeker
            cout<<"do you want to play some more?"<<endl;
            tcpSocket.disconnectFromHost();
            exit(0);
        }
        else{
        tcpSocket.disconnectFromHost();
        exit(0);
        }

    }
    if(q==0)//validation int
    {//that means the previous action was not done from the server

        exit(1);
    }

    in >> x;//read your opponent's current position
    in >> y;
    tcpSocket.flush();
    _player->setoppos((int)x, (int)y);
    Pos p;
    p.row=(int)x;
    p.col=(int)y;
  //  cout<<"Opponent of "<<_player->getType()<<" :    "<<p.x<<" / "<<p.y<<endl;
    if(!_player->getType()) //hider
        _gmap->setSeeker(p);
    else
        _gmap->setHider(p);

    _player->addsmsg();
    _player->setf(); //ready to send my action
    //_player->calculatevisible(); //ag130301: disabled

    if( _player->getType()){ //SEEKER

        takenewseekeraction();
    }else
    if(!_player->gethidertype()) //random hider
        takenewaction();
    else { //smart hider
        //get possible moves
        _player->getpossiblemoves();
        //score all pos moves
        _player->scoreneighb();
        int a = _player->chooseaction();
        //take action
        takenewaction(a);
    }


}

void GameConnector::takenewaction(int a){

    int n;
    n=a;

    if(a==-1){
    //get a random int from 0-8 to define next action
    srand((unsigned)time(0));
    n=rand()%9;
    }

    while(! _player->move(n))
        n=rand()%9;

    if(sendaction())
       cout<<"rh: action sent"<<endl;

}

void GameConnector::takenewseekeraction(){

    /***********************ALEX************************************************/
   /*At this point we have available the observations. (the first time is the initial position of the seeker)
           _gmap->_map is the map (access through _gmap->getItem(r,c) etc..)
           _gmap->_visible the visibility map (_gmap->getVisibletab() etc..)
           _player->_oppos the opponent position (_player->Getoppos(), _player->getopusername() ...)
           _player->_curpos the current positionof the seeker. (_player->Getcurpos())

           also available _player->getpossiblemoves() in respect with the map and its restrictions)*/



        // feed observations to solver


    Pos seekerPos = _player->Getcurpos();
    Pos hiderPos = _player->Getoppos();
    bool visible = _gmap->isVisible(seekerPos,hiderPos);

#ifdef SEEKERHS_CHECKS
    //extra check of the 'SeekerHS.checkValid*' functions
    int action = -1;

    if (_seekerHS!=NULL) {
        //do check before init

        //fill vectors
        vector<double> sPosVec, hPosVec;
        sPosVec.resize(2);
        sPosVec[0] = seekerPos.row;
        sPosVec[1] = seekerPos.col;
        if (!visible) {
            hPosVec.resize(0);
        } else {
            hPosVec.resize(2);
            hPosVec[0] = hiderPos.row;
            hPosVec[1] = hiderPos.col;
        }

        //seeker pos
        bool checkSeekerPos = _seekerHS->checkValidNextSeekerPos(sPosVec,!_seekerHS->getParams()->allowInconsistObs);
        if (!checkSeekerPos) {
            cout << "[check valid seeker pos FAILED]"<<endl;
        }

        //hider pos
        bool checkHiderPos = _seekerHS->checkValidNextHiderPos(hPosVec, sPosVec,!_seekerHS->getParams()->allowInconsistObs);
        if (!checkHiderPos) {
            cout << "[check valid hider pos FAILED]"<<endl;
        }

        if (checkHiderPos && checkSeekerPos) {
            cout << "[seeker+hider pos checks SUCCEEDED]"<<endl;
        }

        vector<double> sPosVecNew;
        int winState=-1;

        action =_seekerHS->getNextPose(sPosVec,hPosVec,sPosVecNew,&winState);

        cout << "Action: "<<action<<" new pos: ("<<sPosVecNew[0]<<","<<sPosVecNew[1]<<") winState="<<winState<<endl;

    } else {
        action = _autoplayer->getNextAction(seekerPos,hiderPos,visible);
    }


#else
    int action = _autoplayer->getNextAction(seekerPos,hiderPos,visible);
#endif





/********************ALEX****************************/
/*Next should be defined the next action that the automated seeker will take.
  At the end the player moves according to the action (MovePlayer(action))
  and the action is send to the server.*/


     // the action of the seeker is 0

    //get next action (int in [0-8]) from policy



    while (! _player->move(action)){ //move according to action
        cout<<"the action was not valid"<<endl;
        action=0; //dont move
    }

//sends to the server the seeker's current position(after the action) and the last action taken < player::_curpos,player::_action >
    if(sendaction())
       cout<<"rh: action sent"<<endl;
}


bool GameConnector::sendaction() {
//msg format: action, current position(row,col)
    if(!_player->getf()) //the action was not done
    {
        cout<<"rh: Action Aborted"<<endl;
        return 0;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;

    out <<(quint16)_player->getlastaction();
    Pos p;
    p=_player->Getcurpos();
    out << (quint16)p.row;
    out << (quint16)p.col;

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket.write(block);
    tcpSocket.flush();
    _player->unsetf();
    _player->addpmsg();
    cout<<"rh: action sent! "<<_player->getlastaction()<<p.row<<p.col<<endl;

    return 1;

}

void GameConnector::receivehinitpos()
{
    cout<<"waiting to get opponent's initial pos"<<endl;

    while (!tcpSocket.waitForReadyRead(6000))
    {

    }
    //read
    quint16 blockSize=0;
    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if (blockSize == 0) {
            if (tcpSocket.bytesAvailable() < (int)sizeof(quint16)){
                cout<<"rh: less bytes in the array than expected1."<<endl;
                    return;
            }
            in >> blockSize;
    }
    if (tcpSocket.bytesAvailable() < blockSize){
        cout<<"rh: less bytes in the array than expected2. blocksize="<<blockSize<<endl;
            return;
    }
    quint16 x, y;
    Pos p;
    in >> x;//hider's initial position
    p.row=(int)x;
    in >> y;
    p.col=(int)y;
    cout<<"seeker: my opponent's positions is: "<<p.row<<p.col<<endl;
    tcpSocket.flush();
    _player->setoppos(p.row,p.col);
    _gmap->setHider(p);
    _gmap->setInitialh(p.row,p.col);
    _player->setf();
        return;

}


void GameConnector::setSeekerHS(SeekerHS *seekerHS) {
    _seekerHS = seekerHS;
}
