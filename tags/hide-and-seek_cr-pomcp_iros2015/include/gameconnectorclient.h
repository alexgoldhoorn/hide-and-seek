#ifndef GAMECONNECTORCLIENT_H
#define GAMECONNECTORCLIENT_H

#ifdef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
#define SHS_IN_GC(x)
#else
#ifndef SEEKERHS_H
class SeekerHS;
#endif
#define SHS_IN_GC(x) x
#endif

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <string>

#include "HSGame/gplayer.h"
#include "HSGame/gmap.h"
#include "HSGame/idpos.h"
#include "autoplayer.h"
#include "game.h"
#include "mutex.h"

#include "seekerhsparams.h"
#include "hsglobaldata.h"

#include "Server/hsserverclientcommunication.h"

/*!
 * \brief The GameConnectorClient class This class connects the client to the server.
 * A request is sent at first to the server containing all the parameters, such as the map, player type, name, etc.
 * Once two playeres are connected to the server, it sends the game data to the clients (parameters + map).
 * A first position is sent to the server, and from then on the positions of the opponent is received and sent to the Auto Player (if passed).
 */
class GameConnectorClient : public Game
{
    Q_OBJECT

public:
   /* GameConnectorClient();

    GameConnectorClient(std::string ip, int port, std::string userName, int mapID, int oppType, bool isSeeker, AutoPlayer* autoPlayer=NULL,
                        GMap* mapToSend=NULL
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                        , SeekerHS* seekerHS=NULL
#endif
            );*/

    /*!
     * \brief GameConnectorClient
     * \param params     the parameters
     * \param metaInfo   meta info of the game (such as params) that should be stored in the DB
     * \param comments   comments by user
     * \param autoPlayer the auto player class that is called to get the next action, if passed (i.e. not NULL)
     * \param mapToSend  if no map ID is given, a map can be sent
     * \param seekerHS   the SeekerHS class which is used by the robot, but can be used here, only to test it
     */
    GameConnectorClient(SeekerHSParams *params,
                        QString metaInfo,
                        QString comments,
                        AutoPlayer* autoPlayer=NULL,
                        GMap* mapToSend=NULL
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                        , SeekerHS* seekerHS=NULL
#endif
            );

    ~GameConnectorClient();

    //! set params
    void setParams(std::string ip, int port, std::string userName, int mapID, int oppType, bool isSeeker, AutoPlayer* autoPlayer, GMap* mapToSend);

    //! connect to the server
    bool connectToServer();


    void disconnectFromServer();

    /*!
     * \brief sendAction send new position of player
     * \param obsPos when not NULL this position is sent instead
     * \return
     */
    bool sendAction(const Pos* chosenPos, const Pos* obsPos);

    /*!
     * \brief sendInitPos send init position
     */
    void sendInitPos();

    virtual bool startGame();


    int getGameStatus();

    bool canDoAction();

    bool isMyInitPosSet();

    bool isOppPosSet();

    bool haveServerParamsBeenRead();

    void setExitOnDisconnect(bool exitOnDisconnect);

    void setParams(SeekerHSParams* params);

    GMap* getGMap();

    Player* getPlayer();

    SeekerHSParams* getParams();

    bool isSeeker();

    /*!
     * \brief _getDynamicObstacleVector return vector of dynamic obstacle vector (auto walkers)
     * \return
     */
    std::vector<IDPos> getDynamicObstacleVector();


    //AG140502: manually enter additional tracks
    bool debugAddNewTracksManually;


    /*!
     * \brief sendMessage send a message to another client through the server
     * \param sendTo send message to whom
     * \param messageByteArray message to send     
     * \return
     */
    bool sendMessage(MessageSendTo sendTo, QByteArray& messageByteArray);

    //AG150217
    //! send a stop request to server
    void sendStopRequest();


signals:
    /*!
     * \brief serverParamsRead signals that the params (map and game params) have been read from the server.
     */
    void serverParamsRead();

    /*!
     * \brief serverUpdateReceived informs about newly received info from server
     * \param opRow
     * \param opCol
     * \param gameStatus
     */
    void serverUpdateReceived(int opRow, int opCol, int gameStatus);

    /*!
     * \brief serverDisconnected signal that server disconnected
     */
    void serverDisconnected();


protected slots:
    //! info received from server
    void serverReadyRead();

    //! server disconnected
    void onServerDisconnect();



protected:

    //! send init info to server
    void sendInfoToServer();

    //! read init info from server
    void readInitInfoFromServer(QDataStream& in);

    //AG150120
    //! receive server update
    void readServerUpdate(QDataStream& in);

    //AG150120
    //! read message
    void readMessage(QDataStream& in);

    //AG150202
    //! send the goals of the robots and the belief to the server (stored in _player)
    void sendRobotGoalsAndBeliefs(); //const std::vector<Pos>& goalPosesVec, const std::vector<double>& goalPosesBeliefVec);

    //! read the other seeker's goals and beliefs
    void readSeeker2GoalsAndBeliefs(QDataStream &in);

    //AG150214
    //! send the multi seeker chosen pos to the server (for logging)
    //AG150216: do in 'sendaction' (only if 2seekers & isSeeker)
    //void sendMultiSeekerChosenPos(const Pos& chosenPos);

    virtual void moveToThreadVariables(QThread* thread);

    void init();


private:
    //map
    GMap* _gmap;
    //current player
    Player* _player;

    //init
    void init(GMap* gmap);
    void init(Player* pl);

    //! connection to server
    QTcpSocket *_tcpSocket;

    //! server ip
    /*QString _ip;
    //! server port
    int _port;
    //! chosen map id
    int _mapID; //the map that the user chose at the popup. 0-4 : 5 possible maps.
    //! chosen opponent type
    int _opp; //the opponent the user chose. [0-3]. 4 possible opponents. 0-a human, 1-a dummy hider,2- an intelligent hider, 3-SeekroBot.
    //! current player is seeker
    bool _isSeeker;*/

    //! own init. pos is set
    bool _myInitPosSet;
    //! init. pos of opponent is set
    bool _oppInitPosSet;

    //! temp file for actions
    //QString _actionFile;

    //! game status (running or won or tie)
    int _gameStatus;

    //! number of messages sent
    unsigned int _numMsgSent;

    //! numb of messages received
    unsigned int _numMsgReceived;

    //! are you allowed to do an action or do you have to wait
    bool _canDoAction;

    //! max actions
    /*int _maxActions;

    //! win distance
    int _winDist;*/

    //! params from server received flag
    bool _serverParamsReceived;

    //! the player used
    AutoPlayer* _autoPlayer;

    //! bool exit on disconnect
    bool _exitOnDisconnect;

    //! map to send
    GMap* _mapToSend;

    //! seekerHS params
    SeekerHSParams* _params;

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    //! seeker HS
    SeekerHS* _seekerHS;
#endif

    //AG140403: added last position to deduce action
    //! last pos
    Pos _lastPos;

    //AG140506: vector of dynamical obstacle (or persons) positions
    //! dynamical obstacles
    std::vector<IDPos> _dynObstPosVec;

    //! mutex for getting dyn obst pos vector
    CMutex _dynObstPosVecMutex;

    //AG140531
    //! meta data to be sent to the server
    QString _metaInfo;

    //! comments to be sent to server
    QString _comments;

    //! receive noise in pos
    bool _receiveNoisyPos;

};

#endif // GAMECONNECTORCLIENT_H
