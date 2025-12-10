
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>

#include "HSGame/gplayer.h"

#include "Utils/generic.h"
#include "Base/hsglobaldata.h"

using namespace std;


Player::Player(GMap* map) {
    DEBUG_PLAYER(cout<<"A player is constructed"<<endl;);

    //AG150602: can be
    //assert(map!=NULL);

    _map = map;

    /*_numActions=0;
    _action = -1; //undefined
    _type =-1;*/

    /*_curPos.row = -1; //AG131219: default value
    _curPos.col= -1;
    _oppPos.row =-1;
    _oppPos.col =-1;*/
    //_hidertype = -1;
}

Player::~Player() {
}

void Player::setMap(GMap* map) {
    if (_map!=NULL)
        cout << "Player::setMap: WARNING a map was already set"<<endl;
    _map = map;
}

bool Player::move(int action) {
    assert(action>=0 && action<HSGlobalData::NUM_ACTIONS);


    //calculate the new pos
    Pos newPos = _map->tryMove(action, playerInfo.currentPos);//player being in position position takes action action and moves to a new position

    if (!newPos.isSet())  {//if the movement was not done
        DEBUG_PLAYER(cout<< "Player's move did not happen"<<endl;);
        //_f=1;
        return false;
    }

    //if movement done should be ok
    assert(_map->isPosInMap(newPos));

    //add the action in the actionlist of the player
    playerInfo.lastAction=action;
    //_curPos = newPos;
    playerInfo.currentPos = newPos;
    //_f= 0;
    //_numActions++;
    playerInfo.numberActions++;
    // cout<<"the new position of player, after taking action="<<action<<",  is = ("<<n.x<<","<<n.y<<")"<<endl;

    return true;
}

void Player::printInfo() {
    //note: x=row,y=col ..

    //cout << " type="<<_type<</*";hiderType="<<_hidertype<<*/";curpos="<<_curPos.toString()<<"; oppos="<<_player2Pos.toString()
    //     <<";#actions="<<_numActions<<";lastAction="<<_action<<endl;


    _map->printMap(playerInfo.currentPos); //ag130301: other syntax printmap

}

/*vector<Pos> Player::getInvisiblePoints(int r, int c) {
    if (r<0) r=_curPos.row;
    if (c<0) c=_curPos.col;

    return _map->getInvisiblePoints(r,c);
}
*/

/*void gaussian_noise(int num ){

        double *noise=NULL;
        int i;
        int ndata=num;
        unsigned int seed;

        if(argc!=2)
        {
            fprintf(stderr, "usage : random 5");
            exit(0);
        }

        ndata = atoi(argv[1]);

        if (-1 == (seed = (unsigned int) time((time_t *) NULL)))
        {
            fprintf(stderr, "time() failed to set seed");
            exit(0);
        }

        srannor(seed);  //seed random number generator
        noise = (double*) calloc (ndata, sizeof(double));
        for(i=0; i<ndata; i++)  //create random data
        {
            // Compute noise vector elements in [-1, 1]
            // GAUSS METHOD. frannor gives elements in N(0,1)--ie. pos & negs
            noise[i] = (double) frannor();
        }

        for(i=0; i<ndata; i++)
            printf("%3.2f   ", noise[i]);
        printf("\n");

        free(noise);  //free allocated memory
        return(1);

}
*/

/*
Pos Player::getCurPos() {
    return _curPos;
}

Pos Player::getPlayer2Pos() {
    return _player2Pos;
}

Pos Player::getHiderObsWNoise() {
    return _hiderObsWNoise;
}

Pos Player::getPlayer3Pos() {
    return _player3Pos;
}

Pos Player::getHiderObs2WNoise() {
    return _hiderObs2WNoise;
}*/

GMap* Player::getMap() {
    return _map;
}
/*
int Player::getType() {
    return _type;
}

void Player::setType(int n) {
    _type=n;
}
void Player::setCurPos(int x, int y) {
    _curPos.set(x,y);
}

void Player::setCurPos(double x, double y) {
    _curPos.set(x,y);
}

void Player::setCurPos(Pos p) {
    _curPos = p;
}

void Player::setPlayer2Pos(int x, int y) {
    _player2Pos.set(x,y);
}

void Player::setPlayer2Pos(double x, double y) {
    _player2Pos.set(x,y);
}

void Player::setPlayer2Pos(Pos p) {
    _player2Pos = p;
}

void Player::setHiderObsWNoise(Pos p) {
    _hiderObsWNoise = p;
}

void Player::setPlayer3Pos(int x, int y) {
    _player3Pos.set(x,y);
}

void Player::setPlayer3Pos(double x, double y) {
    _player3Pos.set(x,y);
}

void Player::setPlayer3Pos(Pos p) {
    _player3Pos = p;
}

void Player::setHiderObs2WNoise(Pos p) {
    _hiderObs2WNoise = p;
}

int Player::getLastAction() {
    return _action;
}

void Player::setNumActions(int n) {
    _numActions = n;
}

int Player::getNumActions() {
    return _numActions;
}

void Player::incrNumActions() {
    _numActions++;
}

void Player::setUsername(string name) {
    _username = name;
}
string Player::getUsername() {
    return _username;
}

void Player::setPlayer2Username(string name) {
    _player2Username = name;
}
string Player::getPlayer2Username() {
    return _player2Username;
}

void Player::setPlayer3Username(string name) {
    _player3Username = name;
}
string Player::getPlayer3Username() {
    return _player3Username;
}*/

void Player::initPlayer() {
    playerInfo.clear();
    //_action=-1;
    //type remains
    /*if(!_type) {
        _curPos = _map->getInitialh();
        _opPos = _map->getInitials();
    }
    else {
        _curPos = _map->getInitials();
        _opPos = _map->getInitialh();
    }*/
    //_numActions=0;
    /*_win=2;
    _msgCntPlayer=0;
    _msgCntServer=0;*/
}


//ag131209: set last action in order to have it sent by gameconnector
/*void Player::setLastAction(int a)  {
    //_action = a;
    playerInfo.lastAction = a;
}*/

bool Player::isSet() {
    //the current player position should be set
    return playerInfo.currentPos.isSet();
}
