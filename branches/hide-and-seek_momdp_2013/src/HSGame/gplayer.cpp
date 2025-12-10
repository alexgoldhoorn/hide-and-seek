//#include "../HSGui/gmapwidget.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>

#include "gplayer.h"

#include "../Utils/generic.h"
#include "../hsglobaldata.h"


Player::Player(GMap* map) {
    DEBUG_PLAYER(cout<<"A player is constructed"<<endl;);
    //_type = 0;//hider

    //construct the playerś map

    if (map==NULL)
        _map=new GMap();
    else
        _map = map;

    _msgCntServer=0; // 0 msg sent or received
    _msgCntPlayer=0;
    _f=0; //player needs to read first
    _numActions=0;
    _win=2;
    _action = -1; //undefined
    _type =-1;
    _curPos.row = -1;
    _curPos.col= -1;
    _opPos.row =-1;
    _opPos.col =-1;
    _hidertype = -1;

    //_inv = new Pos[1600]; //the array that keeps the invisible cells.
    //AG TODO: more nice way to handle big maps (DYNAMICALLY!!)
    //_n=0;
    _pm = new PosAct[10]; //the array that keeps all the neigbour cells and their scores(for the decision on the next move)
    _pmnum=0;
}

void Player::setMap(GMap* map) {
    //if (_map != NULL) delete _map;
    _map = map;
}

bool Player::move(int action) {
    /*if(action<0 || action>8) {
        DEBUG_PLAYER(cout << "the action chosen is not valid! Try again! " << endl;);
        _f=1;
        return false;
    }*/
    //AG130311:asserts
    assert(action>=0 && action<HSGlobalData::NUM_ACTIONS);

    // the curent position of the player
    Pos position;
    //DEBUG_PLAYER(cout << "the action chosen is  valid! " << action <<endl;);
    position=this->Getcurpos();

    //calculate the new pos
    Pos newPos = this->tryMove(action,position);//player being in position position takes action action and moves to a new position

    if (newPos.row==-1)  {//if the movement was not done
        DEBUG_PLAYER(cout<< "Player's move did not happen"<<endl;);
        _f=1;
        return false;
    }

    //if movement done should be ok
    assert(newPos.row>=0 && newPos.row<_map->rowCount() && newPos.col>=0 && newPos.col<_map->colCount());

    //add the action in the actionlist of the player
    _action=action;
    _curPos.row = newPos.row;
    _curPos.col = newPos.col;
    //_f= 0;
    _numActions++;
    // cout<<"the new position of player, after taking action="<<action<<",  is = ("<<n.x<<","<<n.y<<")"<<endl;

    /* //ag130228: disabled
    char b, a;
    b=_map->getVisibility(_opPos.x,_opPos.y); //visibility of opponent before move
    this->calculatevisible(); //calculate it again according to current position
    a=_map->getVisibility(_opPos.x,_opPos.y); //visibility of opponent after move

    if(b == 0 && a== 1) {
        //cout<<"the opponent was not visible before that move and now he is"<<endl;
        _map->deleteItem(_opPos.x, _opPos.y);
    }    
    */

    return true;
}





Pos  Player::tryMove(int action, Pos pos) {
    Pos newPos;
    newPos.row=newPos.col=-1; //ag120418: init
    int rows = _map->rowCount();
    int cols = _map->colCount();


    if (!(pos.row>=0 && pos.row<rows && pos.col>=0 && pos.col<cols)) {
        cout << "ERROR mvplayer: rows="<<rows<<",cols="<<cols<<", pos="<<pos.toString()<<endl;
    }
    //AG130311: asserts
    assert(pos.row>=0 && pos.row<rows && pos.col>=0 && pos.col<cols);
    assert(action>=0 && action<HSGlobalData::NUM_ACTIONS);

    //first make sure that indeed the current pos of the player agrees with the world state
    /*if(_gmap->getItem(a.x,a.y)!= 8) {
        cout<<"The world has wrong position of the player"<<endl;
        n.x=n.y=-1;
        return n;
    }*/

    //cout<<"Inside world´s function mvPlayer!"<<_gmap->getItem(x,y)<<endl;
    //cout<<rows<<cols<<endl;

    switch(action) {
    case HSGlobalData::ACT_N: {
        newPos.row=pos.row-1;
        newPos.col=pos.col;
        if(newPos.row<0) {
            DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        if(_map->isObstacle(newPos)) {
            DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        /*if(_map->getItem(n.x,n.y)== 2) {
            DEBUG_PLAYER(cout<<"back to the base..."<<endl;);
        }*/
        ///_map->xcoChangeMap(a,n); //ag130311: disabled
    }
    break; // move north
    case HSGlobalData::ACT_NE: {
        newPos.row=pos.row-1;
        newPos.col=pos.col+1;
        if( (newPos.row<0) ||(newPos.col>=cols) ) {
            DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        if(_map->isObstacle(newPos)) {
            DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        //DEBUG_PLAYER(if(_map->getItem(n.x,n.y)== 2) cout<<"back to the base..."<<endl;);
        ///_map->xcoChangeMap(a,n);
    }
    break; // move north-east
    case HSGlobalData::ACT_E: {
        newPos.row=pos.row;
        newPos.col=pos.col+1;
        if(newPos.col>=cols) {
            DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        if(_map->isObstacle(newPos)) {
            DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        //if(_map->getItem(n.x,n.y)== 2) DEBUG_PLAYER(cout<<"back to the base..."<<endl;);
        ///_map->xcoChangeMap(a,n);
    }
    break; // move east
    case HSGlobalData::ACT_SE: {
        newPos.row=pos.row+1;
        newPos.col=pos.col+1;
        if( (newPos.row>=rows) ||(newPos.col>=cols) ) {
            DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        if(_map->isObstacle(newPos)) {
            DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        //DEBUG_PLAYER(if(_map->getItem(n.x,n.y)== 2) cout<<"back to the base..."<<endl;);
        ///_map->xcoChangeMap(a,n);
    }
    break; // move east-south
    case HSGlobalData::ACT_S: {
        newPos.row=pos.row+1;
        newPos.col=pos.col;
        if( newPos.row>=rows ) {
            DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        if(_map->isObstacle(newPos)) {
            DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        //DEBUG_PLAYER(if(_map->getItem(n.x,n.y)== 2) cout<<"back to the base..."<<endl;);
        ///_map->xcoChangeMap(a,n);
    }
    break; // move south
    case HSGlobalData::ACT_SW: {
        newPos.row=pos.row+1;
        newPos.col=pos.col-1;
        if( (newPos.row>=rows) ||(newPos.col<0) ) {
            DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        if(_map->isObstacle(newPos)) {
            DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        //DEBUG_PLAYER(if(_map->getItem(n.x,n.y)== 2) cout<<"back to the base..."<<endl;);
        ///_map->xcoChangeMap(a,n);
    }
    break; // move south-west
    case HSGlobalData::ACT_W: {
        newPos.row=pos.row;
        newPos.col=pos.col-1;
        if(newPos.col<0) {
            DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        if(_map->isObstacle(newPos)) {
            DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        //if(_map->getItem(n.x,n.y)== 2) DEBUG_PLAYER(cout<<"back to the base..."<<endl;);
        ///_map->xcoChangeMap(a,n);
    }
    break; // move west
    case HSGlobalData::ACT_NW: {
        newPos.row=pos.row-1;
        newPos.col=pos.col-1;
        if( (newPos.row<0) ||(newPos.col<0) ) {
            DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        if(_map->isObstacle(newPos)) {
            DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
            newPos.row=newPos.col=-1;
            break;
        }
        //DEBUG_PLAYER(if(_map->getItem(n.x,n.y)== 2) cout<<"back to the base..."<<endl;);
        ///_map->xcoChangeMap(a,n);
    }
    break; // move west-north
    case HSGlobalData::ACT_H: {
        DEBUG_PLAYER(cout<<"the position of the player is: ("<<pos.row<<", "<<pos.col<<")"<<endl;);
        newPos.row=pos.row;
        newPos.col=pos.col;
    }
    break; //don´t move
    default:
        cout << " (unknown command)";
    }
    DEBUG_PLAYER(cout << endl;);
    return newPos;
}

void Player::getpossiblemoves() {

    Pos a = Getcurpos();
    int cols = _map->colCount();
    int rows = _map->rowCount();
    Pos n;
    _pmnum=0;
    DEBUG_PLAYER(cout<<"sh: the current pos is ("<<a.row<<", "<<a.col<<")"<<endl;
    cout<<"sh: "<<rows<<cols<<endl;
    cout<<"sh: the possible moves are :"<<endl;);
    for(int i=0; i<9; i++)
    {
        switch(i) {
        case 1: {
            n.row=a.row-1;
            n.col=a.col;
            if(n.row<0) {
                break;
            }
            if(_map->getItem(n.row,n.col)== 1) {
                break;
            }
            _pm[_pmnum].a=i;
            _pm[_pmnum].x=n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;

            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; // move north
        case 2: {
            n.row=a.row-1;
            n.col=a.col+1;
            if( (n.row<0) ||(n.col>=cols) ) {
                break;
            }
            if(_map->getItem(n.row,n.col)== 1) {
                break;
            }
            _pm[_pmnum].a=i;
            _pm[_pmnum].x=n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;
            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; // move north-east
        case 3: {
            n.row=a.row;
            n.col=a.col+1;
            if(n.col>=cols) {
                break;
            }
            if(_map->getItem(n.row,n.col)== 1) {
                break;
            }
            _pm[_pmnum].a=i;
            _pm[_pmnum].x=n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;
            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; // move east
        case 4: {
            n.row=a.row+1;
            n.col=a.col+1;
            if( (n.row>=rows) ||(n.col>=cols) ) {
                break;
            }
            if(_map->getItem(n.row,n.col)== 1) {
                break;
            }
            _pm[_pmnum].a=i;
            _pm[_pmnum].x=n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;
            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; // move east-south
        case 5: {
            n.row=a.row+1;
            n.col=a.col;
            if( n.row>=rows ) {
                break;
            }
            if(_map->getItem(n.row,n.col)== 1) {
                break;
            }
            _pm[_pmnum].a=i;
            DEBUG_PLAYER(cout<<"sh: i="<<i<<" / "<<_pm[_pmnum].a<<endl;);
            _pm[_pmnum].x=n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;
            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; // move south
        case 6: {
            n.row=a.row+1;
            n.col=a.col-1;
            if( (n.row>=rows) ||(n.col<0) ) {
                break;
            }
            if(_map->getItem(n.row,n.col)== 1) {
                break;
            }
            _pm[_pmnum].a=i;
            _pm[_pmnum].x=n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;
            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; // move south-west
        case 7: {
            n.row=a.row;
            n.col=a.col-1;
            if(n.col<0) {
                break;
            }
            if(_map->getItem(n.row,n.col)== 1) {
                break;
            }
            _pm[_pmnum].a=i;
            _pm[_pmnum].x=n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;
            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; // move west
        case 8: {
            n.row=a.row-1;
            n.col=a.col-1;
            //cout<<"sh: i="<<i<<"    "<<n.x<<n.y<<endl;
            if( (n.row<0) ||(n.col<0) ) {
                break;
            }
            if(_map->getItem(n.row,n.col)== 1) {
                break;
            }
            _pm[_pmnum].a=i;
            _pm[_pmnum].x=n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;
            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; // move west-north
        case 0: {
            n.row=a.row;
            n.col=a.col;
            _pm[_pmnum].a = i;
            _pm[_pmnum].x = n.row;
            _pm[_pmnum].y=n.col;
            _pm[_pmnum].score=-1;
            DEBUG_PLAYER(cout<<_pmnum+1<<". "<<_pm[_pmnum].a<<" :: ("<<_pm[_pmnum].x<<", "<<_pm[_pmnum].y<<")"<<endl;);
            _pmnum++;
        }
        break; //don´t move
        default:
            cout << " (unknown command)"<<endl;
        }


    }
}

/*void Player::scoreneighb() {

    int r = _pmap->rowCount();
    int c = _pmap->colCount();
    int L = r*c;
    float g = 0.8;

    //srand()
    for(int i=0; i<_pmnum; i++) {

        DEBUG_HS(cout <<i<<": " << _pm[i].x<<_pm[i].y<<_pmap->GetBase().x<<_pmap->GetBase().y << endl;);
        float dhb=_pmap->distance(_pm[i].y,_pm[i].x,_pmap->GetBase().y,_pmap->GetBase().x);
        DEBUG_HS(cout <<"distance1: " << dhb << endl;);
        float dhs=_pmap->distance(_pm[i].y,_pm[i].x,_oppos.y,_oppos.x);
        DEBUG_HS(cout <<"distance2: " << dhs << endl;);
        //float gsn = gaussian_noise();

        _pm[i].score = L-dhb + g*dhs + rand()*0.1*L ;
    }

}*/
/* AG130206: replaced wtih slightly more advanced smart hider:
 * For each action a score is generated:
 *   score = L - d_hb + f*d_hs + noise
 * with
 *   L = #rows*#col
 *   d_hb = distance hider-base
 *   d_hs = distance hider-seeker
 *   f = factor for d_hs
 *   noise = dhs_f*dhb_f*n_std*n()
 *      with:
 *          dhs_f and dhb_f: factors for d_hs and d_hb respectively, if the value is below SCORE_LESS_RAND_DIST
 *              then it will be divided. E.g.: if d_hs<SCORE_LESS_RAND_DIST then dhs_f=d_hs/SCORE_LESS_RAND_DIST
 *          n_std: standard deviation for the random (uniform) noise
 *          n(): random uniform noise function
*/
void Player::scoreneighb() {
    DEBUG_PLAYER(cout<<"scoring"<<endl;);

    int r = _map->rowCount();
    int c = _map->colCount();
    int L = r*c;


    DEBUG_PLAYER(cout << "L="<<L<<endl;);

    //srand()
    for(int i=0;i<_pmnum;i++){

        DEBUG_PLAYER(cout <<i<<": h=(" << _pm[i].x<<","<<_pm[i].y<<") b=("<<_map->getBase().row<<","<<_map->getBase().col<<") s=("<<_opPos.row<<","<<_opPos.col<<")" << endl;);
        float dhb=_map->distance(_pm[i].y,_pm[i].x,_map->getBase().col,_map->getBase().row);
        DEBUG_PLAYER(cout <<"distance h-b: " << dhb << endl;);
        float dhs=_map->distance(_pm[i].y,_pm[i].x,_opPos.col,_opPos.row); //TODO: should be 'if visible'
        DEBUG_PLAYER(cout <<"distance h-s: " << dhs << endl;);
        //float gsn = gaussian_noise();

        //AG130206: the closer to the goal or the opponent, the less random noise
        float dhs_factor = 1, dhb_factor = 1;
        if (dhs < SCORE_LESS_RAND_DIST)
            dhs_factor = dhs/SCORE_LESS_RAND_DIST;
        if (dhb < SCORE_LESS_RAND_DIST)
            dhb_factor = dhb/SCORE_LESS_RAND_DIST;

        float noise = (dhs_factor*dhb_factor*SCORE_RAND_STD*rand()/RAND_MAX);
        DEBUG_PLAYER(cout << "noise="<<noise<<endl;);

         //AG130206: removed *L because
        _pm[i].score = L-dhb + SCORE_DHS_FACTOR*dhs + noise; //*L ;

        DEBUG_PLAYER(cout <<"-->score="<<_pm[i].score<<endl;);
    }

}

int Player::chooseaction() {
    int n=-1;
    int s=-1;
    for(int i=0; i<_pmnum; i++) {
        if(_pm[i].score > s ) {
            s=_pm[i].score;
            n=i;
        }

    }
    DEBUG_PLAYER(cout<<"sh: the action chosen is: "<<_pm[n].a<<"/ ("<<_pm[n].x<<", "<<_pm[n].y<<")"<<endl;);
    return _pm[n].a;
}


//ag130228: disabled
/*
void raytrace(char **m, int x0, int y0, int x1, int y1)
{
    //if (x0==x1 && y0==y1) returnm

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int x = x0;
    int y = y0;
    int n;
    int x_inc = (x1 > x0) ? 1 : -1;
    int y_inc = (y1 > y0) ? 1 : -1;
    int error = dx - dy;



    n=dx+dy+1;
    dx *= 2;
    dy *= 2;

    //cout << "raytrace-old: ";//tmp
    for (; n > 0; --n)
    {
        //cout<<"("<<x<<","<<y<<") "<<flush;//tmp
        if ( m[x][y] == GMap::VMAP_OBST ) { //2: obstacle
            m[x1][y1]= 0;     //0: not visible in this ray
            //cout << " NOT_VIS"<<endl;
            return;
        }

        if (error > 0)
        {
            x += x_inc;

            error -= dy;
        }
        else if(error<0)
        {
            y += y_inc;
            error += dx;
        }
        else //(error==0)
        {
            x += x_inc;
            error -= dy;
            y += y_inc;
            error += dx;
            n--;
        }
    }

    //cout<< " VIS"<<endl;//tmp

    m[x1][y1]=1; //visible
}*/

/*
 *Calculate visibility of player's current pos.
 */
//ag130228: disabled
#ifdef OLD_CODE
void Player::calculatevisible() {
    GMap* tmap=_map;

    int row = tmap->rowCount();
    int col = tmap->colCount();

    tmap->InitVisible();//initialize the visibility matrix
    //cout<<"the vis map was initialized and we are in calculatevisible()"<<endl;
    Pos p = tmap->GetBase();

    for(int r=0; r<row; r++) {
        for(int c=0; c<col; c++) {
            if( tmap->getVisibility(r,c)==2)//if there is an obstacle continue to the next cell
                continue;
            if (tmap->getVisibility(r,c)!=9)//it is already defined
                continue;
            /* //ag1201XX: this if was created by Chryso to make the base visible on the screen
            	// however this should be done by the GUI!!!
              if ( p.x==r && p.y==c) {//it 's the base
                if(!_type && tmap->getItem(r,c)==8) {
                    raytrace( tmap->getVisibletab(),_curpos.x, _curpos.y, r, c);
                    if(tmap->getVisibility(r,c)==0){
                        tmap->setbase(); //overwrites map to 'see base' in gui
                    }
                }
                tmap->setVisible(r,c);
                continue;
            }*/
            if ((r==_curPos.x) && (c==_curPos.y))
                continue; //we can skip current pos because already checked in raytrace

            raytrace( tmap->getVisibletab(), _curPos.x, _curPos.y, r, c);
        }
    }

}

/* Calculate invisible points seen from the base, i.e. potential places for hider to hide.
 */
void Player::calculateInvisible() {

    GMap* tmap=_map;
    char** map;
    _n=0;


    int row = tmap->rowCount();
    int col = tmap->colCount();

    //initialize the  matrix
    DEBUG_PLAYER(cout<<"randomhider(): calculate the invisible cells for the seeker"<<endl;);
    //cout<<"rh(): allocate mem for row = "<<row<<" and column = "<<col<<endl;

    //ag120416: switched row,col
    map = AllocateDynamicArray<char>(row,col); //(col,row);

    for(int r=0; r<row; r++) {
        for(int c=0; c<col; c++) {
            if(tmap->getItem(r,c)==1) { //there is an obstacle
                DEBUG_PLAYER(cout<<"wall found"<<endl;);
                map[r][c]=2;
            }
            else
                map[r][c]=9;
        }
    }
    DEBUG_PLAYER(cout<<"the  map was initialized and we are in calculateInvisible()"<<endl;);
    Pos p=tmap->GetBase();

    for(int r=0; r<row; r++) {
        //cout << "r="<<r<<" c=";
        for(int c=0; c<col; c++) {
            //cout << c<<","<<flush;
            if( map[r][c]==2)//if there is an obstacle continue to the next cell
                continue;
            if (map[r][c]!=9)//it is already defined
                continue;
            if ( p.x==r && p.y==c) {//it 's the base
                //map[r][c]==1;
                continue;
            }
            //cout << " rt"<<flush;
            raytrace( map,_opPos.x, _opPos.y, r, c);
            if( map[r][c]==0) { //if the cell is invisible add it int the inv array.
                cout<<_n<<flush;
                _inv[_n].x=r;
                _inv[_n].y=c;
                _n++;
                DEBUG_PLAYER(cout<<". ("<<_inv[_n-1].x<<", "<<_inv[_n-1].y<<")"<<endl;);
            }
        } //cout << endl;
    }

    FreeDynamicArray(map);

}
#endif

bool Player::opIsVisible() {
    return _map->isVisible(_curPos,_opPos);
}


void Player::printInfo() {
    //note: x=row,y=col ..

    cout << " type="<<_type<<";hiderType="<<_hidertype<<";curpos=("<<_curPos.col<<","<<_curPos.row<<");oppos=("<<_opPos.col<<","<<_opPos.row<<");#actions="<<_numActions<<";lastAction="<<_action<<endl;
    _map->printMap(_curPos.row,_curPos.col); //ag130301: other syntax printmap

}

vector<Pos> Player::getInvisiblePoints(int r, int c) {
    if (r<0) r=_curPos.row;
    if (c<0) c=_curPos.col;

    return _map->getInvisiblePoints(r,c);
}


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
