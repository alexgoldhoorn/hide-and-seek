#include "actionlistgenerator.h"

#include <iostream>
#include <QFile>
#include <QTextStream>

#include "../hsconfig.h"

using namespace std;

ActionListGenerator::ActionListGenerator(GMap *map, QObject *parent) :
    QObject(parent)
{
    _gmap = map;
    _player = new Player(_gmap);
    srand (time(NULL));
}

void ActionListGenerator::generateFile(QString fileName, QString name) {
    cout << "Generating: "<<fileName.toStdString()<<endl;
    //srand((unsigned)time(0));

    //open file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        cout << "ERROR: Could not open file for writing: "<<fileName.toStdString()<<endl;
        exit(-1);
    }
    QTextStream out(&file);

    //write name
    out << name << endl;


    //init at base
    Pos base = _gmap->getBase();

    //set seeker
    _player->setcurpos(base.row,base.col);
    _gmap->setSeeker(base);
    _gmap->setInitials(base.row,base.col);

    _player->sethidertype(Player::HIDER_TYPE_RANDOM);
    _player->setType(1);
    _player->setoppos(base.row,base.col);
    _gmap->setInitialh(base.row,base.col);

    //
    /*_player->setcurpos(0,0);
    _player->setoppos(3,3);
    _gmap->setInitialh(0,0);
    _gmap->setInitials(3,3);*/

    /*
    p->sethidertype(-1);
    p->setType(2);
    p->setcurpos(0,0);
    p->setoppos(3,3);
    map->setInitialh(0,0);
    map->setInitials(3,3);
    */


    /*_player->calculateInvisible();
    //_player->calculatevisible();
    DEBUG_HS(cout << "invisible cells: "<< _gmap->getInvisibleCount()<<endl;);
    DEBUG_HS(cout << " ninv: "<<_player->get_n()<<endl;);*/

    /*_player.calculateInvisible();

    srand((unsigned)time(0));
    int n=rand()%_player.get_n();
    t = _player.get_inv(n);
    ok=1;*/

    _gmap->printMap(base);

    cout <<" getting invis p:"<<flush;
    //ag130405: get invisible points
    vector<Pos> invisPosVector = _player->getInvisiblePoints();

    cout<<invisPosVector.size()<<endl;

#ifdef DEBUG_HS_ON
    cout << "Invisible pos: ";
    for(int i=0;i<invisPosVector.size() /*_player->get_n()*/;i++) {
        Pos p = invisPosVector[i]; // _player->get_inv(i);
        cout << p.toString() << " ";
    }
    cout << endl;
#endif

    //calc init pos
    Pos initPos;
    if (invisPosVector.size()>1) { //_player->get_n()>1) { //_gmap->getInvisibleCount()>1
        //there are more than 1 cell invisible to the seeker from base
        do {
            int n=rand()%invisPosVector.size(); //_player->get_n();
            initPos = invisPosVector[n];  //_player->get_inv(n);
            /*initPos.x = rand() % _gmap->height(); //since this are the rows .. _gmap->width();
            initPos.y = rand() % _gmap->width();*/
        } while (initPos.equals(base) /*initPos.row==base.row && initPos.col==base.col*/ /*|| _gmap->isVisible(initPos.x,initPos.y)*/ ); //find a non-base cell

    } else {
        //at max 1 cell invisible to seeker from base -> find random cell
        do {
            initPos.row = rand() % _gmap->height(); //since this are the rows .. _gmap->width();
            initPos.col = rand() % _gmap->width();
        } while (initPos.equals(base) /*initPos.row==base.row && initPos.col==base.col*/ || _gmap->isObstacle(initPos)); //find a non-base cell
    }

    //write pos to file
    out << initPos.row << "," << initPos.col << endl;

    //now use this init pos
    _player->setcurpos(initPos.row,initPos.col);
    _gmap->setHider(initPos);
    _gmap->setInitials(initPos.row,initPos.col);

    //now generate actions
    Pos p;
    int a = -1;
    /*do {
        p.x = rand() % _gmap->height(); //since this are the rows .. _gmap->width();
        p.y = rand() % _gmap->width(); //since this are the cols .. _gmap->height();
    } while (p.x==base.x && p.y==base.y || _gmap->isObstacle(p.x,p.y));*/
    quint16 i=0;

    //do actions until max actions reached or base reached
    do {
        do { // generate a valid action (ie not crashing)
            a = rand() % 9;
        } while(! _player->MovePlayer(a));
        i++;
        p = _player->Getcurpos();

        //write action
        out << a << endl;
    } while ( !(i>MAX_ACTIONS || p.row==base.row && p.col==base.col) );

    file.close();
    cout << "Written "<<i<<" actions"<<endl;
}








