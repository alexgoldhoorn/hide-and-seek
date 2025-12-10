#include "randomhider.h"

#include "Utils/generic.h"
#include "hsglobaldata.h"

RandomHider::RandomHider() : AutoHider()
{
    initRandomizer();
    _map = NULL;
}

/*void RandomHider::initBelief(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    _map = gmap;
    _hiderPlayer.setMap(gmap);
    _seekerPlayer.setMap(gmap);
}*/

int RandomHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible) {
    //DEBUG_HS(cout << " takenewaction: "<<a<<endl;);

    int a = -1;

    do {
        _hiderPlayer.setCurPos(hiderPos);
        a = rand() % HSGlobalData::NUM_ACTIONS;
    } while (!_hiderPlayer.move(a));

    return a;
}



Pos RandomHider::getInitPos() {
// it randomly places the hider in one of the invisible for the seeker cells
    assert(_map!=NULL);

    Pos invisPos;
    bool ok=false;

    while(!ok) {

        if(_map->numObstacles()==0) { //if it is a map with no Invisible cells
            //srand((unsigned)time(0));
            int n=rand()%_map->rowCount();
            invisPos.row=n;
            int k=rand()%_map->colCount();
            invisPos.col=k;

            ok=true;
        } else {
            /*DEBUG_HS(cout<<"calculate invisible: "<<flush;);
            _player.calculateInvisible();
            DEBUG_HS(cout<<"ok"<<endl;);

            srand((unsigned)time(0));
            int n=rand()%_player.get_n();
            t = _player.get_inv(n);*/

            Pos base = _map->getBase();
            vector<Pos> invisPoints = _map->getInvisiblePoints(base.row,base.col);

            invisPos = invisPoints[rand()%invisPoints.size()];

            ok=true;
        }


        if (invisPos.equals(_map->getBase())) { // || invisPos.equals(_player.Getoppos())
            ok = false;  //assume seeker starts at base //TODO: should be at a win dist...
        }
        /*if(t.x==_gmap->GetBase().x && t.y==_gmap->GetBase().y)
            ok=false;
        if ( (t.x==_player.Getoppos().x) && (t.y==_player.Getoppos().y) )
            ok=false;*/
    }

    /*_player.setcurpos(t.x,t.y);
    _gmap->setHider(t);
    _gmap->setInitialh(t.x,t.y);

    _player.calculatevisible();*/

    return invisPos;
}
