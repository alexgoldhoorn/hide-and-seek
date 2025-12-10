/*

Hide & Seek client
ag, Welcome to the Hide & Seek game!

connected to server!
Segmentation fault

*/
#include "gmapwidget.h"

#include <QApplication>
#include <QBool>
#include <QPainter>
#include <QKeyEvent>
#include <QtGui>
//#include <QtNetwork>

#include <iostream>
#include <math.h>
#include <assert.h>
#include "popup.h"

#include "hsglobaldata.h"


using namespace std;


GMapWidget::GMapWidget(QWidget *parent) : QWidget(parent) {
    _onlyShowMap = false;
    Dialog* d = new Dialog();
    _dialog = d;

    _coordZeroBased = false;
    _showCoords = 0;
    _player = NULL;

    // _dialog->setWindowTitle(_title);
    QTimer::singleShot(0, this, SLOT(showDialog()));
    init(new Player());    
}

GMapWidget::GMapWidget(GMap *gmap, QWidget *parent) : QWidget(parent) {
    _onlyShowMap = true;
    _dialog = NULL;
    _gmap = gmap;
    _coordZeroBased = false;
    _showCoords = 0;
    _player = NULL;

    gmap->printMap();

    float w = 1.0f * _gmap->width() * height() / _gmap->height();
    resize(round(w), height());    
}

GMapWidget::~GMapWidget() {
    //delete _gmap, _polygon;
    if (_dialog!=NULL) delete _dialog;
}
void GMapWidget::showDialog() {

    _port=0;
    _ip = "";
    _player->setusername("");
    _map= -1;
    _opp=-1;
    _type=0;


    if (_dialog->exec()) {
        _ip= _dialog->getip();
        _port = _dialog->getport();
        _player->setusername(_dialog->getusername().toStdString());
        _map=_dialog->getmap();
        _opp = _dialog->getopp();
        _type =_dialog->getType();
        _hidinitpos =0; //the initial position of hider is not chosen yet!

        _player->initplayer();
        //_gmap->InitMap();

        cout<<_player->getusername()<<", Welcome to the Hide & Seek game!"<<endl<<endl;
        if(!_port|| _ip.isEmpty() || _player->getusername().empty()|| _map==-1 || _opp==-1) {
            //if one of the fields of the popup window is missing

            //_dialog->setTitle("complete all the fields");
            QTimer::singleShot(0, this, SLOT(showDialog()));
            return;
        }
        if(_opp==3 && _type==1) { //if the user selected to play with SeekroBot but chose to be the seeker is not acceptable cause the SeekroBot plays as seeker
            //_dialog->setTitle("to play with the hider you have to be the seeker");
            QTimer::singleShot(0, this, SLOT(showDialog()));
            return;
        }
        if( (_opp==1 || _opp==2) && _type==0) { //if the user selected to play with one of the automated players but chose to be the hider is not acceptable cause the automated players at this point are for the role of the hider
            //_dialog->setTitle("to play with the hider you have to be the seeker");
            QTimer::singleShot(0, this, SLOT(showDialog()));
            return;
        }


        tcpSocket = new QTcpSocket(this);
        tcpSocket->connectToHost(_ip,_port);
        if(tcpSocket->waitForConnected(3000)) {
            cout<<"connected to server!"<<endl;
            sendinfo(); //send the username of the player along with the choises of the popup window.

            cout << "Waiting for server "<<flush;

            while (!tcpSocket->waitForReadyRead(6000)) {
                //cout << "."<<flush;
            }

            cout << endl << "Reading data.."<<endl;
            //read
            quint16 blockSize=0;

            QDataStream in(tcpSocket);
            in.setVersion(QDataStream::Qt_4_0);

            //AGc: OFCOURSE IT SHOWS LESS THANE XP -> BLOCKSIZE=0!!!!!

            //AGc: put (back)?
            in >> blockSize;

            if (blockSize == 0) {
                if (tcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
                    cout<<"less bytes in the array than expected1."<<endl;
                    return;
                }
                in >> blockSize;
            }
            if (tcpSocket->bytesAvailable() < blockSize) {
                cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
                return;
            }


            cout << ".."<<endl;

            quint16 q,i,x,y;
            in >> q;
            _map = (int)q;
            in >> q;
            _player->setType((int)q);
            _type = ((int)q > 0) ? true:false;
            cout<<_type<<"/"<<(int)q<<endl;
            in >> q;
            //_gmap->setrows((int)q);
            cout<<_gmap->rowCount()<<"/"<<(int)q<<endl;
            in >> i;
            //_gmap->setcols((int)i);
            cout<<_gmap->colCount()<<"/"<<(int)i<<endl;
            _gmap->createMap(q,i);

            in >> i;
            int count;
            //cout<<"Obstacles : "<<(int)i<<endl;
            for(count=0; count<(int)i; count++) {
                in >> x;
                in >> y;
                cout<<(int)x<<"/"<<(int)y<<endl;
                //_gmap->setWall(x,y);
                _gmap->addObstacle(x,y);
            }
            in>>x;
            in>>y;
            _gmap->setbase((int)x,(int)y);
            cout<<"Base : "<<_gmap->getBase().row<<"/"<<_gmap->getBase().col<<endl;

            _gmap->setMapFixed();//

            in>>x;
            in>>y;
            _player->setcurpos(x,y);
             cout<<"my pos : "<<(int)x<<"/"<<(int)y<<endl;
            if (_player->getType())
                _gmap->setInitials((int)x,(int)y);
            else
                _gmap->setInitialh((int)x,(int)y);
            in>>x;
            in>>y;
            _player->setoppos(x,y);
            cout<<"opponent's pos' : "<<(int)x<<"/"<<(int)y<<endl;
            if (!_player->getType())
                _gmap->setInitials((int)x,(int)y);
            else
                _gmap->setInitialh((int)x,(int)y);
            QString un;
            in>>un;
            Pos p, op;
            p=_player->Getcurpos();
            op=_player->Getoppos();
            if(!_player->getType()) {//hider
                this->setWindowTitle("You are the Hider. Click on the cell you want to place your self.");
                //_gmap->setHider(p);
                _gmap->setSeeker(op);
                cout<<"You are the hider!"<<endl;
                cout<<"Choose an initial position by clicking on a cell."<<endl;
                _player->setopusername(un.toStdString());
            } else { //seeker

                //_gmap->setHider(op);
                _gmap->setSeeker(p);
                ///_player->calculatevisible();
                cout<<"You are the seeker!"<<endl;
                _player->setopusername(un.toStdString());
                QString t;
                QString numa;
                numa = QString::number(_player->getnuma());
                t=numa;
                /*t.append("/");
                QString max = QString::number(MAXACTIONS);
                t.append(max);*/
                t.append("\tYou are the Seeker!\t");
                t.append("opponent: ");
                t.append(QString::fromStdString(_player->getopusername()));
                this->setWindowTitle(t);

            }
            _player->addsmsg();
            _player->unsetf(); //ready to send his action
            // cout<<endl<<"Base : "<<_gmap->GetBase().x<<"/"<<_gmap->GetBase().y<<endl<<endl;
            // _gmap->setbase();

            cout<<"You are playing against "<<_player->getopusername()<<endl;
            connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));

            if (!tcpSocket->isOpen())
                cout<<"error in socket connection"<<endl;

            _gmap->printMap();

            //send to server action taken and current position
            repaintwidget();


        }
    } else
        this->close();
}

int GMapWidget::InitPos(QPoint h) {


    Pos p;
    p.row=h.ry();
    p.col=h.rx();

    if(_gmap->getItem(p.row,p.col)==1) { //if there is an obstacle there
        cout<<"You are not allowed to place yourself on the wall! Try again!"<<endl;
        return 0;

    }
    if(p.row==_gmap->getBase().row && p.col==_gmap->getBase().col) {
        cout<<"You are not allowed to place yourself on the base! Try again!"<<endl;
        return 0;

    }
    if ( (p.row==_player->Getoppos().row) && (p.col==_player->Getoppos().col) ) {
        cout<<"You are not allowed to place yourself on the Seeker! you 'll loose!"<<endl;
        return 0;
    }
    _player->setcurpos(h.ry(),h.rx());
    _gmap->setHider(p);
    _gmap->setInitialh(p.row,p.col);
    //set hider active
    _player->setf();
    ///_player->calculatevisible();
    repaintwidget();
    initialposh();
    return 1;
}

void GMapWidget::init(Player* pl) {
    _gmap = pl->getMap(); //was getPMap
    _player = pl;

    //set height of window, to make the rectangles square
    //cout<<"set window height!";-g++
    float w = 1.0f * _gmap->width() * height() / _gmap->height();
    resize(round(w), height());
}






void GMapWidget::readyRead() {
//msg format:  blocksize,  0, curpos(x,y), errorstring
//            or
//             blocksize, 1, oppos(x,y)-g++



    //cout<<"Inside readyRead()"<<endl;
    quint16 blockSize=0;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
            cout<<"less bytes in the array than expected1."<<endl;
            return;
        }
        in >> blockSize;
    }
    if (tcpSocket->bytesAvailable() < blockSize) {
        cout<<"less bytes in the array than expected2. blocksize="<<blockSize<<endl;
        return;
    }

    //cout<<"for type: "<<_player->getType()<<"void the initial pos of hider is "<<(int)_hidinitpos<<endl;
    if( (_player->getType()) && (!_hidinitpos)) { //if its the seeker on the first msg then read the initial pos of the hider
        quint16 x, y;
        Pos p;

        in >> x;//hider's initial position
        p.row=(int)x;
        in >> y;
        p.col=(int)y;
        //cout<<"the seeker got the initial pos o f the hider: "<<p.x<<p.y<<endl;
        _player->setoppos(p.row,p.col);
        _gmap->setHider(p);
        _gmap->setInitialh(p.row,p.col);
        _hidinitpos=1;
        _player->setf();
        repaintwidget();
        return;

    }


    quint16 q, s, x, y;
    QString str;
    in >> q; //validation byte
    in >> s; //status byte
    if(s!=0) {

        cout<<"! GAME OVER !"<<endl;
        tcpSocket->disconnect();

        // disconnect(this, Key)
        switch((int)s) {
            case 1: {
                if(_player->getType()) {
                    //cout<<"SEEKER YOU WON!"<<endl<<endl;
                    _player->setwin(1);
                    str="SEEKER, YOU WON!";
                } else {
                    // cout<<"HIDER YOU LOST!"<<endl<<endl;
                    _player->setwin(-1);
                    str = "HIDER, YOU LOST!";
                }
            }
            break;//seeker
            case 2: {
                if(!_player->getType()) {
                    //cout<<"HIDER YOU WON!"<<endl<<endl;
                    _player->setwin(1);
                    str="HIDER, YOU WON!";
                } else {
                    //cout<<"SEEKER YOU LOST!"<<endl<<endl;
                    _player->setwin(-1);
                    str = "SEEKER, YOU LOST!";
                }
            }
            break;//hider
            case 3:
                _player->setwin(0);
                str = "IT´S A TIE!";
                break;//tie
            default:
                cout<<"unknown status"<<endl;
        }
        this->setWindowTitle(str);

    }
    if(q==0) { //validation int
        //that means the previous action was not done from the server

        in >> x;//read your current position
        in >> y;
        _player->setcurpos((int)x, (int)y);
        in >> str;
        //need to put the map as before update action lists.. etc
        cout<<"server says that your move was invalid: "<<(int)s<<(int)q<<(int)x<<(int)y<<str.toStdString()<<endl;
        _player->addsmsg();
        _player->setf();
        if(!_player->getType())
            _gmap->setHider(_player->Getcurpos());
        else
            _gmap->setSeeker(_player->Getcurpos());
        repaintwidget();
        return;
    }

    in >> x;//read your opponent's current position
    in >> y;
    _player->setoppos((int)x, (int)y);
    Pos p;
    p.row=(int)x;
    p.col=(int)y;
    //cout<<"Opponent of "<<_player->getType()<<" :    "<<p.x<<" / "<<p.y<<endl;
    if(!_player->getType()) //hider
        _gmap->setSeeker(p);
    else
        _gmap->setHider(p);

    _player->addsmsg();
    _player->setf(); //ready to send my action

    ///_player->calculatevisible();
    repaintwidget();

    if(_player->getwin()!=2) { //if the game is over
        if(tcpSocket->isOpen())
            tcpSocket->disconnectFromHost();



        QString title;
        if(_player->getwin()==1)
            title = "YOU WIN";
        else if(_player->getwin()==-1)
            title = "YOU LOST";
        else if(_player->getwin()==0)
            title = "IT´S A TIE";


        //this->setWindowTitle(title);
        delete _dialog;
        Dialog* d  = new Dialog(_ip, QString::fromStdString(_player->getusername()),_port, _map, _opp, _type);
        _dialog = d;
        _dialog->setWindowTitle(title);
        QTimer::singleShot(0, this, SLOT(showDialog()));
    }
}


void GMapWidget::replay() {
    // _gmap->setHider(_gmap->getInitialh());
    // _gmap->setSeeker(_gmap->getInitials());
    _hidinitpos=0;
    _player->initplayer();
    ///_gmap->InitMap();
    repaintwidget();
}

void GMapWidget::initialposh() {

//the hider sends his initial position
    Pos p = _player->Getcurpos();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out <<(quint16)0;
    out <<(quint16)p.row;
    out <<(quint16)p.col;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket->write(block);
    tcpSocket->flush();
//cout<<"Your initial position is: "<<_player->getusername().toStdString()<<endl;
}


void GMapWidget::sendinfo() {

//send the player name to the server.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out <<(quint16)0;
    out <<(quint16)_map;

    //ag120903: send file name of actions TEMP
    if (!_actionFile.isEmpty()) _opp = HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST;

    out <<(quint16)_opp;
    //ag120903: send file name of actions TEMP
    if (!_actionFile.isEmpty()) { out << _actionFile; cout << "SENDING: opp="<<_opp<<"; actionFile="<<_actionFile.toStdString()<<endl;}

    out <<(quint16)_type;
    out << QString::fromStdString( _player->getusername() );

    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket->write(block);
    tcpSocket->flush();
    cout<<"player sent his name : "<<_player->getusername()<<endl;
}


bool GMapWidget::sendaction() {
//msg format: action, current position(x,y)
    if(!_player->getf()) { //the action was not done
        cout<<"Action Aborted"<<endl;
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
    tcpSocket->write(block);//send to player
    tcpSocket->flush();
    _player->unsetf();
    _player->addpmsg();
    // cout<<"action sent! "<<_player->getlastaction()<<p.x<<p.y<<endl;
    return 1;

}


void GMapWidget::drawMap(QPainter& painter) {
    GMap* tmap;
    tmap=_gmap;

    bool f = false;    
    Pos pos;
    if (_player!=NULL) {
        f = _player->getf();
        pos = _player->Getcurpos();
    }

    //draw complete map
    for (int r=0; r<_gmap->rowCount(); r++) {
        for (int c=0; c<_gmap->colCount(); c++) {
            bool visib = true; //ag130508: update visiblitiy
            if (_player!=NULL) visib = tmap->isVisible(pos.row,pos.col,r,c);

            drawCell(painter, c, r, tmap->getItem(r,c), visib, f);
        }
    }

    painter.setPen(Qt::black);
    painter.drawRect(1,1,width(),height());
}






void GMapWidget::drawCell(QPainter& painter, int x, int y, int c, int v, bool f) {
    QRect r = GMapToWidgetRect(x, y);
    float s, t = 5.0;
    QRect rp = GMapToWidgetRect(x, y, t);
    QRect rs = GMapToWidgetRect(x, y, t, s);



    if(v==0) {
        painter.setPen(Qt::black);
        painter.setBrush(Qt::lightGray);
    } else {
        switch(c) {
            case GMap::GMAP_FREE_SPACE:
                painter.setPen(Qt::black);
                painter.setBrush(Qt::NoBrush);
                break;
            case GMap::GMAP_OBSTACLE:
                painter.setPen(Qt::black);
                painter.setBrush(Qt::black); //darkGray);
                //painter.fillRect(r,Qt::darkGray);
                break;
            case GMap::GMAP_BASE:
                painter.setPen(Qt::black);
                painter.setBrush(Qt::darkGray); //green);
                //painter.fillRect(r,Qt::green);
                break;
            case GMap::GMAP_SEEKER:
                painter.setPen(Qt::darkBlue);
                painter.setBrush(Qt::blue);
                painter.drawEllipse(r); //, Qt::green);
                if(_type) { //if you are the seeker
                    if(!f) //active
                        painter.fillRect(rp,Qt::darkRed);
                    else
                        painter.fillRect(rp,Qt::darkGreen);
                }

                painter.setPen(Qt::black);
                painter.setBrush(Qt::NoBrush);
                break;
            case GMap::GMAP_HIDER:
                painter.setPen(Qt::darkRed);
                painter.setBrush(Qt::red);
                painter.drawEllipse(r); //, Qt::green);
                //painter.setPen(Qt::darkGreen);
                //painter.setBrush(Qt::green);
                //painter.drawEllipse(rs);

                if(!_type) { //if you are the hider
                    if(!f) //active
                        painter.fillRect(rp,Qt::darkRed);
                    else
                        painter.fillRect(rp,Qt::darkGreen);
                }
                painter.setPen(Qt::black);
                painter.setBrush(Qt::NoBrush);
                break;
            default:
                cout << "Unknown map item: " << c << " at (" << x << "," << y <<")"<<endl;
                assert(false);
        }
    }   // if (!tcpSocket->isOpen())
    //cout<<"error in socket connection"<<endl;

//send to server action taken and current position

    //show square as grid
    painter.drawRect(r);

    if (_showCoords>0) {
        QString coordS;



        //AG120904: switch for different coords/index
        switch (_showCoords) {
        case SHOW_COORD_XY:
            if (!_coordZeroBased) {
                ++x;
                ++y;
            }
            coordS = "("+QString::number(x)+","+QString::number(y)+")";
            break;
        case SHOW_COORD_RC:
            if (!_coordZeroBased) {
                ++x;
                ++y;
            }
            coordS = "r"+QString::number(y)+"c"+QString::number(x);
            break;
        case SHOW_COORD_I: {
            int i = _gmap->getIndexFromCoord(y,x) + (_coordZeroBased?0:1);
            coordS = "i"+QString::number(i);
            break;
        }
        default:
            break;
        }

        painter.drawText(r,Qt::AlignCenter,coordS);
    }

}


void GMapWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    drawMap(painter);

}


QRect GMapWidget::GMapToWidgetRect(QPoint& p) {
    return GMapToWidgetRect(p.x(), p.y());
}

QRect GMapWidget::GMapToWidgetRect(int px, int py) {
    //calc the cell center of the discrete boxes
    int dx = round(1.0f * width()  /this->getGMap()->colCount());
    int dy = round(1.0f * height() /this->getGMap()->rowCount());

    //calculate the (x,y) on the widget using the row and col
    int x = round(1.0f * px * width() / (this->getGMap()->colCount() )) - 1;
    int y = round(1.0f * py * height() / (this->getGMap()->rowCount() )) - 1; /*height() -*/

    return QRect(x,y,dx,dy);
}


QRect GMapWidget::GMapToWidgetRect(int px, int py, float f) {
    //calc the cell center of the discrete boxes
    int dx1 = round(1.0f * width()  /this->getGMap()->colCount());
    int dy1 = round(1.0f * height() /this->getGMap()->rowCount());
    int dx=dx1/f;
    int dy=dy1/f;

    //calculate the (x,y) on the widget using the row and col
    int x = round(1.0f * px * width() / (this->getGMap()->colCount() )) - 1;
    int y = round(1.0f * py * height() / (this->getGMap()->rowCount() )) - 1; /*height() -*/

    x = x+ dx1-dx;
    y = y+ dy1 -dy;

    return QRect(x,y,dx,dy);
}

QRect GMapWidget::GMapToWidgetRect(int px, int py, float f, float s) {
    //calc the cell center of the discrete boxes
    int dx1 = round(1.0f * width()  /this->getGMap()->colCount());
    int dy1 = round(1.0f * height() /this->getGMap()->rowCount());
    int dx=dx1/f;
    int dy=dy1/f;

    //calculate the (x,y) on the widget using the row and col
    int x = round(1.0f * px * width() / (this->getGMap()->colCount() )) - 1;
    int y = round(1.0f * py * height() / (this->getGMap()->rowCount() )) - 1; /*height() -*/

    x = x+ (dx1/2)-(dx/2);
    y = y+ dy1 -dy;

    return QRect(x,y,dx,dy);
}

void GMapWidget::keyPressEvent ( QKeyEvent * event ) {
    //keys always available:
    switch(event->key()) {
        case Qt::Key_P:
        case Qt::Key_F4:
            saveScreen();
            return;
        case Qt::Key_Escape:
            exit(0);
            return;
        case Qt::Key_C:
        case Qt::Key_F2:
            _showCoords++;
            if (_showCoords>SHOW_COORD_I) _showCoords=0;
            repaint();
            return;
        case Qt::Key_F3:
            _coordZeroBased = !_coordZeroBased;
            repaint();
            return;
        case Qt::Key_F1:
            showHelp();
            return;
        case Qt::Key_Plus: {
            int w = (int)(width()*1.10);
            resize(w,w);
            return;
        }
        case Qt::Key_Minus: {
            int w = (int)(width()*0.9);
            resize(w,w);
            return;
        }
    }
    //}

    if (!_onlyShowMap) { //ag120509:
        if(!_hidinitpos) { //if the initial position was not set yet from the hider then dont make a move yet.
            if(_player->getType()) {
                cout<<"Please wait until the hider is ready"<<endl;
            } else {
                cout<<"Please click on the cell of the map where you want to place your self"<<endl;
            }
            return;
        }

        Player* pl = this->getPlayer();
        if(_player->getwin()!=2)
            return;

        if (!pl->getf()) {
            cout<<"Wait a sec before you make your move"<<endl;
            return;
        }
        if(pl->getsmsg() <= pl->getpmsg()) {
            cout<<"Wait until your opponent make his move!"<<endl;
            return;
        }


        bool b=0;
        //AG120607: updated keys -> use numeric keys
        switch(event->key()) {
            case Qt::Key_Y:
            case Qt::Key_Up:
            case Qt::Key_8:
                // move north
                b=pl->MovePlayer(1);            
                break;
            case Qt::Key_U:
            case Qt::Key_PageUp:
            case Qt::Key_9:
                // move north-east
                b=pl->MovePlayer(2);
                break;
            case Qt::Key_H:
            case Qt::Key_Right:
            case Qt::Key_6:
                // move east
                b=pl->MovePlayer(3);
                break;
            case Qt::Key_N:
            case Qt::Key_PageDown:
            case Qt::Key_3:
                // move east-south
                b=pl->MovePlayer(4);
                break;
            case Qt::Key_B:
            case Qt::Key_Down:
            case Qt::Key_2:
                // move south
                b=pl->MovePlayer(5);
                break;
            case Qt::Key_V:
            case Qt::Key_End:
            case Qt::Key_1:
                // move south-west
                b=pl->MovePlayer(6);
                break;
            case Qt::Key_G:
            case Qt::Key_Left:
            case Qt::Key_4:
                // move west
                b=pl->MovePlayer(7);
                break;
            case Qt::Key_T:
            case Qt::Key_Home:
            case Qt::Key_7:
                // move west-north
                b=pl->MovePlayer(8);
                break;
            case Qt::Key_Space:
            case Qt::Key_Insert:
            case Qt::Key_0:
                b=pl->MovePlayer(0);
                break; //don´t move
            case Qt::Key_Escape:
                tcpSocket->disconnectFromHost();
                exit(0);
                break; //popup window to reassure the quiting and quit...?
            /*default: {
                cout << " (unknown command)"<<endl;
                return;
            }*/
        }
        if(b) { //the action was made on the client
            if (!sendaction()) //send action to the server
                cout<<"the action was NOT sent"<<endl;
            if(_type) {
                QString t;
                QString numa;
                numa = QString::number(_player->getnuma());
                t=numa;
                /*t.append("/");
                QString max = QString::number(MAXACTIONS);
                t.append(max);*/
                t.append("\tYou are the Seeker!\t ");
                t.append("opponent: ");
                t.append(QString::fromStdString(_player->getopusername()));
                this->setWindowTitle(t);
            } else  {
                QString t;
                QString numa;
                numa = QString::number(_player->getnuma());
                t=numa;
                /*t.append("/");
                QString max = QString::number(MAXACTIONS);
                t.append(max);*/
                t.append("\t You are the Hider! \t");
                t.append("opponent: ");
                t.append(QString::fromStdString(_player->getopusername()));
                this->setWindowTitle(t);
            }
            //repaintwidget();
            repaint();
        }
    }// else {



}


void GMapWidget::showHelp() {
    //TODO: visual
    cout    << " Hide & Seek client - Help"<<endl
            << "--------------------------"<<endl
            << "Y / Up / 8      - go up/north"<<endl
            << "U / PgUp / 9    - go up-right/north-east"<<endl
            << "H / Right / 6   - go right/east"<<endl
            << "N / PgDown / 3  - go down-right/south-east"<<endl
            << "B / Down / 2    - go down/south"<<endl
            << "V / End / 1     - go down-left/south-west"<<endl
            << "G / Left / 4    - go left/west"<<endl
            << "T / Home / 7    - go up-left/north-west"<<endl
            << "space / ins / 0 - halt/don't move"<<endl
            << "Y / Up / 8      - go up/north"<<endl
            <<endl
            << "F1              - this help"<<endl
            << "F2 / C          - show/hide coordinates (x,y) or row-col or index"<<endl
            << "F3              - toggle between 0 or 1-based coordinates"<<endl
            << "F4 / P          - print screen to hideandseek.png"<<endl
            << "+ / -           - increase/decrease window by 10%"<<endl
            << "Esc             - exit"<<endl<<endl;

}


QPoint GMapWidget::WidgetToGMapPoint(QPoint& p) {
    //calc the cell center of the discrete boxes
    int dx = round(1.0f * width()  / _gmap->colCount());
    int dy = round(1.0f * height() / _gmap->rowCount());

    //calculate the (x,y) on the widget using the row and col
    int x = round(p.x()/dx);
    int y = round(p.y()/dy);

    return QPoint(x,y);
}


//ag 111019
void GMapWidget::mousePressEvent(QMouseEvent *e) {
    if(_type || _onlyShowMap) { //if is the seeker don't grab the clicking
        return;
    }
    if(_hidinitpos) { //if the initial position is set don't grab the clicking
        return;
    }

    QPoint p = e->pos();
    p = WidgetToGMapPoint(p);
    //qDebug() << "(" << (p.x()+1) << "," << (p.y()+1) << ")";
    //cout<<"The initial position of the hider is: ("<<p.ry()<<", "<<p.rx()<<")"<<endl;
    //set the initial position....? and send it...
    if(!InitPos(p))
        return;
    _hidinitpos=1;
    _player->setf();

    QString t;
    QString numa;
    numa = QString::number(_player->getnuma());
    t=numa;
    /*t.append("/");
    QString max = QString::number(MAXACTIONS);
    t.append(max);*/
    t.append("\t You are the Hider! \t");
    t.append("opponent: ");
    t.append(QString::fromStdString(_player->getopusername()));
    this->setWindowTitle(t);


}



void GMapWidget::saveScreen() {
    cout << "Saving screen :" << flush;
    QImage img(size(),QImage::Format_ARGB32);
    //QPicture img();
    QPainter painter(&img);
    render(&painter);
    img.save("hideandseek.png","PNG");
    cout << "ok" << endl;
}
