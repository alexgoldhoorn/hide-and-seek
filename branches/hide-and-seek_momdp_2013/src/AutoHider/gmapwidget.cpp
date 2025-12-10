
#include "gmapwidget.h"


#include <QBool>
//#include <QtNetwork>

#include <iostream>
#include <math.h>
#include <assert.h>



using namespace std;

GMapWidget::~GMapWidget(){

}


void GMapWidget::init(int type, Player* pl){
    _gmap = pl->getPMap();
    _player = pl;

    _player->sethidertype(type);
    cout<<"rh: the hidertype at the automated player is "<<type<<endl;

    if (_player->gethidertype()){
        _player->setusername("SmartHider");
        cout<<"rh: AUTOMATED SmartHider: ";
    }
    else {
        _player->setusername("RandomHider");
        cout<<"rh: AUTOMATED RandomHider: ";
    }

    cout<<"rh: "<<_ip.toStdString()<<" : "<<_port<<" : "<<_player->getusername().toStdString()<<endl;

    tcpSocket = new QTcpSocket();
    tcpSocket->connectToHost(_ip,_port);
    if(tcpSocket->waitForConnected(3000))
    {
        cout<<"rh: connected!"<<endl;
        connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(ondisconnect()));

        //send the player name to the server.
        sendInfomsg();

        while (!tcpSocket->waitForReadyRead(6000))
        {

        }
        //read
        quint16 blockSize=0;
        QDataStream in(tcpSocket);
        in.setVersion(QDataStream::Qt_4_0);
        if (blockSize == 0) {
                if (tcpSocket->bytesAvailable() < (int)sizeof(quint16)){
                    cout<<"rh: less bytes in the array than expected1."<<endl;
                        return;
                }
                in >> blockSize;
        }
        if (tcpSocket->bytesAvailable() < blockSize){
            cout<<"rh: less bytes in the array than expected2. blocksize="<<blockSize<<endl;
                return;
        }



        quint16 q,i,x,y;
        in >> q;
        _map = (int)q;
        cout<<"rh: playing map no: "<<endl;
        in >> q;
        _player->setType((int)q);
        cout<<"rh: "<<_player->getType()<<"/"<<(int)q<<endl;
        in >> q;
        _gmap->setrows((int)q);
        cout<<"rh: "<<_gmap->rowCount()<<"/"<<(int)q<<endl;
        in >> i;
        _gmap->setcols((int)i);
        cout<<"rh: "<<_gmap->colCount()<<"/"<<(int)i<<endl;
        //_gmap->createMap(q,i);

        in >> i;
        int count;
       cout<<"rh: Obstacles : "<<(int)i<<endl;
        for(count=0; count<(int)i;count++)
        {
            in >> x;
            in >> y;
            _gmap->setWall(x,y);
           cout<<"rh: "<<"  "<<(int)x<<"/"<<(int)y<<endl;
        }
        in>>x;
        in>>y;
        _gmap->setbase((int)x,(int)y);
        cout<<"rh: "<<"Base : "<<_gmap->GetBase().x<<"/"<<_gmap->GetBase().y<<endl;
        in>>x;
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
        cout<<"rh: "<<"opponent's pos' : "<<(int)x<<"/"<<(int)y<<endl;
        if (_player->getType())
            _gmap->setInitialh((int)x,(int)y);
        else
            _gmap->setInitials((int)x,(int)y);

        QString un;
        in>>un;
        Pos p, op;
        p=_player->Getcurpos();
        op=_player->Getoppos();
        if(!_player->getType()) {//hider
            _gmap->setSeeker(op);
            cout<<"rh: "<<"You are the hider!"<<endl;
            cout<<"rh: "<<"Choose an initial position by clicking on a cell."<<endl;
            _player->setopusername(un);
        }
        else {//seeker
           _gmap->setSeeker(p);
           _player->calculatevisible();
           cout<<"rh: "<<"You are the seeker!"<<endl;
           _player->setopusername(un);
        }

        _player->addsmsg();
        _player->setf(); //ready to send his action


	//AG111202: removed this, replaced by readyReadTest() (see lower)
        //connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));


         //send to server action taken and current position
        if (!_player->gethidertype()){ //if this is the random player
            //tcpSocket->flush();
            setInitialPos();
            takenewaction();

         }
         else //if this is the smart player
        {
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

	//AG111202: test if ready for read..
	_stopped = false;
	readyReadTest();

    }
    else
        cout<<"rh: "<<"not connected"<<endl;
}

void GMapWidget::ondisconnect() {
    exit(0);
}


void GMapWidget::setInitialPos(){
// it randomly places the hider in one of the invisible for the seeker cells

	Pos t;
        int ok=0;

    cout<<"before while in initial pos"<<endl;
        while(!ok){

            if(_map==0)//if it is the first map with no Invisible cells
            {
                srand((unsigned)time(0));
                int n=rand()%_gmap->rowCount();
                t.x=n;
                int k=rand()%_gmap->colCount();
                t.y=k;
                cout<< "rh: the initial pos of  hider is: ("<<t.x<<", "<<t.y<<")"<<endl;
                ok=1;
            }
            else{
                 _player->calculateInvisible();
   
                 srand((unsigned)time(0));
                 int n=rand()%_player->get_n();
                 t = _player->get_inv(n);
                 ok=1;
            }


             if(t.x==_gmap->GetBase().x && t.y==_gmap->GetBase().y)
                ok=0;
             if ( (t.x==_player->Getoppos().x) && (t.y==_player->Getoppos().y) )
                ok=0;
         }
    _player->setcurpos(t.x,t.y);
    _gmap->setHider(t);
    _gmap->setInitialh(t.x,t.y);

    //the hider sends his initial position

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out <<(quint16)0;
    out <<(quint16)t.x;
    out <<(quint16)t.y;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket->write(block);
    tcpSocket->flush();
    cout<<"rh: "<<"hider set his initial pos and sent it to server"<<endl;

    _player->calculatevisible();

}


void GMapWidget::sendInfomsg(){
    //send the player name to the server.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out <<(quint16)0;
    out <<(quint16)0;//map irrelevant
    out <<(quint16)0;//opp irrelevant
    out <<(quint16)0; //type of automated player
    out << (QString)_player->getusername();

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket->write(block);
    tcpSocket->flush();
    cout<<"rh: "<<"player sent his name : "<<_player->getusername().toStdString()<<endl;

}

//AG111202: readReadyTest replaces the signal-slot solution (although that still should be the best solution)
void GMapWidget::readyReadTest() {    

    while (!_stopped) {
        while (!tcpSocket->waitForReadyRead(1000) && !_stopped)
        {
           cout << '.'<<flush;
           sleep(1);
        }
        
        if (_stopped) return; //end
        
        cout << "[ready read]"<<endl;
        readyRead();
    }
}

void GMapWidget::readyRead() {
//msg format:  blocksize,  0, curpos(x,y), errorstring
 //            or
 //             blocksize, 1, oppos(x,y)



    cout<<"rh:  Inside readyRead()!!!!!"<<endl;
    quint16 blockSize=0;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if (blockSize == 0) {
            if (tcpSocket->bytesAvailable() < (int)sizeof(quint16)){
               // cout<<"less bytes in the array than expected1."<<endl;
                    return;
            }
            in >> blockSize;
    }
    if (tcpSocket->bytesAvailable() < blockSize){
       cout<<"rh: "<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
        return;
    }


    quint16 q, s, x, y;
    QString str;
    in >> q;
    in >> s;
    if(s!=0){

        cout<<"rh: "<<(int)s<<"! GAME OVER !"<<(int)s<<endl;
        tcpSocket->disconnectFromHost();
        exit(0);

    }
    if(q==0)//validation int
    {//that means the previous action was not done from the server

        exit(1);
    }

    in >> x;//read your opponent's current position
    in >> y;
    _player->setoppos((int)x, (int)y);
    Pos p;
    p.x=(int)x;
    p.y=(int)y;
  //  cout<<"Opponent of "<<_player->getType()<<" :    "<<p.x<<" / "<<p.y<<endl;
    if(!_player->getType()) //hider
        _gmap->setSeeker(p);
    else
        _gmap->setHider(p);

    _player->addsmsg();
    _player->setf(); //ready to send my action

    _player->calculatevisible();

    if(!_player->gethidertype())
        takenewaction();
    else {
        //get possible moves
        _player->getpossiblemoves();
        //score all pos moves
        _player->scoreneighb();
        int a = _player->chooseaction();
        //take action
        takenewaction(a);
    }
}

void GMapWidget::takenewaction(int a){

    int n;
    n=a;

    if(a==-1){
    //get a random int from 0-8 to define next action
    srand((unsigned)time(0));
    n=rand()%9;
    }

    while(! _player->MovePlayer(n))
        n=rand()%9;

    if(sendaction())
       cout<<"rh: action sent"<<endl;

}


bool GMapWidget::sendaction() {
//msg format: action, current position(x,y)
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
    out << (quint16)p.x;
    out << (quint16)p.y;

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket->write(block);//send to player
    tcpSocket->flush();
    _player->unsetf();
    _player->addpmsg();
    cout<<"rh: action sent! "<<_player->getlastaction()<<p.x<<p.y<<endl;

    return 1;

}
