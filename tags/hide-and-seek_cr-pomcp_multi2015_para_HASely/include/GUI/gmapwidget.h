#ifndef GMAPWIDGET_H
#define GMAPWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QFile>
#include <QTimer>
#include <vector>

#include "popup.h"
#include "HSGame/gplayer.h"
#include "HSGame/idpos.h"

#include "Base/gameconnectorclient.h"
#include "Base/autoplayer.h"

//#ifndef HSCLIENTMAINWINDOW_H
class HSClientMainWindow;
//#endif

class GMapWidget : public QWidget {
    Q_OBJECT
public:
    //AG120607: show coords modes
    static const char SHOW_COORD_XY = 2;
    static const char SHOW_COORD_RC = 1;
    static const char SHOW_COORD_I = 3;

    enum WState { normal, deleteRow, deleteCol, placeBase, debugSetStart, debugSetGoal, debugAddDynObst, debugRemDynObst };

    GMapWidget(bool useMouseObs, bool allowClickNextPos, QString metaInfo, QString comments, QWidget *parent = 0);

    //ag120509: added to have a visual example of map
    GMapWidget(GMap* gmap, QWidget *parent = 0);

    ~GMapWidget();


    //! repaint the widget
    void repaintwidget();


    //calculate the rectangle in the discrete map
    QRect GMapToWidgetRect(QPoint&);
    QRect GMapToWidgetRect(double px, double py);
    QRect GMapToWidgetRect(double px, double py, float f);
    QRect GMapToWidgetRect(double px, double py, float f, float s);

    QPoint GMapToWidgetPoint(double x, double y);
    QPoint GMapToWidgetPoint(Pos p);


    Pos /*QPoint*/ WidgetToGMapPoint(QPoint& p);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent ( QKeyEvent * event );

    void setMainWindow(HSClientMainWindow* mainWindow);

    //! send stop request to server
    void sendStopToServer();

    void resetWindowToMapSize();

protected slots:
    //void serverReadyRead();
    void showDialogForNewGame();


    void handleServerParamsRead();

    /*!
     * \brief serverUpdateReceived informs about newly received info from server
     * \param gameStatus
     */
    void handleServerUpdateReceived(int gameStatus);

    void handleServerDisconnected();


protected:

    virtual void paintEvent(QPaintEvent *event);


    void loadMap();
    void saveMap();



    void saveScreen(bool askFileName);

    //draw a cell in the discrete map
    void drawCell(QPainter& painter, int x, int y, int c, bool isVisib);
    //draw player
    void drawPlayer(QPainter& painter, const Pos& p, bool isSeeker, bool isMe, bool canDoAction);
    //draw person, general
    void drawPlayer(QPainter& painter, const Pos& p, Qt::GlobalColor penColor, Qt::GlobalColor brushColor, bool isMe, bool canDoAction, QString id);

    //draw coord
    void drawCoord(QPainter& painter, int x, int y, int fontSize);
    //draw the full map
    void drawMap(QPainter& painter);

    bool initPos(Pos initPos /*QPoint h*/);

    void sendInitPos();

//private slots:
    bool sendAction(); //1-for success, 0-for failure.

    void replay();



    void showHelp();

    void setWindowTitleWithSteps();


    void setTitle(QString title);
    void setStatus(QString status);


private:
    AutoPlayer* getDebugAutoPlayer();

    //! map
    GMap* _gmap;
    //! current player
    Player _player;

    //AG150525
    //! all players (includes 'this')
    std::vector<PlayerInfo*> _playerInfoVec;

    //init
    void init(GMap* gmap);
    void init(Player* pl);

    //! only show map
    bool _onlyShowMap;

    //! choose between showing coords
    char _showCoords;
    //! are coords 0-based?
    bool _coordZeroBased;

    //! give observations by mouse, i.e. not same as we have
    bool _useMouseObs;
    //! position of observation sent to server
    Pos _mouseObs;

    //! previous point where user clicked on map
    Pos _prevClickPos;

    //AG131125
    //! show all players
    bool _showAllPlayers;

    GameConnectorClient* _connector;

    //! allow to click next pos
    bool _allowClickNextPos;

    //! state for editing/debug
    enum WState _wstate;

    //! last mouse button
    Qt::MouseButton _lastMouseButton;


    //for debug
    //! debug mode: test navigation algo's
    bool _debugMode;

    Pos _debugStartPos, _debugEndPos;
    std::vector<Pos> _debugPath;
    Pos _debugNextStep;
    //! debug auto player
    AutoPlayer* _debugAutoPlayer;

    //AG140601: to send to server
    QString _comments, _metaInfo;

    //AG140710
    //! show visib of opponent instead of own player (to make demo screenshots)
    bool _showOppVisib;

    //! main window to show status and title
    HSClientMainWindow* _mainWindow;
    //! store status and title for main window (when set)
    QString _titleMW, _statusMW;
};

#endif // GMAPWIDGET_H
