#include "randomtester.h"

#include <iostream>
#include <cmath>


using namespace std;

RandomTester::RandomTester(GMap *map, QObject *parent) :
    QObject(parent)
{
    _gmap = map;
    _player = new Player(_gmap);
}

double RandomTester::testMap(int n, double &std) {
    cout << "Testing: "<<endl;
    srand((unsigned)time(0));



    //init at base
    Pos base = _gmap->GetBase();

    //set seeker
    _player->setcurpos(base.x,base.y);
    _gmap->setSeeker(base);
    _gmap->setInitials(base.x,base.y);

    _player->sethidertype(Player::HIDER_TYPE_RANDOM);
    _player->setType(1);
    _player->setoppos(base.x,base.y);
    _gmap->setInitialh(base.x,base.y);

    _player->calculateInvisible();
    //_player->calculatevisible();
    cout << "invisible cells: "<< _gmap->getInvisibleCount()<<endl;
    cout << " ninv: "<<_player->get_n()<<endl;

    _gmap->printMap(true);

    cout << "Invisible pos: ";
    for(int i=0;i<_player->get_n();i++) {
        Pos p = _player->get_inv(i);
        cout << "("<<p.x<<","<<p.y<<") ";
    }
    cout << endl;

    cout << "Starting:"<<endl;

    QList<int> numList;
    double sum = 0;

    for(int i=0;i<n;i++) {
        cout << "Iteration "<<i<<endl;
        int p = testMapOnce();
        sum += p;
        numList.push_back(p);
    }

    //calc avg
    double mean = sum/n;
    std=0;
    //calc std
    for(int i=0;i<n;i++) {
        std += pow(numList[i]-mean,2);
    }
    std = sqrt(std/n);
    return mean;

}

int RandomTester::testMapOnce() {
    //calc init pos
    Pos base = _gmap->GetBase();
    Pos initPos;
    if (_player->get_n()>1) { //_gmap->getInvisibleCount()>1
        //there are more than 1 cell invisible to the seeker from base
        do {
            int n=rand()%_player->get_n();
            initPos = _player->get_inv(n);
            /*initPos.x = rand() % _gmap->height(); //since this are the rows .. _gmap->width();
            initPos.y = rand() % _gmap->width();*/
        } while (initPos.x==base.x && initPos.y==base.y /*|| _gmap->isVisible(initPos.x,initPos.y)*/ ); //find a non-base cell

    } else {
        //at max 1 cell invisible to seeker from base -> find random cell
        do {
            initPos.x = rand() % _gmap->height(); //since this are the rows .. _gmap->width();
            initPos.y = rand() % _gmap->width();
        } while (initPos.x==base.x && initPos.y==base.y || _gmap->isObstacle(initPos.x,initPos.y)); //find a non-base cell
    }

    //write pos to file
    //out << initPos.x << "," << initPos.y << endl;

    //now use this init pos
    _player->setcurpos(initPos.x,initPos.y);
    _gmap->setHider(initPos);
    _gmap->setInitials(initPos.x,initPos.y);

    //now generate actions
    Pos p;
    int a = -1;
    /*do {
        p.x = rand() % _gmap->height(); //since this are the rows .. _gmap->width();
        p.y = rand() % _gmap->width(); //since this are the cols .. _gmap->height();
    } while (p.x==base.x && p.y==base.y || _gmap->isObstacle(p.x,p.y));*/
    quint16 i=0;

    int nSteps=0;

    //do actions until max actions reached or base reached
    do {
        do { // generate a valid action (ie not crashing)
            a = rand() % 9;
        } while(! _player->MovePlayer(a));
        i++;
        p = _player->Getcurpos();

        //write action
        //out << a << endl;
        nSteps++;
    } while ( !(i>MAX_ACTIONS || p.x==base.x && p.y==base.y) );

    //file.close();
    //cout << "Written "<<i<<" actions"<<endl;
    return nSteps;
}








