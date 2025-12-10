#ifndef GPLAYER_H
#define GPLAYER_H

#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string>

///#include <QString>

#include "gmap.h"


using namespace std;






void raytrace(char** m, int x0, int y0, int x1, int y1);

//void gaussian_noise(int num );

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
    static const double SCORE_DHS_FACTOR = 0.4;
    static const double SCORE_RAND_STD = 2;
    static const double SCORE_LESS_RAND_DIST = 3;
    //static const double SCORE_RAND_STD = 4;


    //Player();

    Player(GMap* map=NULL);

    ~Player() {
        //delete [] _action;
        //delete [] _inv;
        delete [] _pm;
    }

    bool move(int action);

    /*!
     * \brief tryMove try to move with action from pos, returns the new Pos. If not possible the
     * new pos has value (-1,-1)
     * \param action
     * \param a
     * \return
     */
    Pos tryMove(int action, Pos pos );


    void setMap(GMap* map);

    Pos Getcurpos() {
        return _curPos;
    }

    Pos Getoppos() {
        return _opPos;
    }


    void setf() {
        _f=1;
    }

    void unsetf() {
        _f=0;
    }

    bool getf() {
        return _f;
    }
    int getnuma() {
        return _numActions;
    }
    int getwin() {
        return _win;
    }

    //ag130228: disabled
    /*
    void calculatevisible(); //calculates the visible map for the player (i.e. Seeker!!)

    void calculateInvisible(); //calculates the invisible cells for the opponent nad stores it in Pos *_inv
    */

    GMap* getMap() {
        return _map;
    }

    int getType() {
        return _type;
    }

    void setType(int n) {
        _type=n;
    }
    void setcurpos(int x, int y) {        
        _curPos.row=x;
        _curPos.col=y;
    }

    void setoppos(int x, int y) {
        _opPos.row=x;
        _opPos.col=y;
    }

    void setOpPos(Pos p) {
        _opPos.set(p);
    }

    void setCurPos(Pos p) {
        _curPos.set(p);
    }

    int getlastaction() {
        return _action;
    }

    void addsmsg() {
        _msgCntServer++;
    }
    void addpmsg() {
        _msgCntPlayer++;
    }
    int getsmsg() {
        return _msgCntServer;
    }
    int getpmsg() {
        return _msgCntPlayer;
    }

    void setwin(int n) {
        _win=n;
        if(_type) cout<<"SEEKER, ";
        else cout<<"HIDER, ";
        if (_win==1)
            cout<<"YOU WIN!!!!!!!!!!!!!!!!!!"<<endl;
        else if (_win==-1)
            cout<<"YOU LOOSE!!!!!!!!!!!!!!!"<<endl;
        else if (_win==0)
            cout<<"IT'S A TIE!!!"<<endl;
    }
    void setusername(string name) {
        _username = name;
    }
    string getusername() {
        return _username;
    }

    void setopusername(string name) {
        _opusername = name;
    }
    string getopusername() {
        return _opusername;
    }

    void initplayer() {
        _action=-1;
        //type remains
        if(!_type) {
            _curPos = _map->getInitialh();
            _opPos = _map->getInitials();
        }
        else {
            _curPos = _map->getInitials();
            _opPos = _map->getInitialh();
        }
        _numActions=0;
        _win=2;
        _msgCntPlayer=0;
        _msgCntServer=0;

    }

    void sethidertype(int n) {

        if (n==2) {
            _type=1;
            _hidertype=-1;
        }
        else {
            _type=0;
            _hidertype= n;
        }
    }

    int gethidertype() {
        return _hidertype;
    }

    /* //ag130228: disabled
    int get_n() {
        return _n;
    }*/

    //ag130228: (was get_inv) get list of invisible points
    //if no params -> use currentPos
    vector<Pos> getInvisiblePoints(int r=-1,int c=-1);

    void getpossiblemoves(); //stores into _pm all the possible actions for the player

    void scoreneighb();//it scores all neighbour cells that could be a future position

    int chooseaction(); //returns the choosen action


    //ag120111: is the opponent visible?
    bool opIsVisible();

    //ag120112: print map and other info
    void printInfo();


protected:


private:

    int _action; //last action//a list of all the actions followed by the Player
    int _type; //

    int _hidertype; //0 for random hider, 1-for intelligent hider, -1 undefined
    Pos _curPos; // current position of player in terms of (x,y)
    Pos _opPos; //current position of opponent in terms of (x,y)
    int _numActions;//number of actions taken untill now

    bool _f; // 1-when the player is ready to sent his msg... that is he has already received previous action
    int _win;//1 when the player won, 0 when brought a tie, -1 when it lost,2-when the game is still on

    GMap* _map;//pointer to a map as is observed by the player.

    int _msgCntServer; //the number of msgs received from the server
    int _msgCntPlayer; //the number of msgs sent from the player

    string _username;
    string _opusername;

    //ag130228: disabled - use new visibility matrix
    //Pos* _inv; // A list with the invisible cells  for the seeker
    //int _n;  // the number of the invisible cells for the seeker

    int _pmnum;
    PosAct *_pm; //a list of the possible actions


    //quint16 _numa;


};


#endif // GPLAYER_H
