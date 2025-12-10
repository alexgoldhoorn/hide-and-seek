#ifndef GMAPWIDGET_H
#define GMAPWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QFile>
#include <QTimer>
#include <QtNetwork/QTcpSocket>
#include "HSGame/gplayer.h"
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>
#include "popup.h"

class GMapWidget : public QWidget {
    Q_OBJECT
public:
    //AG120607: show coords modes
    static const char SHOW_COORD_XY = 1;
    static const char SHOW_COORD_RC = 2;
    static const char SHOW_COORD_I = 3;

    //static const int MAXACTIONS         = 100;

    GMapWidget(QWidget *parent = 0);

    //ag120509: added to have a visual example of map
    GMapWidget(GMap* gmap, QWidget *parent = 0);

    ~GMapWidget();

    GMap* getGMap() {
        return _gmap;
    }
    Player* getPlayer() {
        return _player;
    }


    void Repaintevent() {
        paintEvent(0);
    }

    void repaintwidget() {
        repaint(0,0,-1,-1);
    }


    //calculate the rectangle in the discrete map
    QRect GMapToWidgetRect(QPoint&);
    QRect GMapToWidgetRect(int px, int py);
    QRect GMapToWidgetRect(int px, int py, float f);
    QRect GMapToWidgetRect(int px, int py, float f, float s);

    QPoint WidgetToGMapPoint(QPoint& p);
    void mousePressEvent(QMouseEvent *e);
    void saveScreen();

    //draw a cell in the discrete map
    void drawCell(QPainter& painter, int x, int y, int c, int v, bool f);
    //draw the full map
    void drawMap(QPainter& painter);

    int InitPos(QPoint h);

    void initialposh();

//private slots:
    bool sendaction(); //1-for success, 0-for failure.

    void replay();

    void sendinfo();

    void showHelp();

    //AG120903: temp file for actions
    QString _actionFile;

public slots:
    void readyRead();
    void showDialog();


protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent ( QKeyEvent * event );

private:
    GMap* _gmap;
    Player* _player;
    void init(GMap* gmap);
    void init(Player* pl);
    QTcpSocket *tcpSocket;
    quint16 blockSize;
    Dialog *_dialog;
    QString _ip;
    int _port;
    int _map; //the map that the user chose at the popup. 0-4 : 5 possible maps.
    int _opp; //the opponent the user chose. [0-3]. 4 possible opponents. 0-a human, 1-a dummy hider,2- an intelligent hider, 3-SeekroBot.
    bool _type; //1 for seeker, 0 for hider
    bool _hidinitpos; //flag to show if the initialposition of hider was chosen (1) or not(0)


    bool _onlyShowMap; //ag120509: only show map

    //AG120607: choose between showing coords
    char _showCoords;
    bool _coordZeroBased;

};

#endif // GMAPWIDGET_H
