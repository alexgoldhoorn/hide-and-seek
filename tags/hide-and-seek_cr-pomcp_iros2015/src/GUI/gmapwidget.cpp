/*

Hide & Seek client

*/
#include "GUI/gmapwidget.h"

#include <QApplication>

#include <QPainter>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QInputDialog>

#include <iostream>
#include <math.h>
#include <assert.h>
#include <sstream>

#include "GUI/popup.h"
#include "GUI/setparamsdialog.h"

#include "hsglobaldata.h"
#include "hsconfig.h"
#include "abstractautoplayer.h"
#include "seekerhsparams.h"
#include "Utils/generic.h"

#ifndef HSCLIENTMAINWINDOW_H
#include "GUI/hsclientmainwindow.h"
#endif



//AG140527: TODO: cleanup and use a SeekerHSParams* variable

using namespace std;


GMapWidget::GMapWidget(bool useMouseObs, bool allowClickNextPos, QString metaInfo, QString comments, QWidget *parent) : QWidget(parent) {
    _onlyShowMap = false;

    _coordZeroBased = true;
    _showCoords = 0;
    _player = NULL;
    _gmap = NULL;
    _useMouseObs = useMouseObs;
    _allowClickNextPos = allowClickNextPos;
    _mouseObs.set(-2,-2); // row = _mouseObs.col = -2;
    _showAllPlayers = false;
    _connector = NULL;
    _wstate = WState::normal;
    _debugMode = false;
    _debugAutoPlayer = NULL;
    _comments = comments;
    _metaInfo = metaInfo;
    _showOppVisib = false;
    _mainWindow = NULL;

    setTitle("Hide-and-Seek Client");

    QTimer::singleShot(0, this, SLOT(showDialogForNewGame()));

    resetWindowToMapSize();
}

GMapWidget::GMapWidget(GMap *gmap, QWidget *parent) : QWidget(parent) {
    _onlyShowMap = true;
    _gmap = gmap;
    _coordZeroBased = true;
    _showCoords = 0;
    _player = NULL;
    _showAllPlayers = false;
    _connector = NULL;
    _wstate = WState::normal;
    _debugMode = false;
    _debugAutoPlayer = NULL;
    _showOppVisib = false;
    _mainWindow = NULL;

    setTitle("Hide-and-Seek Client");

    if (gmap->getParams()==NULL) {
        cout <<"Setting new params."<<endl;
        //AG NOTE: should be stored and deleted.. but 'only' for debug
        _gmap->setParams(new SeekerHSParams);
        _gmap->getParams()->useContinuousPos = true;
    }

    //move out of left top corner
    move(25,25);
    //set map size to 'default'
    resetWindowToMapSize();
}

GMapWidget::~GMapWidget() {
    if (_connector!=NULL) delete _connector;
    if (_debugAutoPlayer!=NULL) delete _debugAutoPlayer;
}

void GMapWidget::showDialogForNewGame() {    
    //init vars
    string username = "test";
    if (_player!=NULL) {
        username = _player->getUsername();
    }

    /*QString ip = QString::fromStdString(HSGlobalData::DEFAULT_SERVER_IP);
    int port = HSGlobalData::DEFAULT_SERVER_PORT;
    int mapID = 0;
    bool isSeeker = true;
    int opp = 0;

    if (_connector!=NULL) {
        SeekerHSParams* params = _connector->getParams();

        ip = QString::fromStdString(params->serverIP);
        port = params->serverPort;
        username = _connector->getPlayer()->getUsername();
        mapID = params->mapID;
        isSeeker = params->isSeeker;
        opp = params->opponentType;
    }*/

    SeekerHSParams* params = NULL;
    if (_connector!=NULL) {
        params = _connector->getParams();
    }
    if (params==NULL){
        params = new SeekerHSParams();
    }
    params->userName = username;


    //create dialog
    Dialog dialog(params,this); //(ip, QString::fromStdString(username), port, mapID, opp, isSeeker, this);
    //Dialog dialog(QString::fromStdString(params->serverIP), QString::fromStdString(params->userName), params->serverPort,
    //              params->mapID, params->opponentType, params->isSeeker, this);
    //Dialog dialog(this);


    if (dialog.exec()) {
        //get info from dialog
        dialog.getParams();

        /*ip = dialog.getIP();
        port = dialog.getPort();
        username = dialog.getUsername().toStdString();
        mapID = dialog.getMapIndex();
        isSeeker = dialog.isSeeker();
        opp = dialog.getOpponentType();*/

        //first close other connector        
        if (_connector!=NULL) {            
            //disconnect and delete
            /*disconnect(_connector,SIGNAL(serverDisconnected()),this,SLOT(handleServerDisconnected()));
            disconnect(_connector,SIGNAL(serverParamsRead()),this,SLOT(handleServerParamsRead()));
            disconnect(_connector,SIGNAL(serverUpdateReceived(int,int,int)),this,SLOT(handleServerUpdateReceived(int,int,int)));*/

            //AG140107: old connector should be deleted! But this causes that the new connection crashes,
            //although it seems that only Player and GMap are deleted. TODO: check why this happens
            cout << "WARNING: old connector is NOT deleted!"<<endl;
            //delete _connector;
        }

        GMap* mapToSend = NULL;
        if (params->mapFile.length()>0) {
            mapToSend = new GMap(params->mapFile, params);
            cout <<"Map sent:"<<endl;
            mapToSend->printMap();
        }

        //create connection object
        _connector = new GameConnectorClient(params, _metaInfo, _comments, NULL, mapToSend);

        connect(_connector,SIGNAL(serverDisconnected()),this,SLOT(handleServerDisconnected()));
        connect(_connector,SIGNAL(serverParamsRead()),this,SLOT(handleServerParamsRead()));
        connect(_connector,SIGNAL(serverUpdateReceived(int,int,int)),this,SLOT(handleServerUpdateReceived(int,int,int)));

        //connect
        bool ok = _connector->connectToServer();

        if (!ok) {
            cout << "Error: could not connect to the server, retry please."<<endl;
            showDialogForNewGame();
        } else {
            setWindowTitle("Hide & Seek client - Waiting for server...");
            _player = _connector->getPlayer();
            _gmap = _connector->getGMap(); //note: map is loaded when read from server event received, then set it again!
        }
    } else {
        _onlyShowMap = true;
    }
}


bool GMapWidget::initPos(Pos initPos) {
    assert(_connector!=NULL);
    Pos pos(initPos);

    if(_gmap->isObstacle(pos)) {
        cout<<"You are not allowed to place yourself on the wall! Try again!"<<endl;
        QMessageBox::warning(this,"Warning","You are not allowed to place yourself on the wall.");
        return false;
    } else if (!_connector->isSeeker() && pos.equals(_gmap->getBase())) {
        cout<<"You are not allowed to place yourself on the base as a hider! Try again!"<<endl;
        QMessageBox::warning(this,"Warning","You are not allowed to place yourself on the base as a hider.");
        return false;
    }

    /*if (_connector->getParams()->useContinuousPos) {
        pos.add(0.5,0.5); //centre of cell
    }*/

    if (!_connector->getParams()->useContinuousPos) {
        pos.convertValuesToInt();
    }


    _player->setCurPos(pos);
    _connector->sendInitPos();

    repaintwidget();

    return true;
}


void GMapWidget::handleServerParamsRead() {
    assert(_connector!=NULL);

    //reset
    _player = _connector->getPlayer();
    _gmap = _connector->getGMap();

    if(_connector->isSeeker()) { //seeker
        setWindowTitle("You are the Seeker. Click on the cell you want to place your self.");
    } else {//hider
        setWindowTitle("You are the Hider. Click on the cell you want to place your self.");
    }

    //DEBUG_CLIENT_VERB(cout<<"You are playing against "<<_player->getOppUsername()<<endl;);

    //send to server action taken and current position
    resetWindowToMapSize();
    repaintwidget();
}


void GMapWidget::handleServerUpdateReceived(int oppRow, int opCol, int gameStatus) {
    assert(_connector!=NULL);

    //AG140508: get dynamicle obstacles
    //_dynObstVec = _connector->getDynamicObstacleVector();
    //AG140525: should also be in GMap!

    repaintwidget();

    if(gameStatus > HSGlobalData::GAME_STATE_RUNNING) {
        // game finished
        QString msg;
        QString title;

        DEBUG_CLIENT_VERB(cout<<"GAME OVER!"<<endl;);

        switch(gameStatus) {
            case HSGlobalData::GAME_STATE_SEEKER_WON: {
                if(_connector->isSeeker()) {
                    msg="SEEKER, YOU WON!";
                } else {
                    msg = "HIDER, YOU LOST!";
                }
                title = "YOU WIN";
            }
            break;//seeker
            case HSGlobalData::GAME_STATE_HIDER_WON: {
                if(!_connector->isSeeker()) {
                    msg="HIDER, YOU WON!";
                } else {
                    msg = "SEEKER, YOU LOST!";
                }
                title = "YOU LOST";
            }
            break;//hider
            case HSGlobalData::GAME_STATE_TIE:
                msg = "IT'S A TIE!";
                title = "IT'S A TIE";
                break;//tie
            default:
                cout<<"unknown status"<<endl;
                break;
        }
        setWindowTitle(msg);

        QMessageBox::information(this,"Game Over",msg);

        //showDialogForNewGame();
    }
}

void GMapWidget::handleServerDisconnected() {
    cout << "Disconnected from server"<<endl;
    if (_connector!=NULL && _connector->getGameStatus()==HSGlobalData::GAME_STATE_RUNNING) {
        QMessageBox::warning(this,"Disconnected","You have been disconnected from the server.");
    }
    showDialogForNewGame();
}

bool GMapWidget::sendAction() {
    assert(_connector!=NULL);

//msg format: action, current position(x,y)
    if(!_connector->canDoAction()) { //the action was not done
        QMessageBox::information(this,"Warning","The message has not been sent.");
        return false;
    }

    bool ok = false;

    if (_useMouseObs) { //AG130722: use mouse observation, ie user clicks observation
        ok = _connector->sendAction(NULL,&_mouseObs);
    } else {
        Pos p;
        p=_player->getCurPos();
        DEBUG_CLIENT(cout << "sending pos: "<<p.toString()<<endl;);
        ok = _connector->sendAction(NULL,NULL);
    }

    return ok;
}


//-----------|WINDOW CODE|-----------//


void GMapWidget::resetWindowToMapSize() {
    if (_gmap==NULL) {
        setVisible(false);
    } else {
        setVisible(true);
        double w = 1.0 * _gmap->width() * height() / _gmap->height();
        if(_mainWindow!=NULL) {
            _mainWindow->resizeInternalWidget((int)round(w), height());
        } else {
            resize(w,height());
        }
    }
}


void GMapWidget::drawMap(QPainter& painter) {
    Pos playerPos;
    if (_player!=NULL) {
        if (_showOppVisib) //AG140710
            playerPos = _player->getPlayer2Pos();
        else
            playerPos = _player->getCurPos();
    }

    //draw complete map
    for (int r=0; r<_gmap->rowCount(); r++) {
        for (int c=0; c<_gmap->colCount(); c++) {

            bool isVisib = true;
            bool canDoAction = false;

            if (_player!=NULL && playerPos.isSet()) {
                isVisib = _gmap->isVisible(playerPos.row(), playerPos.col(), r, c, true);
            }
            if (_connector!=NULL) {
                canDoAction = _connector->canDoAction();
            }

            drawCell(painter, c, r, _gmap->getItem(r,c), isVisib);
        }
    }

    if (_showCoords>0) {
        //decide font size for coords
        double widthPerCell = width()/_gmap->colCount();
        double heightPerCell = height()/_gmap->rowCount();
        double szPerCell = (widthPerCell<heightPerCell?widthPerCell:heightPerCell);
        int fontSize = (int)(szPerCell/4);

        //draw coords
        for (int r=0; r<_gmap->rowCount(); r++) {
            for (int c=0; c<_gmap->colCount(); c++) {
                if (!_gmap->isObstacle(r,c))
                    drawCoord(painter, c, r, fontSize);
            }
        }
    }

    //draw dynamical obstacles
    vector<IDPos> dynObstVec = _gmap->getDynamicObstacles();
    for(const IDPos& dPos: dynObstVec) { //size_t i=0; i<dynObstVec.size(); i++) {
        bool visib = (_showAllPlayers || _debugMode || _gmap->isVisible(_player->getCurPos(), dPos, false) );
        if (visib) {
            drawPlayer(painter, dPos, Qt::darkYellow, Qt::yellow, false, false, QString::number(dPos.id()));
        }
    }

    //draw players
    if (_player!=NULL && _player->getCurPos().isSet()) {
        //draw other player(s)
        if (_player->getPlayer2Pos().isSet()) {
            bool visib = (_showAllPlayers || _gmap->isVisible(_player->getCurPos(), _player->getPlayer2Pos(), true) );
            if (visib) {
                drawPlayer(painter, _player->getPlayer2Pos(), !_connector->isSeeker(), false, false);
            }
        }

        //draw current player
        drawPlayer(painter, _player->getCurPos(), _connector->isSeeker(), !_showOppVisib, _connector->canDoAction());

        //AG150114: draw other player (if exists)
        if (_player->getPlayer3Pos().isSet()) {
            bool visib = (_showAllPlayers || _gmap->isVisible(_player->getCurPos(), _player->getPlayer3Pos(), true) );
            if (visib) {
                drawPlayer(painter, _player->getPlayer3Pos(), Qt::darkBlue, Qt::cyan, false, false, "s2");
            }
        }
    }
    if (_debugMode) {
        if (_debugStartPos.isSet()) {
            drawPlayer(painter, _debugStartPos, true, false, false);
        }
        if (_debugEndPos.isSet())
            drawPlayer(painter, _debugEndPos, false, false, false);
    }

    if (_debugMode) {
        if (_debugPath.size()>1) {
            //convert path to QPoints
            //cout <<"Print path:"<<endl;
            QVector<QPoint> lines;
            for(const Pos& p : _debugPath) {
                QPoint qp = GMapToWidgetPoint(p);
                if (lines.size()>1)
                    lines.push_back(lines[lines.size()-1]); //repeat last to make pairs
                lines.push_back(qp);
                //cout <<" * "<<p.toString()<<" -> ("<<qp.x()<<","<<qp.y()<<")"<<endl;
            }
            QPen pen(Qt::darkGreen, 3);
            painter.setPen(pen);
            painter.drawLines(lines);

            if (_debugNextStep.isSet()) {
                //convert next point to QPoint
                QPen pen(Qt::green, 3);
                painter.setPen(pen);
                QPoint p1 = GMapToWidgetPoint(_debugPath[0]); //.colDouble(), _debugStartPos.rowDouble());
                QPoint p2 = GMapToWidgetPoint(_debugNextStep); //.colDouble(), _debugNextStep.rowDouble());
                painter.drawLine(p1,p2);
            }
        }
    }
}


void GMapWidget::drawPlayer(QPainter &painter, const Pos &p, bool isSeeker, bool isMe, bool canDoAction) {
    if (isSeeker)
        drawPlayer(painter, p, Qt::darkBlue, Qt::blue, isMe, canDoAction, "S");
    else
        drawPlayer(painter, p, Qt::darkRed, Qt::red, isMe, canDoAction, "H");
}

void GMapWidget::drawPlayer(QPainter &painter, const Pos &p, Qt::GlobalColor penColor, Qt::GlobalColor brushColor, bool isMe, bool canDoAction, QString id) {
    double x = p.colDouble();
    double y = p.rowDouble();
    if ( (_connector!=NULL && _connector->getParams()->useContinuousPos)
          || (_gmap->getParams()->useContinuousPos)
       ) { //AG140525,: TODO should be moved to a global _params
        //cout << " coord before: "<<x<<","<<y;
        //coordinates should give the centre
        x -= 0.5;// * width()  /_gmap->colCount();
        y -= 0.5; // * height() /_gmap->rowCount();
        //cout << " after: "<<x<<","<<y<<endl;
    }

    QRect r = GMapToWidgetRect(x, y);
    float t = 5.0;
    QRect rp = GMapToWidgetRect(x, y, t);

    painter.setPen(penColor);
    painter.setBrush(brushColor);

    painter.drawEllipse(r);

    if (isMe) {
        if(!canDoAction)
            painter.fillRect(rp,Qt::darkRed);
        else
            painter.fillRect(rp,Qt::darkGreen);
    }

    /*QFont font = painter.font();

    font.setPointSize(fontSize);
    painter.setFont(font);*/
    QPen penText(Qt::black);
    painter.setPen(penText);
    //painter.setBrush(Qt::black);
    painter.drawText(r,Qt::AlignCenter,id);
}

void GMapWidget::drawCell(QPainter& painter, int x, int y, int c, bool isVisib /*, bool f*/) {
    QRect r = GMapToWidgetRect(x, y);
    float s, t = 5.0;
    QRect rp = GMapToWidgetRect(x, y, t);    

    bool isBase = (c==GMap::GMAP_BASE);

    switch(c) {
        case GMap::GMAP_FREE_SPACE:
            painter.setPen(Qt::black);
            if (isVisib) {
                painter.setBrush(Qt::NoBrush);
            } else {
                painter.setBrush(Qt::lightGray);
            }
            break;
        case GMap::GMAP_OBSTACLE:
            painter.setPen(Qt::black);
            painter.setBrush(Qt::black);
            break;
        case GMap::GMAP_BASE:
            painter.setPen(Qt::black);
            painter.setBrush(Qt::darkGray);
            break;
        default:
            cout << "Unknown map item: " << c << " at (" << x << "," << y <<")"<<endl;
            assert(false);
    }

//send to server action taken and current position

    //show square as grid
    painter.drawRect(r);
}

void GMapWidget::drawCoord(QPainter &painter, int x, int y, int fontSize) {
    if (_showCoords>0) {
        QRect r = GMapToWidgetRect(x, y);

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

        QFont font = painter.font();

        font.setPointSize(fontSize);
        painter.setFont(font);
        QPen penText(Qt::darkGreen);
        painter.setPen(penText);
        //painter.setBrush(Qt::black);
        painter.drawText(r,Qt::AlignCenter,coordS);
    }

}


void GMapWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    if (_gmap != NULL) {
        drawMap(painter);
    }
}


QRect GMapWidget::GMapToWidgetRect(QPoint& p) {
    return GMapToWidgetRect(p.x(), p.y());
}

QRect GMapWidget::GMapToWidgetRect(double px, double py) {
    //calc the cell center of the discrete boxes
    int dx = round(1.0f * width()  /_gmap->colCount());
    int dy = round(1.0f * height() /_gmap->rowCount());

    //calculate the (x,y) on the widget using the row and col
    int x = round(1.0f * px * width() / (_gmap->colCount() )) - 1;
    int y = round(1.0f * py * height() / (_gmap->rowCount() )) - 1;

    return QRect(x,y,dx,dy);
}


QRect GMapWidget::GMapToWidgetRect(double px, double py, float f) {
    //calc the cell center of the discrete boxes
    int dx1 = round(1.0f * width()  /_gmap->colCount());
    int dy1 = round(1.0f * height() /_gmap->rowCount());
    int dx=dx1/f;
    int dy=dy1/f;

    //calculate the (x,y) on the widget using the row and col
    int x = round(1.0f * px * width() / (_gmap->colCount() )) - 1;
    int y = round(1.0f * py * height() / (_gmap->rowCount() )) - 1;

    x += dx1-dx;
    y += dy1 -dy;

    return QRect(x,y,dx,dy);
}

QRect GMapWidget::GMapToWidgetRect(double px, double py, float f, float s) {
    //calc the cell center of the discrete boxes
    int dx1 = round(1.0f * width()  /_gmap->colCount());
    int dy1 = round(1.0f * height() /_gmap->rowCount());
    int dx=dx1/f;
    int dy=dy1/f;

    //calculate the (x,y) on the widget using the row and col
    int x = round(1.0f * px * width() / (_gmap->colCount() )) - 1;
    int y = round(1.0f * py * height() / (_gmap->rowCount() )) - 1;

    x += (dx1/2)-(dx/2);
    y += dy1 -dy;

    return QRect(x,y,dx,dy);
}

QPoint GMapWidget::GMapToWidgetPoint(double px, double py) {
    int x = round(1.0f * px * width() / (_gmap->colCount() )) - 1;
    int y = round(1.0f * py * height() / (_gmap->rowCount() )) - 1;
    return QPoint(x,y);
}

QPoint GMapWidget::GMapToWidgetPoint(Pos p) {
    if (!p.hasDouble() /*&& _gmap->getParams()->useContinuousPos*/) p.add(0.5,0.5); //always show in center of a cell
    double px = p.colDouble();
    double py = p.rowDouble();
    int x = round(1.0f * px * width() / (_gmap->colCount() )) - 1;
    int y = round(1.0f * py * height() / (_gmap->rowCount() )) - 1;
    return QPoint(x,y);
}

void GMapWidget::keyPressEvent ( QKeyEvent * event ) {
    //keys always available:
    switch(event->key()) {
        case Qt::Key_P:
        case Qt::Key_F4:
            saveScreen(event->modifiers() & Qt::ShiftModifier);
            return;
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
        case Qt::Key_F5:
            _allowClickNextPos = !_allowClickNextPos;
            cout << "Now "<< (_allowClickNextPos?"":"NOT") << " allowing to click for next position"<<endl;
            return;
        case Qt::Key_Plus: {
            /*int w = (int)round(width()*1.10);
            int h = (int)round(height()*1.10);
            //setStatus("Set new size to: "+QString::number(w)+"x"+QString::number(h));
            _mainWindow->resize(w,h);*/
            if(_mainWindow!=NULL)
                _mainWindow->resizeFactor(1.1);
            return;
        }
        case Qt::Key_Minus: {
            /*int w = (int)round(width()*0.9);
            int h = (int)round(height()*0.9);
            resize(w,h);*/
            if(_mainWindow!=NULL)
                _mainWindow->resizeFactor(0.9);
            return;
        }
        case Qt::Key_Escape:{
            if (_connector!=NULL)
                _connector->disconnect();
            exit(0);
            return;
        }
    }

    if (_onlyShowMap) {
        switch(event->key()) {
            case Qt::Key_F6: {
                _debugMode = !_debugMode;
                cout <<"Debug mode is: "<<(_debugMode?"on":"off")<<endl;
                if (_debugMode)
                    setTitle("Hide & Seek Client - Debug mode");
                else
                    setTitle("Hide & Seek Client");
                break;
            }
            case Qt::Key_F7: {
                _wstate = WState::placeBase;
                break;
            }
            case Qt::Key_F8: {
                if (QMessageBox::question(this, "Delete rows", "Do you want to click the row?")==QMessageBox::Yes) {
                    _wstate = WState::deleteRow;
                } else {
                    bool ok=false;
                    int sr = QInputDialog::getInt(this,"Delete rows","Start row",0,0,_gmap->rowCount()-1,1,&ok);
                    if (!ok) return;
                    int er = QInputDialog::getInt(this,"Delete rows","End row",sr,sr,_gmap->rowCount()-1,1,&ok);
                    if (!ok) return;
                    _gmap->deleteRows(sr,er);
                }
                break;
            }
            case Qt::Key_F9: {
                if (QMessageBox::question(this, "Delete cols", "Do you want to click the col?")==QMessageBox::Yes) {
                    _wstate = WState::deleteCol;
                } else {
                    bool ok=false;
                    int sc = QInputDialog::getInt(this,"Delete cols","Start col",0,0,_gmap->colCount()-1,1,&ok);
                    if (!ok) return;
                    int ec = QInputDialog::getInt(this,"Delete cols","End col",sc,sc,_gmap->colCount()-1,1,&ok);
                    if (!ok) return;
                    _gmap->deleteCols(sc,ec);
                }
                break;
            }
            case Qt::Key_F10:  {
                bool editMode = _gmap->getEditMode();

                if (editMode) {
                    editMode=false;
                    setTitle("Hide & Seek Client");
                } else {
                    editMode = true;
                    setTitle("Hide & Seek Client - Edit mode");
                }
                _gmap->setEditMode(editMode);
                cout << "Edit mode "<< (_gmap->getEditMode()?"enabled":"disabled")<<endl;
                break;
            }
            case Qt::Key_F11: {
                loadMap();
                break;
            }
            case Qt::Key_F12: {
                saveMap();
                break;
            }
    }

        //AG140524: debug mode
    if (_debugMode) {
        switch(event->key()) {
            case Qt::Key_O:
            case Qt::Key_S:
                _wstate = WState::debugSetStart;
                break;
            case Qt::Key_G:
            case Qt::Key_E:
                _wstate = WState::debugSetGoal;
                break;
            case Qt::Key_C:
                _gmap->getParams()->useContinuousPos = true;
                if (_debugStartPos.isSet() && !_debugStartPos.hasDouble()) _debugStartPos.add(0.5,0.5);
                if (_debugEndPos.isSet() && !_debugEndPos.hasDouble()) _debugEndPos.add(0.5,0.5);
                cout <<"Setting continuous movements"<<endl;
                break;
            case Qt::Key_D:
                _gmap->getParams()->useContinuousPos = false;
                if (_debugStartPos.isSet()) _debugStartPos.convertValuesToInt();
                if (_debugEndPos.isSet()) _debugEndPos.convertValuesToInt();
                cout <<"Setting discrete movements"<<endl;
                break;
            case Qt::Key_F:
                if (!_debugStartPos.isSet()){
                    QMessageBox::warning(this, "Find Shortest Path", "Set a start position first.");
                } else if (!_debugEndPos.isSet()){
                    QMessageBox::warning(this, "Find Shortest Path", "Set an end position first.");
                } else {
                    _debugPath.clear();
                    //TODO: FILL DEBUGPATH, and nextstep (using 1 distance.. of gmap)
                    //TODO2: paint

                    //now get path
                    Pos p = _debugStartPos;
                    _debugPath.push_back(p);
                    cout <<"Path from "<<_debugStartPos.toString()<<" to "<<_debugEndPos.toString()<<":"<<endl;
                    while (!p.equalsInt(_debugEndPos)) {
                        if (!p.isSet()) {
                            cout <<" -> FAILED"<<endl;
                            break;
                        }
                        p = _gmap->getNextStep(p, _debugEndPos);
                        _debugPath.push_back(p);
                        cout <<" * "<<p.toString()<<endl;
                    }
                    cout << " --> distance = "<< flush<<_gmap->distance(_debugStartPos, _debugEndPos)<<endl;
                    //set next step
                    if (_debugPath.size()>1) {
                        Pos nextPos = _debugPath[1];
                        if (_gmap->getParams()->useContinuousPos && !nextPos.hasDouble()) nextPos.add(0.5,0.5);
                        double dir = _gmap->getDirection(_debugStartPos,nextPos);

                        //move in the direction only distance 1 (also in diagonal)
                        _debugNextStep = _gmap->tryMoveDirStep(dir, _debugStartPos, 1, 0.8, true);

                        cout <<"Next step from "<<_debugStartPos.toString()<<": "<<_debugNextStep.toString()<<" dir="<<(180*dir/M_PI)<<"º"<<endl;
                    } else {
                        _debugNextStep.clear();
                    }
                    repaint();
                }
                break;
            case Qt::Key_V:
                if (!_debugStartPos.isSet()){
                    QMessageBox::warning(this, "Is Visible", "Set a start position first.");
                } else if (!_debugEndPos.isSet()){
                    QMessageBox::warning(this, "Is Visible", "Set an end position first.");
                } else {
                    //_debugPath.clear();
                    //TODO: FILL DEBUGPATH, and nextstep (using 1 distance.. of gmap)
                    //TODO2: paint
                    bool v = _gmap->isVisible(_debugStartPos, _debugEndPos, true);
                    cout << "Is visible: "<<_debugStartPos.toString()<<" - "<<_debugEndPos.toString()<<": "<< (v?"true":"false") <<endl;
                }
                break;
            case Qt::Key_A:
                _wstate = WState::debugAddDynObst;
                break;
            case Qt::Key_R:
                _wstate = WState::debugRemDynObst;
                break;
            case Qt::Key_X: {
                    _debugPath.clear();
                    _debugNextStep.clear();
                    if (event->modifiers() & Qt::ShiftModifier) {
                        _debugEndPos.clear();
                        _debugStartPos.clear();
                        _gmap->removeDynamicObstacles();
                    }
                    repaint();
                }
                break;
            case Qt::Key_Tab: {
                    SetParamsDialog setParamDiag(_gmap->getParams(), this);
                    setParamDiag.exec();
                }
                break;
            case Qt::Key_M: {
                    if (_debugStartPos.isSet()) {
                        cout <<"Setting minimum obst. distance for pos "<<_debugStartPos.toString()<<endl;
                        getDebugAutoPlayer()->setMinDistanceToObstacle(_debugStartPos);
                        cout <<"->"<< _debugStartPos.toString()<<endl;
                    }
                    if (_debugEndPos.isSet()) {
                        cout <<"Setting minimum obst. distance for pos "<<_debugEndPos.toString()<<endl;
                        getDebugAutoPlayer()->setMinDistanceToObstacle(_debugEndPos);
                        cout <<"->"<< _debugEndPos.toString()<<endl;
                    }
                    repaint();
                }
                break;
            case Qt::Key_T: {
                    if (_debugStartPos.isSet()) {
                        cout<<"Trying all actions from "<<_debugStartPos.toString()<<":"<<endl;
                        for(int a=0;a<HSGlobalData::NUM_ACTIONS;a++) {
                            cout << "  * "<<ACTION_COUT(a)<<": "<<_gmap->tryMove(a,_debugStartPos).toString()<<endl;
                        }
                    }
                }
                break;
            case Qt::Key_Comma: {
                    _debugStartPos = _gmap->genRandomPos();
                    cout <<"Random start pos "<<_debugStartPos.toString()<<endl;
                    _debugEndPos = _gmap->genRandomPos();
                    cout <<"Random end pos "<<_debugEndPos.toString()<<endl;
                    repaint();
                }
            case Qt::Key_Ampersand:
                _gmap->getParams()->pathPlannerType = SeekerHSParams::PATHPLANNER_PROPDIST;
                _gmap->createPathPlanner();
                cout <<"New Path planner: "<<_gmap->getPathPlanner()->getName()<<endl;
                break;
            case Qt::Key_Asterisk:
                _gmap->getParams()->pathPlannerType = SeekerHSParams::PATHPLANNER_ASTAR;
                _gmap->createPathPlanner();
                cout <<"New Path planner: "<<_gmap->getPathPlanner()->getName()<<endl;
                break;

            }
        }

    } else if (_connector!=NULL) { // !_onlyShowMap

        if (!_connector->isMyInitPosSet() ) { //if the initial position was not set yet from the hider then dont make a move yet.
            QMessageBox::information(this, "Set Position", "Please set your own position first by clicking on the map.");
            return;
        } else if (!_connector->isOppPosSet()) {
            QString msg = "Wait for the opponent ";
            msg += (_connector->isSeeker() ? "hider":"seeker");
            msg += " to set its position.";
            QMessageBox::information(this, "Wait for Opponent", msg);
            return;
        } else if(_connector->getGameStatus()!=HSGlobalData::GAME_STATE_RUNNING) { //_player->getwin()!=2)
            cout << "Game already finished!"<<endl;
            return;
        } else if (!_connector->canDoAction()) {
            QMessageBox::information(this, "Not your turn yet", "You cannot move yet.");
            return;
        }

        bool madeMovement = false;


        //AG120607: updated keys -> use numeric keys
        switch(event->key()) {
            case Qt::Key_Y:
            case Qt::Key_Up:
            case Qt::Key_8:
                // move north
                madeMovement = _player->move(HSGlobalData::ACT_N);
                break;
            case Qt::Key_U:
            case Qt::Key_PageUp:
            case Qt::Key_9:
                // move north-east
                madeMovement = _player->move(HSGlobalData::ACT_NE);
                break;
            case Qt::Key_H:
            case Qt::Key_Right:
            case Qt::Key_6:
                // move east
                madeMovement = _player->move(HSGlobalData::ACT_E);
                break;
            case Qt::Key_N:
            case Qt::Key_PageDown:
            case Qt::Key_3:
                // move east-south
                madeMovement = _player->move(HSGlobalData::ACT_SE);
                break;
            case Qt::Key_B:
            case Qt::Key_Down:
            case Qt::Key_2:
                // move south
                madeMovement = _player->move(HSGlobalData::ACT_S);
                break;
            case Qt::Key_V:
            case Qt::Key_End:
            case Qt::Key_1:
                // move south-west
                madeMovement = _player->move(HSGlobalData::ACT_SW);
                break;
            case Qt::Key_G:
            case Qt::Key_Left:
            case Qt::Key_4:
                // move west
                madeMovement = _player->move(HSGlobalData::ACT_W);
                break;
            case Qt::Key_T:
            case Qt::Key_Home:
            case Qt::Key_7:
                // move west-north
                madeMovement = _player->move(HSGlobalData::ACT_NW);
                break;
            case Qt::Key_Space:
            case Qt::Key_Insert:
            case Qt::Key_0:
                madeMovement = _player->move(HSGlobalData::ACT_H);
                break; //don´t move
            case Qt::Key_Asterisk:
                _showAllPlayers = !_showAllPlayers;
                repaintwidget();
                break;
            case Qt::Key_Ampersand:
                _showOppVisib = !_showOppVisib;
                cout << "Showing visibility of: "<<(_showOppVisib?"opponent":"own player")<<endl;
                break;
            case Qt::Key_Backslash:
                //AG150120: test message
                QString msg="Test message";
                QByteArray msgBA=msg.toStdString().c_str();
                MessageSendTo to = (MessageSendTo)random(0,6);
                cout << "Sending message to: "<<HSGlobalData::MESSAGESENDTO_NAMES[to].toStdString()<<endl;
                _connector->sendMessage(to, msgBA);
                break;
        }

        if(madeMovement ) { //the action was made on the client
            if (!sendAction()) //send action to the server
                cout<<"the action was NOT sent"<<endl;

            setWindowTitleWithSteps();

            repaintwidget();
        }
    } // !onlyShowMap
}

void GMapWidget::setWindowTitleWithSteps() {
    assert(_connector!=NULL);

    QString title = "Hide-and-Seek - ";

    if(_connector->isSeeker()) {
        //title.append("\t You are the Seeker! \t");
        title.append("Seeker");
    } else  {
        //title.append("\t You are the Hider! \t");
        title.append("Hider");
    }

    // set title
    title.append(" - against: ");
    title.append(QString::fromStdString(_player->getPlayer2Username()));

    string p3Username = _player->getPlayer3Username();
    if (!p3Username.empty()) {
        title.append(" - with: ");
        title.append(QString::fromStdString(p3Username));
    }

    setTitle(title);

    QString status = "Action "+QString::number(_player->getNumActions()) + "/" + QString::number(_connector->getParams()->maxNumActions);
    setStatus(status);
}


void GMapWidget::showHelp() {
    stringstream sstr;

    //TODO: visual
    sstr    << "<tt><font size=-1>"
           /* << " Hide & Seek client - Help"<<endl
            << "--------------------------"<<endl*/;
    if (!_onlyShowMap)
            sstr
            << "Y / Up / 8      - go up/north<br/>"<<endl
            << "U / PgUp / 9    - go up-right/north-east<br/>"<<endl
            << "H / Right / 6   - go right/east<br/>"<<endl
            << "N / PgDown / 3  - go down-right/south-east<br/>"<<endl
            << "B / Down / 2    - go down/south<br/>"<<endl
            << "V / End / 1     - go down-left/south-west<br/>"<<endl
            << "G / Left / 4    - go left/west<br/>"<<endl
            << "T / Home / 7    - go up-left/north-west<br/>"<<endl
            << "space / ins / 0 - halt/don't move<br/>"<<endl
            << "Y / Up / 8      - go up/north<br/>"<<endl<<endl;

     sstr   << "F1              - this help<br/>"<<endl
            << "F2              - show/hide coordinates (x,y) or row-col or index<br/>"<<endl
            << "F3              - toggle between 0 or 1-based coordinates<br/>"<<endl
            << "F4 / P          - print screen to hideandseek.png<br/>"<<endl
            << "Shift - F4 / P  - print screen to file (asked)<br/>"<<endl
            << "F5              - toggle allow click of next position<br/>"<<endl
            << "+ / -           - increase/decrease window by 10%<br/>"<<endl
            << "Esc             - exit<br/>"<<endl<<endl;
     if (_onlyShowMap)
            sstr
            << "F6              - debug mode<br/>"<<endl
            << "F7              - place base<br/>"<<endl
            << "F8              - delete row<br/>"<<endl
            << "F9              - delete col<br/>"<<endl
            << "F10             - change edit mode<br/>"<<endl
            << "F11             - load map<br/>" <<endl
            << "F12             - save map<br/>"<<endl;
     if (_debugMode)
            sstr
            << "O / S           - place origin/goal<br/>"<<endl
            << "G / E           - place goal/end<br/>"<<endl
            << "F               - find shortest path between start - end<br/>"<<endl
            << "V               - check if visible<br/>"<<endl
            << "C               - set continuous movements<br/>"<<endl
            << "D               - set discrete movements<br/>"<<endl
            << "A               - add dyn. obst/person<br/>"<<endl
            << "R               - remove dyn. obst/person<br/>"<<endl
            << "X               - clear path<br/>"<<endl
            << "Shift-X         - clear all<br/>"<<endl
            << "M               - set debug pos. to min distance<br/>"<<endl
            << "T               - test action movements from start pos</br>"<<endl
            << ",               - set random pos for start and end<br/>"<<endl
            << "*               - use A* path planner<br/>"<<endl
            << "&               - use propagation path planner<br/>"<<endl
            << "Tab             - change parameters<br/>"<<endl;
      sstr  <<endl<<"</font></tt>";

     string helpStr = sstr.str();
     cout << helpStr;
     QMessageBox::information(this, "Hide & Seek client - Help", QString::fromStdString(helpStr));
}


/*QPoint*/ Pos GMapWidget::WidgetToGMapPoint(QPoint& p) {
    //calc the cell center of the discrete boxes
    double dx = (1.0 * width()  / _gmap->colCount());
    double dy = (1.0 * height() / _gmap->rowCount());

    //calculate the (x,y) on the widget using the row and col
    /*int x = floor(p.x()/dx);
    int y = floor(p.y()/dy);*/
    double x = p.x() / dx;
    double y = p.y() / dy;

    //return QPoint(x,y);
    return Pos(y,x);
}


// used to 'paint' obstacles
void GMapWidget::mouseMoveEvent(QMouseEvent *e) {
    if (_gmap==NULL) return;

    //only edit mode
    if (_gmap->getEditMode() == false) {
        return;
    }

    //get position of event
    QPoint p = e->pos();

    //check inside window
    if (p.x()<0 || p.x()>=width() || p.y()<0 || p.y()>=height()) {
        //ignore because out of window
        return;
    }

    Pos pos = WidgetToGMapPoint(p);

    //for editing a map
    if (!_prevClickPos.equalsInt(pos)) { //  (p.x()!=_prevClickPoint.x() || p.y()!=_prevClickPoint.y()) ) {
        _prevClickPos = pos;

        if (_lastMouseButton==Qt::LeftButton && !_gmap->isObstacle(pos)) {
            _gmap->addObstacle(pos);
        } else if (_lastMouseButton==Qt::RightButton && _gmap->isObstacle(pos)) {
            _gmap->setItem(pos.row(),pos.col(),GMap::GMAP_FREE_SPACE);
        }

        repaint();
    }
}

//ag 111019
void GMapWidget::mousePressEvent(QMouseEvent *e) {
    _lastMouseButton = e->button();
    if (_gmap==NULL) return;

    QPoint point = e->pos();
    Pos pos /*point*/ = WidgetToGMapPoint(point);

    //ag130722
    DEBUG_CLIENT(cout << "Clicked: "<< pos.toString() /*point.x()<<","<<point.y()*/ <<""<<endl;);

    if (!_onlyShowMap && _connector!=NULL && _connector->haveServerParamsBeenRead() && !_connector->isMyInitPosSet()) {
        if(!initPos(pos)) {
            return;
        }

        setWindowTitleWithSteps();
    } else if (_useMouseObs){
        //ag130722
        //use as observation (if set) (note row=y, col=x)
        _mouseObs = pos;
    } else if (_allowClickNextPos && _connector != NULL) {
        //Pos newPos(point.y(),point.x());
        if (_gmap->isPosInMap(pos) && !_gmap->isObstacle(pos)) {
            DEBUG_CLIENT(cout << "Sending as position"<<endl;);
            _player->setCurPos(pos);
            if (!sendAction()) cout << "Failed to send clicked position as next action"<<endl;
        }
    }

    if (_gmap->getEditMode()) {
        //Pos pos(point);

        switch (e->button()) {
            case Qt::LeftButton: {
                switch (_wstate) {
                    case WState::normal: {
                        /*if (_gmap->isObstacle(pos)) {
                            _gmap->setItem(pos.row(),pos.col(),GMap::GMAP_FREE_SPACE);
                        } else {
                            _gmap->addObstacle(pos);
                        }*/
                        if (!_gmap->isObstacle(pos)) {
                            _gmap->addObstacle(pos);
                        }

                        //prev button clicked, used to 'paint' obstacles
                        //_prevClickPoint = point;
                        _prevClickPos = pos;
                        break;
                    }
                    case WState::placeBase: {
                        //remove old base
                        Pos base = _gmap->getBase();
                        if (base.isSet()) {
                            _gmap->setItem(base.row(),base.col(),GMap::GMAP_FREE_SPACE);
                        }
                        //set new base
                        _gmap->setBase(pos);
                        break;
                    }
                    case WState::deleteCol: {
                        _gmap->deleteCols(pos.col(),pos.col());
                        break;
                    }
                    case WState::deleteRow: {
                        _gmap->deleteRows(pos.row(),pos.row());
                        break;
                    }
                }
                //reset state
                _wstate = WState::normal;
                break;
            }
            case Qt::RightButton: {
                /*//remove old base
                Pos base = _gmap->getBase();
                if (base.isSet()) {
                    _gmap->setItem(base.row(),base.col(),GMap::GMAP_FREE_SPACE);
                }
                //set new base
                _gmap->setBase(pos);*/
                if (_gmap->isObstacle(pos)) {
                    _gmap->setItem(pos.row(),pos.col(),GMap::GMAP_FREE_SPACE);
                }
                break;
            }
        }

        //repaint since new obstacles
        repaint();
    }

    if (_debugMode && e->button()==Qt::LeftButton) {
        if (_wstate!=WState::normal && _gmap->isObstacle(pos)) {
            QMessageBox::warning(this,"Obstacle","The clicked location is an obstacle!");
            return;
        }
        if (!_gmap->getParams()->useContinuousPos) pos.convertValuesToInt();
        switch(_wstate) {
            case WState::debugSetGoal:
                _debugEndPos = pos;
                break;
            case WState::debugSetStart:
                _debugStartPos = pos;
                break;
            case WState::debugAddDynObst:
                cout << "Adding dyn. obst:"<<pos.toString()<<endl;
                _gmap->addDynamicObstacle(IDPos(pos,0));
                break;
            case WState::debugRemDynObst: {
                vector<IDPos> dynObstVec = _gmap->getDynamicObstacles();
                int minI=-1;
                double minD=1e5;
                for(size_t i=0;i<dynObstVec.size();i++) {
                    double d=pos.distanceEuc(dynObstVec[i]);
                    if (d<1 && d<minD) {
                        minI=i;
                        minD=d;
                    }
                }
                if (minI>=0) {
                    dynObstVec.erase(dynObstVec.begin()+minI);
                    _gmap->setDynamicObstacles(dynObstVec);
                }
                break;
            }
        }
        repaint();
        _wstate = WState::normal;
    }
}



void GMapWidget::saveScreen(bool askFileName) {
    cout << "Saving screen " << flush;
    QString fileName = "hideandseek.png";
    if (askFileName) {
        fileName = QFileDialog::getSaveFileName(this,
                 tr("Save Screenshot"), "", tr("Images (*.png);;All files (*.*)"));
        if (fileName.isEmpty()) {
            cout <<"cancelled"<<endl;
            return;
        }
    }
    cout << "as "<< fileName.toStdString()<<": "<<flush;
    QImage img(size(),QImage::Format_ARGB32);
    //QPicture img();
    QPainter painter(&img);
    render(&painter);
    img.save(fileName,"PNG");
    cout << "ok" << endl;
}



void GMapWidget::loadMap() {
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Map"), "", tr("Map Text Files (*.txt *.map);;Image Map File (*.pgm);;All files (*.*)"));

    if (!fileName.isEmpty()) {
        cout << "Map file: '"<<fileName.toStdString()<<"'"<<endl;
        GMap *newMap = new GMap(fileName.toStdString(),NULL);
        cout << "Loaded file:"<<endl;
        newMap->printMap();
        cout << "Switch:"<<flush;
        GMap* oldMap = _gmap;
        _gmap = newMap;
        cout << "ok"<<endl<< "delete old map: "<<flush;
        delete oldMap;
        cout<<"ok"<<endl;

        repaint();
    }


}

void GMapWidget::saveMap() {
    QString fileName = QFileDialog::getSaveFileName(this,
         tr("Save Map"), "", tr("Map Text Files (*.txt *.map);;Image Map File (*.pgm);;All files (*.*)"));

    if (!fileName.isEmpty()) {
        cout << "Save to: '"<<fileName.toStdString()<<"' ... "<<flush;
        _gmap->writeMapFile(fileName.toStdString().c_str());
        cout << "done"<<endl;
    }
}

void GMapWidget::repaintwidget() {
    repaint(0,0,-1,-1);
}


AutoPlayer* GMapWidget::getDebugAutoPlayer() {
    if (_debugAutoPlayer==NULL)
        _debugAutoPlayer = new AbstractAutoPlayer(_gmap->getParams(), _gmap);
    return _debugAutoPlayer;
}

void GMapWidget::setMainWindow(HSClientMainWindow *mainWindow) {
    assert(mainWindow!=NULL);
    _mainWindow = mainWindow;
    _mainWindow->setTitle(_titleMW);
    _mainWindow->setStatus(_statusMW);
}

void GMapWidget::setTitle(QString title) {
    _titleMW = title;
    if(_mainWindow!=NULL) _mainWindow->setTitle(title);
}

void GMapWidget::setStatus(QString status) {
    _statusMW = status;
    if (_mainWindow!=NULL) _mainWindow->setStatus(status);
}

void GMapWidget::sendStopToServer() {
    if (_connector != NULL)
        _connector->sendStopRequest();
}
