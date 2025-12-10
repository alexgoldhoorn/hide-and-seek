#ifndef GPLAYER_H
#define GPLAYER_H

#include <string>

#include "HSGame/gmap.h"

#include "Base/playerinfo.h"


/*!
 * \brief The Player class represents the player.
 * AG150515: all meta info is now stored in the PlayerInfo object, since this is the same structure used by the server.
 */
class Player
{
public:

    //ag120412: types as defined prev by Chryso
    // TODO: IMPROVE + RECHECK -> make only 1 TYPE and consistently!!!!!
    static const int HIDER_TYPE_UNKNOWN = -1;
    static const int HIDER_TYPE_RANDOM = 0;
    static const int HIDER_TYPE_SMART = 1;

    static const int TYPE_HIDER = 0;
    static const int TYPE_HUMAN = 1;

    //AG130206: score parameters
    static constexpr double SCORE_DHS_FACTOR = 0.4;
    static constexpr double SCORE_RAND_STD = 2;
    static constexpr double SCORE_LESS_RAND_DIST = 3;
    //const double SCORE_RAND_STD = 4;

    /*!
     * \brief Player create player struct
     * \param map
     */
    Player(GMap* map=NULL);

    ~Player();

    /*!
     * \brief move move the player
     * \param action
     * \return
     */
    bool move(int action);

    //! set map
    void setMap(GMap* map);

    //AG150515: use PlayerInfo
    /*Pos getCurPos();

    Pos getPlayer2Pos();

    Pos getHiderObsWNoise();

    Pos getPlayer3Pos();

    Pos getHiderObs2WNoise();*/

    //! get map
    GMap* getMap();

    //AG150515: use PlayerInfo
    /*int getType() ;

    void setType(int n);

    void setCurPos(int x, int y);

    void setCurPos(double x, double y);

    void setCurPos(Pos p);

    void setPlayer2Pos(int x, int y);

    void setPlayer2Pos(double x, double y);

    void setPlayer2Pos(Pos p);

    void setHiderObsWNoise(Pos p);

    void setPlayer3Pos(int x, int y);

    void setPlayer3Pos(double x, double y);

    void setPlayer3Pos(Pos p);

    void setHiderObs2WNoise(Pos p);


    int getLastAction() ;

    void setNumActions(int n) ;

    int getNumActions() ;

    void incrNumActions();

    void setUsername(std::string name);
    std::string getUsername();

    void setPlayer2Username(std::string name);
    std::string getPlayer2Username();

    void setPlayer3Username(std::string name);
    std::string getPlayer3Username();*/

    //! init player
    void initPlayer() ;

    //ag131209: set last action in order to have it sent by gameconnector
    //void setLastAction(int a) ;

    //ag120112
    //! print map and other info
    void printInfo();

    //AG150526
    //! check if the current player is set
    bool isSet();


    //AG150515: put here to make consistently use of the same structures in the server and client
    //! player info
    PlayerInfo playerInfo;

    //AG150515: use PlayerInfo
    /*//AG150202
    //! vector of goal poses (multi seekers only)
    std::vector<Pos> goalPosesVec;
    //! vector of beliefs for goal poses (multi seekers only)
    std::vector<double> goalPosesBelVec;*/




protected:


private:

    //AG150515: use PlayerInfo
    /*// last action//a list of all the actions followed by the Player
    int _action;
    int _type;

    //! current position of player in terms of (x,y)
    Pos _curPos;
    //! current position of opponent (player 2) in terms of (x,y)
    Pos _player2Pos;
    //AG140606
    //! opp pos w. noise
    Pos _hiderObsWNoise;
    //AG150114: second seeker (?)
    //! current position of opponent (player 3) in terms of (x,y)
    Pos _player3Pos;
    //! opp pos w. noise
    Pos _hiderObs2WNoise;
    //! number of actions taken untill now
    int _numActions;

    std::string _username;
    std::string _player2Username;
    std::string _player3Username;*/


    //! pointer to a map as is observed by the player.
    GMap* _map;
};


#endif // GPLAYER_H
