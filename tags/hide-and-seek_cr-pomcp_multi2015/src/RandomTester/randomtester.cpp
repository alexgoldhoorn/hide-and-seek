#include "RandomTester/randomtester.h"

#include <iostream>
#include <cmath>
#include <vector>


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
    Pos base = _gmap->getBase();

    //set seeker
    _player->setcurpos(base.row,base.row);
    _gmap->setSeeker(base);
    _gmap->setInitials(base.row,base.row);

    _player->sethidertype(Player::HIDER_TYPE_RANDOM);
    _player->setType(1);
    _player->setoppos(base.row,base.row);
    _gmap->setInitialh(base.row,base.row);

    //_player->calculateInvisible();
    //_player->calculatevisible();
    vector<Pos> invisPos = _gmap->getInvisiblePoints(base);
    ///cout << "invisible cells: "<< _invisPos.size() /* gmap->getInvisibleCount()*/<<endl;
    //cout << " ninv: "<<_player->get_n()<<endl;

    _gmap->printMap(true);

    cout << "Invisible pos: ";
    /*for(int i=0;i<_player->get_n();i++) {
        Pos p = _player->get_inv(i);
        cout << "("<<p.row<<","<<p.row<<") ";
    }*/
    for (vector<Pos>::iterator it=invisPos.begin(); it!=invisPos.end(); it++) {
        cout << it->toString()<<" ";
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
    Pos base = _gmap->getBase();
    Pos initPos;
/* TODO this part has to be updated
    if (_player->get_n()>1) { //_gmap->getInvisibleCount()>1
        //there are more than 1 cell invisible to the seeker from base
        do {
            int n=rand()%_player->get_n();
            initPos = _player->get_inv(n);
        } while (initPos.row==base.row && initPos.row==base.row  ); //find a non-base cell

    } else {
        //at max 1 cell invisible to seeker from base -> find random cell
        do {
            initPos.row = rand() % _gmap->height(); //since this are the rows .. _gmap->width();
            initPos.row = rand() % _gmap->width();
        } while (initPos.row==base.row && initPos.row==base.row || _gmap->isObstacle(initPos.row,initPos.row)); //find a non-base cell
    }
*/
    //write pos to file
    //out << initPos.x << "," << initPos.y << endl;

    //now use this init pos
    _player->setcurpos(initPos.row,initPos.row);
    _gmap->setHider(initPos);
    _gmap->setInitials(initPos.row,initPos.row);

    //now generate actions
    Pos p= _player->Getcurpos();
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
            p = _player->tryMove(a,p);
        } while(!p.isSet());
        i++;
        p = _player->Getcurpos();

        //write action
        //out << a << endl;
        nSteps++;
    } while ( !(i>MAX_ACTIONS || p.row==base.row && p.row==base.row) );

    //file.close();
    //cout << "Written "<<i<<" actions"<<endl;
    return nSteps;
}








