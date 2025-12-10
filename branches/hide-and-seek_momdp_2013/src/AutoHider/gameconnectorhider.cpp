
#include "gameconnectorhider.h"


#include <QBool>
//#include <QtNetwork>

#include <iostream>
#include <math.h>
#include <assert.h>

#include "hsconfig.h"

#include "hsglobaldata.h"

using namespace std;

GameConnectorHider::GameConnectorHider(QString ip, int port, AutoHider* autoHider /*int hiderType, QString actionFile*/) {
//GameConnectorHider(char * ip, char* port, char* type) {


    _port= port;//atoi(port);
    _ip = (QString)ip;

    DEBUG_HS(cout<<"rh: the hidertype at the automated player is "<<hiderType<<endl;);
    _player.sethidertype(autoHider->getHiderType());
    _player.setusername(autoHider->getName());


    init(autoHider);//hiderType,actionFile); ///*atoi(type),*/ new Player());

}

GameConnectorHider::~GameConnectorHider() {

}


void GameConnectorHider::init(AutoHider* autoHider) { //int hiderType, QString actionFile) { //int type, Player* pl){
    _gmap = _player.getMap(); //was getPMap

    _autoHider = autoHider;
    //_player = pl;

    /*_player.sethidertype(type);
    cout<<"rh: the hidertype at the automated player is "<<type<<endl;*/

    //AG120903: put hider type constants
/*    switch (hiderType) {
        case HSGlobalData::OPPONENT_TYPE_HIDER_SMART: {
            _player.setusername("SmartHider");
            _autoHider = new SmartHider();
            cout<<"Starting SmartHider... "<<endl;
            break;
        }
        case HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING: {
            _player.setusername("AllKnowingSmartHider");
            _autoHider = new SmartHider(true);
            cout<<"Starting AllKnowingSmartHider... "<<endl;
            break;
        }
        case HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM: {
            _player.setusername("RandomHider");
            _autoHider = new RandomHider();
            cout<<"Starting RandomHider... "<<endl;
            break;
        }
        case HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST: {
            RandomListHider* randomListHider = new RandomListHider(actionFile.toStdString());
            QString name = "ActionList_" + QString::fromStdString(randomListHider->getActionListName());
            _player.setusername(name.toStdString());
            _autoHider = randomListHider;
            cout << "Starting RandomListHider..."<<endl;
            break;
        }
        case HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART: {
            _player.setusername("VerySmartHider");
            _autoHider = new VerySmartHider();
            cout << "Starting VerySmartHider..."<<endl;
            break;
        }
        case HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING: {
            _player.setusername("AllKnowingVerySmartHider");
            _autoHider = new VerySmartHider(true);
            cout<<"Starting AllKnowingVerySmartHider... "<<endl;
            break;
        }
        default:
            //not hider
            cout << "ERROR: unknown hider"<<endl;
            assert(false);
            break;
    }*/
    _autoHider->setMap(_gmap);

    DEBUG_HS(cout<<"rh: "<<_ip.toStdString()<<" : "<<_port<<" : "<<_player.getusername()<<endl;);

    tcpSocket = new QTcpSocket();
    tcpSocket->connectToHost(_ip,_port);
    if(tcpSocket->waitForConnected(3000)) {
        DEBUG_HS(cout<<"rh: connected!"<<endl;);
        connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(ondisconnect()));

        //send the player name to the server.
        sendInfomsg();

        while (!tcpSocket->waitForReadyRead(6000)) {

        }
        //read
        quint16 blockSize=0;
        QDataStream in(tcpSocket);
        in.setVersion(QDataStream::Qt_4_0);
        if (blockSize == 0) {
            if (tcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
                DEBUG_HS(cout<<"rh: less bytes in the array than expected1."<<endl;);
                return;
            }
            in >> blockSize;
        }
        if (tcpSocket->bytesAvailable() < blockSize) {
            cout<<"rh: less bytes in the array than expected2. blocksize="<<blockSize<<endl;
            return;
        }



        quint16 q,i,x,y;
        in >> q;
        _map = (int)q;
        DEBUG_HS(cout<<"rh: playing map no: "<<endl;);
        in >> q;
        _player.setType((int)q);
        DEBUG_HS(cout<<"rh: "<<_player.getType()<<"/"<<(int)q<<endl;);
        in >> q;
        _gmap->setrows((int)q);
        DEBUG_HS(cout<<"rh: "<<_gmap->rowCount()<<"/"<<(int)q<<endl;);
        in >> i;
        _gmap->setcols((int)i);
        DEBUG_HS(cout<<"rh: "<<_gmap->colCount()<<"/"<<(int)i<<endl;);
        _gmap->createMap(q,i);

        in >> i;
        int count;
        DEBUG_HS(cout<<"rh: Obstacles : "<<(int)i<<endl;);
        for(count=0; count<(int)i; count++) {
            in >> x;
            in >> y;
            //_gmap->setWall(x,y);
            _gmap->addObstacle(x,y);
            DEBUG_HS(cout<<"rh: "<<"  "<<(int)x<<"/"<<(int)y<<endl;);
        }
        in>>x;
        in>>y;
        _gmap->setbase((int)x,(int)y);
        DEBUG_HS(cout<<"rh: "<<"Base : "<<_gmap->getBase().row<<"/"<<_gmap->getBase().col<<endl;);
        in>>x;
        in>>y;
        _player.setcurpos(x,y);
        DEBUG_HS(cout<<"rh: "<<"my pos : "<<(int)x<<"/"<<(int)y<<endl;);
        if (_player.getType())
            _gmap->setInitials((int)x,(int)y);
        else
            _gmap->setInitialh((int)x,(int)y);
        in>>x;
        in>>y;
        _player.setoppos(x,y);
        DEBUG_HS(cout<<"rh: "<<"opponent's pos' : "<<(int)x<<"/"<<(int)y<<endl;);
        if (_player.getType())
            _gmap->setInitialh((int)x,(int)y);
        else
            _gmap->setInitials((int)x,(int)y);

        QString un;
        in>>un;
        Pos myPos, opPos;
        myPos=_player.Getcurpos();
        opPos=_player.Getoppos();

        if(!_player.getType()) {//hider
            _gmap->setSeeker(opPos);
            DEBUG_HS(cout<<"rh: "<<"You are the hider!"<<endl;);
            DEBUG_HS(cout<<"rh: "<<"Choose an initial position by clicking on a cell."<<endl;);
            _player.setopusername(un.toStdString());
        } else { //seeker
            _gmap->setSeeker(myPos);
            //_player.calculatevisible();
            DEBUG_HS(cout<<"rh: "<<"You are the seeker!"<<endl;);
            _player.setopusername(un.toStdString());
        }

        _player.addsmsg();
        _player.setf(); //ready to send his action

        //AG111202: removed this, replaced by readyReadTest() (see lower)
        //connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));

        //ag130508: fix map (ie allr ead)
        _gmap->setMapFixed();

        //send to server action taken and current position
        setInitialPos();
        //AG120903: make hider switch

        myPos=_player.Getcurpos();
        opPos=_player.Getoppos();
        //_player.setMap(_gmap);

        //cout << "INIT hidertype "<<_player.gethidertype()<<": ";//tmp
/*        switch (_player.gethidertype()) {
            case Player::OPPONENT_TYPE_HIDER_RANDOM: {
                //if (!_player.gethidertype()){ //if this is the random player
                //tcpSocket->flush();
                cout<<"random"<<endl;
                takenewaction();
                break;
            }
            case Player::OPPONENT_TYPE_HIDER_SMART: {

                cout<<"smart"<<endl;

                //tcpSocket->flush();
                _gmap->createPropDistPlanner();
                //get possible moves
                _player.getpossiblemoves();
                //score all pos moves
                _player.scoreneighb();
                int a = _player.chooseaction();
                //take action
                takenewaction(a);
                break;
            }
            case Player::OPPONENT_TYPE_HIDER_ACTION_LIST: {
                cout<<"action_list"<<endl;

                int a = _player.getNextActionFromList();
                if (a<0 || a>8) cout << "WARNING illegal action id from file: "<<a<<endl;
                takenewaction(a);
                break;
            }
            default:
                cout << "ERROR: unknown hider type"<<endl;
                exit(-1);
                break;
        }*/

        //ag130507: new action


        //cout << "FIRST ACTION: mypos="<<myPos.toString()<<",opPos="<<opPos.toString()<<",map size="<<_gmap->rowCount()<<"x"<<_gmap->colCount()<<flush;
        bool visib = _gmap->isVisible(myPos,opPos);
        //cout<<" visib="<<visib<<flush;
        int a = _autoHider->getNextAction(opPos,myPos,visib);
        //cout<<" action="<<a<<endl;
        takenewaction(a);

        //AG111202: test if ready for read..
        _stopped = false;
        readyReadTest();

    } else
        DEBUG_HS(cout<<"rh: "<<"not connected"<<endl;);
}

void GameConnectorHider::ondisconnect() {
    exit(0);
}


void GameConnectorHider::setInitialPos() {
// it randomly places the hider in one of the invisible for the seeker cells

    Pos initPos = _autoHider->getInitPos();

    DEBUG_AUTOHIDER(cout << "Initial: "<<initPos.toString()<<endl;);
/*   int ok=0;

    //cout<<"before while in initial pos"<<endl;
    if (_player.gethidertype() != Player::OPPONENT_TYPE_HIDER_ACTION_LIST) { //AG120906
        while(!ok) {

            if(_map==0) { //if it is the first map with no Invisible cells
                srand((unsigned)time(0));
                int n=rand()%_gmap->rowCount();
                t.row=n;
                int k=rand()%_gmap->colCount();
                t.col=k;
                DEBUG_HS(cout<< "rh: the initial pos of  hider is: ("<<t.row<<", "<<t.col<<")"<<endl;);
                ok=1;
            } else {
                DEBUG_HS(cout<<"calculate invisible: "<<flush;);
                _player.calculateInvisible();
                DEBUG_HS(cout<<"ok"<<endl;);

                srand((unsigned)time(0));
                int n=rand()%_player.get_n();
                t = _player.get_inv(n);
                ok=1;
            }


            if(t.row==_gmap->getBase().x && t.col==_gmap->getBase().y)
                ok=0;
            if ( (t.row==_player.Getoppos().row) && (t.col==_player.Getoppos().col) )
                ok=0;
        }
    } else {
        //AG120906: when using action list it is alread set by loader of map
        t = _player.getInitActionListPos();
    }
*/

    //_player.setcurpos(initPos.row,initPos.col);
    _player.setCurPos(initPos);
    _gmap->setHider(initPos);
    _gmap->setInitialh(initPos.row,initPos.col);

    //the hider sends his initial position

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out <<(quint16)0;
    out <<(quint16)initPos.row;
    out <<(quint16)initPos.col;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket->write(block);
    tcpSocket->flush();
    DEBUG_HS(cout<<"rh: "<<"hider set his initial pos and sent it to server"<<endl;);


    //_player.calculatevisible();

}


void GameConnectorHider::sendInfomsg() {
    //send the player name to the server.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out <<(quint16)0;
    out <<(quint16)0;//map irrelevant
    out <<(quint16)0;//opp irrelevant
    out <<(quint16)0; //type of automated player
    out << QString::fromStdString(_player.getusername());

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket->write(block);
    tcpSocket->flush();
    DEBUG_HS(cout<<"rh: "<<"player sent his name : "<<_player.getusername()<<endl;);

}

//AG111202: readReadyTest replaces the signal-slot solution (although that still should be the best solution)
void GameConnectorHider::readyReadTest() {

    while (!_stopped) {
        while (!tcpSocket->waitForReadyRead(1000) && !_stopped) {
            DEBUG_HS(cout << '.'<<flush;);
            sleep(1);
        }

        if (_stopped) return; //end

        DEBUG_HS(cout << "[ready read]"<<endl;);
        readyRead();
    }
}

void GameConnectorHider::readyRead() {
//msg format:  blocksize,  0, curpos(x,y), errorstring
//            or
//             blocksize, 1, oppos(x,y)



    DEBUG_HS(cout<<"rh:  Inside readyRead()!!!!!"<<endl;);
    quint16 blockSize=0;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
            // cout<<"less bytes in the array than expected1."<<endl;
            return;
        }
        in >> blockSize;
    }
    if (tcpSocket->bytesAvailable() < blockSize) {
        DEBUG_HS(cout<<"rh: "<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;);
        return;
    }


    quint16 q, s, x, y;
    QString str;
    in >> q;
    in >> s;
    if(s!=0) {

        DEBUG_HS(cout<<"rh: "<<(int)s<<"! GAME OVER !"<<(int)s<<endl;);
        tcpSocket->disconnectFromHost();
        exit(0);

    }
    if(q==0) { //validation int
        //that means the previous action was not done from the server

        exit(1);
    }

    in >> x;//read your opponent's current position
    in >> y;
    _player.setoppos((int)x, (int)y);
    Pos p;
    p.row=(int)x;
    p.col=(int)y;
    //  cout<<"Opponent of "<<_player.getType()<<" :    "<<p.x<<" / "<<p.y<<endl;
    if(!_player.getType()) //hider
        _gmap->setSeeker(p);
    else
        _gmap->setHider(p);

    _player.addsmsg();
    _player.setf(); //ready to send my action

    ///_player.calculatevisible();
/*
    //AG120903: make hider switch
    switch (_player.gethidertype()) {
        case Player::OPPONENT_TYPE_HIDER_RANDOM: {
            takenewaction();
            break;
        }
        case Player::OPPONENT_TYPE_HIDER_SMART: {
            //get possible moves
            _player.getpossiblemoves();
            //score all pos moves
            _player.scoreneighb();
            int a = _player.chooseaction();
            //take action
            takenewaction(a);
            break;
        }
        case Player::OPPONENT_TYPE_HIDER_ACTION_LIST: {
            int a = _player.getNextActionFromList();
            if (a<0 || a>8) cout << "WARNING illegal action id from file: "<<a<<endl;
            takenewaction(a);
            break;
        }
        default:
            DEBUG_HS(cout << "unknown hider type"<<endl;);
            exit(-1);
            break;
    }*/


    //ag130507: new action
    Pos myPos = _player.Getcurpos();
    bool visib = _gmap->isVisible(myPos,p);
    int a = _autoHider->getNextAction(p,myPos,visib);

    takenewaction(a);


    /*if(!_player.gethidertype())
        takenewaction();
    else {
        //get possible moves
        _player.getpossiblemoves();
        //score all pos moves
        _player.scoreneighb();
        int a = _player.chooseaction();
        //take action
        takenewaction(a);
    }*/
}

void GameConnectorHider::takenewaction(int a) {

    DEBUG_HS(cout << " takenewaction: "<<a<<endl;);
    /*int n;
    n=a;

    if(a==-1) {
        //get a random int from 0-8 to define next action
        srand((unsigned)time(0));
        n=rand()%9;
        DEBUG_HS(cout << "   new action: "<<n<<endl;);
    }

    while(! _player.MovePlayer(n)) {
        DEBUG_HS(cout << "  couldn't move with " << n;);
        n=rand()%9;
        DEBUG_HS(cout << " trying "<<n<<endl;);
    }*/

    if(! _player.MovePlayer(a)) {
        cout << "ERROR: COULD NOT MOVE with action "<<a<<endl;
    }

    if(sendaction())
        DEBUG_HS(cout<<"rh: action sent"<<endl;);

}


bool GameConnectorHider::sendaction() {
//msg format: action, current position(x,y)
    if(!_player.getf()) { //the action was not done
        DEBUG_HS(cout<<"rh: Action Aborted"<<endl;)
        return 0;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;

    out <<(quint16)_player.getlastaction();
    Pos p;
    p=_player.Getcurpos();
    out << (quint16)p.row;
    out << (quint16)p.col;

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket->write(block);//send to player
    tcpSocket->flush();
    _player.unsetf();
    _player.addpmsg();
    DEBUG_HS(cout<<"rh: action sent! a="<<_player.getlastaction()<<", r="<<p.row<< ",c="<<p.col<<endl;);

    return 1;

}
