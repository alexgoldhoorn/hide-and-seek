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
#include <vector>

#include "HSGame/gplayer.h"
#include "HSGame/gmap.h"
#include "HSGame/idpos.h"
#include "Base/autoplayer.h"
#include "Base/game.h"
#include "mutex.h"

#include "Base/seekerhsparams.h"
#include "Base/hsglobaldata.h"

#include "Server/hsserverclientcommunication.h"

#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
#include "Base/posxy.h"
#endif

/*!
 * \brief The GameConnectorClient class connects the client to the server.
 * A request is sent at first to the server containing all the parameters (sendInfoToServer), such as the map, player type, name, etc.
 * Previous version:
 * Once two players are connected to the server, it sends the game data to the clients (parameters + map).
 * A first position is sent to the server, and from then on the positions of the opponent is received and sent to the Auto Player (if passed).
 * New version:
 * The seeker client should send a flag indicating that as soon as the hider connects (and at least 1 seeker is connected), the game should start.
 * (This is done for the multi agent environment in which the amount of players is unknown.) First the initial position is sent, and then
 * position and action updates are sent to the server. Before a new position is sent the server should send all the positions (and noisy positions)
 * of the other players.
 */
class GameConnectorClient : public Game
{
    Q_OBJECT

public:
    /*!
     * \brief GameConnectorClient constructor for 1 autoplayer
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

    //AG150525
    /*!
     * \brief GameConnectorClient constructor for no autoplayer, only 'manual' through playerInfo
     * \param params     the parameters
     * \param metaInfo   meta info of the game (such as params) that should be stored in the DB
     * \param comments   comments by user
     * \param mapToSend  if no map ID is given, a map can be sent
     */
    GameConnectorClient(SeekerHSParams *params,
                        QString metaInfo,
                        QString comments,
                        Player* thisPlayer,
                        GMap* mapToSend=NULL
            );

    //AG150518
    /*!
     * \brief GameConnectorClient constructor for several autoplayers
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
                        std::vector<AutoPlayer*> autoPlayerVec,
                        GMap* mapToSend=NULL
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
                        , SeekerHS* seekerHS=NULL
#endif
            );

    ~GameConnectorClient();

    //AG150518: depricated
    // set params
    //void setParams(std::string ip, int port, std::string userName, int mapID, int oppType, bool isSeeker, AutoPlayer* autoPlayer, GMap* mapToSend);

    //! connect to the server
    bool connectToServer();

    //! disconnect from server
    void disconnectFromServer();

    /*!
     * \brief sendAction send new position of player
     * \param obsPos when not NULL this position is sent instead
     * \return
     */
    //bool sendAction(const Pos* chosenPos, const Pos* obsPos);

    /*!
     * \brief sendPos send the poses to the server. The fakepos is sent instead if not NULL (for debugging).
     * \param fakePos (optional, default NULL) the positions sent instead of the real postion (if not NULL)
     * \return
     */
    bool sendPos(const Pos* fakePos=NULL);

    /*!
     * \brief sendInitPos send init position
     */
    void sendInitPos();

    virtual bool startGame();

    //! return game status
    int getGameStatus();

    //! is the player allowed to do an action now
    bool canDoAction();

    //AG150518: in player
    /*bool isMyInitPosSet();

    bool isOppPosSet();*/

    //! have the server params. been received
    bool haveServerParamsBeenRead();

    //! exit client on disconnect
    void setExitOnDisconnect(bool exitOnDisconnect);

    //! set params
    void setParams(SeekerHSParams* params);

    //! return the used GMap
    GMap* getGMap();

    // get current player
    //Player* getPlayer();

    //! return params
    SeekerHSParams* getParams();

    //! is the player a seeker
    bool isSeeker();

    /*!
     * \brief _getDynamicObstacleVector return vector of dynamic obstacle vector (auto walkers)
     * \return
     */
    std::vector<IDPos> getDynamicObstacleVector();

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

    //AG140502
    //! manually enter additional tracks (for debugging)
    bool debugAddNewTracksManually;

    /*!
     * \brief getAutoPlayer get the autoplayer based on the index
     * Note: the index is set after the initial 'game' parameters have been received from the server
     * \param id
     * \return
     */
    AutoPlayer* getAutoPlayer(int id);

    /*!
     * \brief getThisPlayerInfo return this player info if there are no autoplayers (e.g. only 'manual')
     * \return
     */
    PlayerInfo* getThisPlayerInfo();

    //! return the player info list for all players (includes also 'this' connection's players)
    std::vector<PlayerInfo*> getPlayerInfoVec();

signals:
    /*!
     * \brief serverParamsRead signals that the params (map and game params) have been read from the server.
     */
    void serverParamsRead();

    /*!
     * \brief serverUpdateReceived informs about newly received info from server, the specific data (e.g. new locations)
     *      have to be queried to this class.
     * \param gameStatus
     */
    void serverUpdateReceived(int gameStatus);

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

    //! send init info/request to server
    void sendInfoToServer();

    //! read init info from server
    void readInitInfoFromServer(QDataStream& in);

    //AG150120
    //! receive server update
    void readServerUpdate(QDataStream& in);

    //AG150526
    //! write player update (action, pos, reward, etc)
    void writePlayerUpdate(QDataStream& out, PlayerInfo* playerInfo, const Pos* fakePos);

    //AG150522: create sub functions for readServerUpdate
    //! init the belief of the auto players
    void initBelief();
    //! calculate next poses for the seeker(s) (or for the multi players: calculate the potential locations)
    void calcNextPosSeeker();
    //! calculate next poses for the agent if it is a hider
    void calcNextPosHider();

    //AG150120
    //! read message
    void readMessage(QDataStream& in);

    //AG150202
    //! send the goals of the robots and the belief to the server (stored in _player)
    void sendRobotGoalsAndBeliefs(); //const std::vector<Pos>& goalPosesVec, const std::vector<double>& goalPosesBeliefVec);

    //AG150522
    //! send the highest beliefs and the points (stored in _player)
    void sendHBs();

    //! read the other seeker's goals and beliefs
    void readSeeker2GoalsAndBeliefs(QDataStream &in);

    //AG150519
    //! read the other seeker's highest belief points and beliefs
    void readSeekerHB(QDataStream &in);

    //AG150214
    // send the multi seeker chosen pos to the server (for logging)
    //AG150216: do in 'sendaction' (only if 2seekers & isSeeker)
    //void sendMultiSeekerChosenPos(const Pos& chosenPos);

    virtual void moveToThreadVariables(QThread* thread);

    //! init vars
    void init();

private:
#ifndef DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
    /*!
     * \brief getSeerkHSPosXYForAll convert the current position of the seeker, its observation, and other seeker poses and observations
     * to PosXY. This is only used to test the SeekerHS class
     * \param thisSeekerPosXY
     * \param thisSeekerHiderObsPosXY
     * \param otherSeekerPosXYVec
     * \param otherSeekerHiderObsPosXYVec
     */
    void getSeerkHSPosXYForAll(PosXY& thisSeekerPosXY, PosXY& thisSeekerHiderObsPosXY, std::vector<PosXY>& otherSeekerPosXYVec,
                               std::vector<PosXY>& otherSeekerHiderObsPosXYVec);
#endif

    //!map
    GMap* _gmap;

    //!current player
    //Player* _player;

    //!init
    void init(GMap* gmap);
    void init(Player* pl);

    //! connection to server
    QTcpSocket *_tcpSocket;

    //AG150515: should be in Player.PlayerInfo
    /*//! own init. pos is set
    bool _myInitPosSet;
    //! init. pos of opponent is set
    bool _oppInitPosSet;*/


    //! game status (running or won or tie)
    int _gameStatus;
    //! number of messages sent
    unsigned int _numMsgSent;

    //! numb of messages received
    unsigned int _numMsgReceived;

    //! are you allowed to do an action or do you have to wait
    bool _canDoAction;

    //! params from server received flag
    bool _serverParamsReceived;

    //AG150519
    //! used to indicate if the init positions have been received
    bool _initBeliefDone;

    //AG150515: now a vector (since client can contain more)
    //! the player used
    std::vector<AutoPlayer*> _autoPlayerVec;

    //! the player used, map indexed on id
    std::map<quint8,AutoPlayer*> _autoPlayerMap;

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

    //AG150515: in Player.PlayerInfo
    //AG140403: added last position to deduce action
    // last pos
    //Pos _lastPos;

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

    //AG150515: by default also sends noisy poses
    //! receive noise in pos
    //bool _receiveNoisyPos;

    //AG15015
    //! list of all players: should contain all info, indexed on the id
    //! note: the playerInfo objects of the current clients refer to the AutoPlayer.playerInfo variable
    std::vector<PlayerInfo*> _playerInfoVec;

    //AG150520
    //! player hider
    PlayerInfo* _hiderPlayer;

    //AG150602
    //! player seeker
    PlayerInfo* _seeker1Player;
    //AG150721: other seeker
    PlayerInfo* _seeker2Player;

    //AG150525
    //! this player's info
    //! Only used when there is no autoplayer
    Player* _thisPlayer;
};

#endif // GAMECONNECTORCLIENT_H
