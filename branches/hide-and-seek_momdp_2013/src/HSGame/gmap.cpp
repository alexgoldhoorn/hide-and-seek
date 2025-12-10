
#include "gmap.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <sstream>
#include <string>
#include <cmath>

#include "../Utils/readwriteimage.h"

/*
#include <QFile>
#include <QString>
#include <QStringList>
*/

//#include "gworld.h"
#include "../hsconfig.h"

#include "../Utils/generic.h"
#include "../Utils/timer.h"
#include "../Utils/readwriteimage.h"

//! check if row,col is inside map
#define IS_POS_IN_MAP(r,c) (r>=0 && r<_rows && c>=0 && c<_cols)

using namespace std;


//test main
int main_map(int argc, char **argv) {
    if (argc < 2) {
        cout << "gfr: Expected a map file as parameter" << endl;
    } else {
        GMap gmap(argv[1]);
        DEBUG_HS(gmap.printMap(););
    }
    return 0;
}

// --- constructors ---

GMap::GMap() : _mapFixed(false) {
    //ag130227: disabled, REQUIRE createMap
    //createMap(MAX_ROWS,MAX_COLS);
    //_numFreeCells = -1;

    _indexMap = NULL;
    _pathPlanner = NULL;
    _visibleMat = NULL;
    _distMat = NULL;
}

GMap::GMap(int rows, int cols) : _mapFixed(false) {
    createMap(rows,cols);
}

GMap::GMap(const char* fileName) : _mapFixed(false)  {
    readMapFile(fileName);
    setMapFixed();
}

GMap::GMap(string fileName) : _mapFixed(false) {
    readMapFile(fileName.c_str());
    setMapFixed();
}

GMap::GMap(const char* pgmFileName, Pos base, unsigned int zoomOutFactor) : _mapFixed(false) {
    readPGMMapFile(pgmFileName,base,zoomOutFactor);
}

void GMap::readPGMMapFile(const char* pgmFileName, Pos base, unsigned int zoomOutFactor) {
    PGMData pgmdata;
    DEBUG_MAP(cout << "reading PGM: "<<pgmFileName<<endl;);
    readPGM(pgmFileName,&pgmdata);
    DEBUG_MAP(cout <<"ok"<<endl<<"size: "<<pgmdata.row <<"x"<<pgmdata.col << endl << " max gray: " << pgmdata.max_gray << endl
        << "data==null: "<< (pgmdata.matrix==NULL)<<endl;);

    //assume base doesn't need zoom, but already in right coords...
    _base.set(base);


    bool foundMapInfo = false;
    int beginRow = 0;
    int endRow = pgmdata.row-1;
    int beginCol = 0;
    int endCol = pgmdata.col-1;
    //first check for ray lines (value 205)
    //first begin row
    for(;beginRow<pgmdata.row && !foundMapInfo;beginRow++) {
        for(int c=0;c<pgmdata.col && !foundMapInfo;c++) {
            foundMapInfo = (pgmdata.matrix[beginRow][c]!=PGM_NO_EXPLORED_GRAY);
        }
    }
    //end row
    foundMapInfo = false;
    for(;endRow>=0 && !foundMapInfo;endRow--) {
        for(int c=0;c<pgmdata.col && !foundMapInfo;c++) {
            foundMapInfo = (pgmdata.matrix[endRow][c]!=PGM_NO_EXPLORED_GRAY);
        }
    }

    foundMapInfo = false;
    //first begin col
    for(;beginCol<pgmdata.col && !foundMapInfo; beginCol++) {
        for(int r=0; r<pgmdata.row && !foundMapInfo; r++) {
            foundMapInfo = (pgmdata.matrix[r][beginCol]!=PGM_NO_EXPLORED_GRAY);
        }
    }


    //end col
    foundMapInfo = false;
    for(;endCol>=0 && !foundMapInfo;endCol--) {
        for(int r=0; r<pgmdata.row && !foundMapInfo; r++) {
            foundMapInfo = (pgmdata.matrix[r][endCol]!=PGM_NO_EXPLORED_GRAY);
        }
    }

    cout << "Filtered: rows: "<<beginRow<<" - "<<endRow << "; cols: "<<beginCol<<" - " <<endCol<<endl;


    //calc new rows,cols based on zoomout fact
    /*int rows = (int)ceil(1.0*pgmdata.row/zoomOutFactor);
    int cols = (int)ceil(1.0*pgmdata.col/zoomOutFactor);*/
    int rows = (int)ceil(1.0*(endRow - beginRow +1)/zoomOutFactor);
    int cols = (int)ceil(1.0*(endCol - beginCol +1)/zoomOutFactor);

    //create map
    createMap(rows,cols,true);

    int numTot=0;
    int numObs=0;

    //now loop map and group cells
    int rStart = beginRow; //0;
    int cStart = beginCol; //0;
    for(int r=0;r<rows;r++) {
        cStart = beginCol;
        for(int c=0;c<cols;c++) {

            bool isObs = false;
            //check if 1 in the group is an obstacle..


            /*for(int r1=rStart;r1<rStart+zoomOutFactor && !isObs && r1<pgmdata.row;r1++) {
                for(int c1=cStart;c1<cStart+zoomOutFactor && !isObs && c1<pgmdata.col;c1++) {
                    if (pgmdata.matrix[r1][c1]<220) {
                        isObs = true;
                    }
                }
            }*/
            //search for obstacle in the group
            for(int r1=rStart;r1<rStart+zoomOutFactor && !isObs && r1<endRow;r1++) {
                for(int c1=cStart;c1<cStart+zoomOutFactor && !isObs && c1<endCol;c1++) {
                    if (pgmdata.matrix[r1][c1]<220) {
                        isObs = true;
                    }
                }
            }

            if (isObs) {
                addObstacle(r,c);
                numObs++;
            }
            numTot++;

            cStart += zoomOutFactor;
        }

        rStart += zoomOutFactor;
    }

    cout <<endl<<"Obs: "<<numObs<<" / "<<numTot<<endl;
    cout << "GMap obs: "<<numObstacles()<< " / " << _rows*_cols << " - " << _rows<<"x"<<_cols<<endl;

    /*
    createMap(pgmdata.row,pgmdata.col,true);

    numObs = numTot = 0;
    for(int r=0;r<pgmdata.row;r++) {
        for(int c=0;c<pgmdata.col;c++) {
            cout << pgmdata.matrix[r][c] <<" ";
            if (pgmdata.matrix[r][c]<255) {
                addObstacle(r,c);
                numObs++;
            }
            numTot++;
        }
        cout << endl;
    }
    cout <<endl<<"2Obs: "<<numObs<<" / "<<numTot<<endl;*/

    setMapFixed();
}

void GMap::setMapFixed() {
    //assert(!_mapFixed);
    _mapFixed = true;
    //TODO stuff that needs to be init. when map completely loaded
}

GMap::~GMap() {
    //AG120425: added freeing//AG130228: disable
    /*if (_visible!=NULL) {
        FreeDynamicArray(_visible);
    }*/
    if (_map!=NULL)  {
        FreeDynamicArray(_map);
    }
    if (_indexMap!=NULL)  {
        FreeDynamicArray(_indexMap);
    }
    if (_pathPlanner!=NULL)  {
        delete _pathPlanner;
    }
    /*if (_obstacles!=NULL) {
        delete _obstacles;
    }*/
    //AG130227: free vis mat
    if (_visibleMat!=NULL) {
        FreeDynamicArray(_visibleMat);
    }
    //AG130509: free vis mat
    if (_distMat!=NULL) {
        FreeDynamicArray(_distMat);
    }
}

// --- init ---

void GMap::createMap(int rows, int cols, bool doInit) {
    //assert(_map==NULL); //AG120425: only to be called at init
    assert(!_mapFixed);

    _indexMap = NULL;
    _pathPlanner = NULL;
    //_obstacles = NULL;
    //_numFreeCells = -1;

    //ag130227: set when requested
    _visibleMat = NULL;
    _pathPlanner = NULL;
    _distMat = NULL;

    _rows = rows;
    _cols = cols;
    //AG120416: fixed memory problem-> change order params to (rows,cols)
    _map = AllocateDynamicArray<char>(rows,cols);// (cols,rows);
    //_visible = AllocateDynamicArray<char>(rows,cols);//(cols,rows);
    if (doInit) {
        //InitVisible(1); //AG130228: disable
        initMap();
    }

}

void GMap::initVisMat() {
    //ag130227: init visible mat
    DEBUG_MAP(cout<<"GMap.initVisMat "<<_rows<<"x"<<_cols<<endl;);
    _visibleMat = AllocateDynamicArray<char>(_rows,_cols,_rows,_cols);
    FOR(r1,_rows) {
        FOR(c1,_cols) {
            FOR(r2,_rows) {
                FOR(c2,_cols) {
                    if (_map[r1][c1]==GMAP_OBSTACLE || _map[r2][c2]==GMAP_OBSTACLE) {
                        _visibleMat[r1][c1][r2][c2] = VMAT_OBST;
                    } else {
                        _visibleMat[r1][c1][r2][c2] = VMAT_UNSET;
                    }
                }
            }
        }
    }
    DEBUG_MAP(cout<<"GMap.initVisMat done"<<endl;);
}

//AG130228: disable
/*
void GMap::InitVisible(int all) {
    int row=_rows;
    int col=_cols;

    if(all==0) {
        for(int r=0; r<row; r++) {
            for(int c=0; c<col; c++) {
                if( _visible[r][c]==GMAP_BASE) //if its the base dont change it
                    continue;
                _visible[r][c]=VMAP_UNDEFINED; //insert a 9 (undefined)
            }
        }

    } else {
        for(int r=0; r<row; r++) {
            for(int c=0; c<col; c++) {
                _visible[r][c]=VMAP_UNDEFINED;
            }
        }
    }
}
*/


void GMap::initMap() {
    //initializes a map by putting zeros in all positions
    //??(except if their were obstacles)

    for(int r=0; r<_rows; r++) {
        for(int c=0; c<_cols; c++) {
            _map[r][c]=GMAP_FREE_SPACE;

            //_visible[r][c]=9; //AG130228: disable


            /* //NOTE: in version of server (or maybe all... we should
                    if( _map[r][c]==GMAP_OBSTACLE)
                        continue;
                    _map[r][c]=GMAP_FREE_SPACE;
            */
        }
    }

}



#ifdef USE_QT_MAP_LOAD
//AG120416: refactored using Qt (due to problems with last line)
void GMap::readMapFile(const char *fileName) {
    assert(!_mapFixed); //ag130304

    QFile mFile(fileName);

    DEBUG_MAP(cout << "Opening map "<<fileName<<" ... " <<flush;);

    if (mFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //read all data
        QString data;
        data = mFile.readAll();
        mFile.close();
        DEBUG_MAP(cout <<"ok"<<endl<<"parsing size ... "<<flush;);

        //parse string
        QStringList splitList = data.split(',');
        if (splitList.size()<3) {
            cout <<"ERROR @ GMap::readMapFile: Expected 3 parameters: #cols,#rows,<map>."<<endl;
            exit(EXIT_FAILURE);
        }

        //cols
        QString str = splitList[0];
        bool ok=false;
        _cols = str.toInt(&ok);
        if (!ok) {
            cout <<"ERROR @ GMap::readMapFile: first parameter expected to be the number of columns."<<endl;
            exit(EXIT_FAILURE);
        }

        //rows
        str = splitList[1];
        ok=false;
        _rows = str.toInt(&ok);
        if (!ok) {
            cout <<"ERROR @ GMap::readMapFile: first parameter expected to be the number of rows."<<endl;
            exit(EXIT_FAILURE);
        }

        if (_cols<=0 || _rows<=0 || _cols>MAX_COLS || _rows>MAX_ROWS) {
            cout <<"ERROR @ GMap::readMapFile: #cols="<<_cols<<",#rows="<<_rows<<", expected #rows: [1,"<<MAX_ROWS
                 <<"], expected #cols: [1,"<<MAX_COLS<<"]" <<endl;
            exit(EXIT_FAILURE);
        }

        DEBUG_MAP(cout <<_rows<<" x "<<_cols<<endl<<"creating map... "<<flush;);
        createMap(_rows, _cols, false);
        DEBUG_MAP(cout <<"ok"<<"parsing ... "<<endl;);

        //ag120521: obstacles
        //_obstacles = new Pos[_rows*_cols];
        //_obstnum = 0;
        //ag130301: use vector


        //the map
        data = splitList[2];
        splitList = data.split('\n'); //AG-NOTE: didn't work using : QChar::LineSeparator); //'\n');
        QStringList::iterator splitListIt;
        int r=0;
        //loop lines of map
        for (splitListIt=splitList.begin(); splitListIt!=splitList.end(); splitListIt++) {
            QString line = *splitListIt;
            if (line.isEmpty()) continue; //skip empty line

            DEBUG_MAP(cout <<"("<<r<<">)";);

            //check #cols
            if (line.size()!=_cols) {
                cout << "ERROR @ GMap::readMapFile: line "<<r<<" contains "<<line.size()<<" columns!"<<endl;
                exit(EXIT_FAILURE);
            }

            //loop cols
            for (int c=0; c<_cols; c++) {
                DEBUG_MAP(cout<<"{"<<c<<"}"<<flush;);

                char ch = line[c].toAscii();
                _map[r][c] = convertTextToNumChar(ch);
                //_visible[r][c] = VMAP_UNDEFINED; //AG130228: disable
                if(_map[r][c]==GMAP_BASE) {
                    _base.row=r;
                    _base.col=c;
                    DEBUG_MAP(cout<<"B"<<flush;);
                } else if(_map[r][c]== GMAP_OBSTACLE) {
                    //_visible[r][c] = VMAP_OBST; //GMAP_OBSTACLE;//TODO AG WAS 2!!!!!!! //AG130228: disable

                    /*_obstacles[_obstnum].x=r;
                    _obstacles[_obstnum].y=c;
                    _obstnum++;*/
                    //AG130301: change way of storing obstacles
                    _obstacleVector.push_back(Pos(r,c));


                    DEBUG_MAP(cout<<"X"<<flush;);
                }
                DEBUG_MAP(else cout <<"·"<<flush;);
            }

            DEBUG_MAP(cout <<"(<"<<r<<")"<<endl;);
            r++;
        }

        mFile.close();

        if (_rows != r) {
            cout << "ERROR @ GMap::readMapFile: "<<r<<" rows found, "<<_rows<<" expected!"<<endl;
            exit(EXIT_FAILURE);
        }

        DEBUG_MAP(cout<<"ok-map loaded"<<endl;);

    } else {
        cout << "ERROR @ GMap::readMapFile: Map file could not be opened."<<endl;
        exit(EXIT_FAILURE);
    }
}
#else //ELSE (ie not using QT)

//AG120416: refactored using Qt (due to problems with last line)
void GMap::readMapFile(const char *fileName) {
    assert(!_mapFixed); //ag130304

    DEBUG_MAP(cout << "Opening map "<<fileName<<" ... " <<flush;);

    ifstream reader;
    reader.open(fileName, ifstream::in);
    if (reader.good()) {

        //read first line
        string line;
        getline(reader,line);
        int _rows=-1,_cols=-1;

        //find first comma, read #cols
        unsigned int i = line.find(",");
        if (i!=string::npos) {
            string cstr = line.substr(0,i);
            _cols = atoi(cstr.c_str());
        } else {
            cout <<"ERROR @ GMap::readMapFile: first parameter expected to be the number of columns."<<endl;
            exit(EXIT_FAILURE);
        }

        //second comma, read #rows
        unsigned int i2 = line.find(",",i+1);
        if (i2!=string::npos) {
            string rstr = line.substr(i+1,i2-i+1);
            _rows = atoi(rstr.c_str());
        } else {
            cout <<"ERROR @ GMap::readMapFile: first parameter expected to be the number of rows."<<endl;
            exit(EXIT_FAILURE);
        }

        if (_cols<=0 || _rows<=0 || _cols>MAX_COLS || _rows>MAX_ROWS) {
            cout <<"ERROR @ GMap::readMapFile: #cols="<<_cols<<",#rows="<<_rows<<", expected #rows: [1,"<<MAX_ROWS
                 <<"], expected #cols: [1,"<<MAX_COLS<<"]" <<endl;
            exit(EXIT_FAILURE);
        }

        DEBUG_MAP(cout <<_rows<<" x "<<_cols<<endl<<"creating map... "<<flush;);
        createMap(_rows, _cols, false);
        DEBUG_MAP(cout <<"ok"<<"parsing ... "<<endl;);

        int r = 0;
        while(!reader.eof() && r<_rows) // To get you all the lines.
        {
            getline(reader,line);

            if (line.size()==0 || line.at(0)=='#') continue; //skip empty line

            DEBUG_MAP(cout <<"("<<r<<">)";);

            //check #cols
            if ((int)line.size()!=_cols) {
                cout << "ERROR @ GMap::readMapFile: line "<<r<<" contains "<<line.size()<<" columns!"<<endl;
                exit(EXIT_FAILURE);
            }

            //loop cols
            for (int c=0; c<_cols; c++) {
                DEBUG_MAP(cout<<"{"<<c<<"}"<<flush;);

                _map[r][c] = convertTextToNumChar(line[c]);

                if(_map[r][c]==GMAP_BASE) {
                    _base.row=r;
                    _base.col=c;
                    DEBUG_MAP(cout<<"B"<<flush;);
                } else if(_map[r][c]== GMAP_OBSTACLE) {
                    //AG130301: change way of storing obstacles
                    _obstacleVector.push_back(Pos(r,c));
                    DEBUG_MAP(cout<<"X"<<flush;);
                }
                DEBUG_MAP(else cout <<"·"<<flush;);
            }

            DEBUG_MAP(cout <<"(<"<<r<<")"<<endl;);
            r++;
        }

        //close
        reader.close();

        if (_rows != r) {
            cout << "ERROR @ GMap::readMapFile: "<<r<<" rows found, "<<_rows<<" expected!"<<endl;
            exit(EXIT_FAILURE);
        }

        DEBUG_MAP(cout<<"ok-map loaded"<<endl;);

    } else {
        cout << "ERROR @ GMap::readMapFile: Map file could not be opened."<<endl;
        exit(EXIT_FAILURE);
    }
}


void GMap::writeMapFile(const char *fileName) {
    assert(_mapFixed); //ag130304

    DEBUG_MAP(cout << "Opening map "<<fileName<<" ... " <<flush;);

    //ifstream reader;
    ofstream writer;
    writer.open(fileName);

    writer << _cols << "," << _rows << ","<<endl;
    for(int r=0; r<_rows; r++) {
        for(int c=0; c<_cols; c++) {
            writer << (int)_map[r][c];
        }
        writer << endl;
    }
    writer << endl;

    writer.close();
}
#endif //USE_QT


void GMap::printMap(int viewR,int viewC) {

    /*if (useVisib && _visible == NULL) {
        useVisib = false;
        cout << "not using visibility map"<<endl;
    }*/
    //AG130228
    bool useVisib = viewR>=0;



    if (useVisib) cout <<" =visible free; ·=invisible free; capital=visible, small=invisible"<<endl;

    char x=(char)0;
    //draw map
    for (int r=-2; r<=_rows; r++) {
        for (int c=-2; c<=_cols; c++) {

            //draw corners and line borders
            if (r<0 || r==_rows) {
                if (c<0 || c==_cols)
                    x='+';
                else if (r == -2)
                    x = (c % 10) + '0';
                else
                    x='-';
            } else if (c<0 || c==_cols) {
                if (c == -2)
                    x = (r % 10) + '0';
                else
                    x = '|';
            } else {
                //ag130301: first check visibility
                bool isInVis = false;
                if (useVisib) isInVis = !isVisible(viewR,viewC,r,c); //ag130301: use function, since it generates the visibility map

                //show map item
                switch (_map[r][c]) {
                case GMAP_FREE_SPACE:
                    if (isInVis)
                        x='.';
                    else
                        x=' ';
                    break;
                case GMAP_OBSTACLE:
                    if (isInVis)
                        x='x';
                    else
                        x='X';
                    break;
                case GMAP_BASE:
                    if (isInVis)
                        x='b';
                    else
                        x='B';
                    break;
                case GMAP_SEEKER:
                    if (isInVis)
                        x='s';
                    else
                        x='S';
                    break;
                case GMAP_HIDER:
                    if (isInVis)
                        x='h';
                    else
                        x='H';
                    break;

                default:
                    x='?';
                }
            }

            cout<<x;
        }
        if (x=='.')
            cout <<"·";
        else
            cout <<endl;
    }
}



void GMap::printMap(Pos seekerPos, Pos hiderPos) {

    /*if (useVisib && _visible == NULL) {
        useVisib = false;
        cout << "not using visibility map"<<endl;
    }*/
    //AG130228
    bool useVisib = true; // viewR>=0;

    int viewR = seekerPos.row;
    int viewC = seekerPos.col;



    if (useVisib) cout <<" =visible free; ·=invisible free; capital=visible, small=invisible"<<endl;

    char x=(char)0;
    //draw map
    for (int r=-2; r<=_rows; r++) {
        for (int c=-2; c<=_cols; c++) {

            //draw corners and line borders
            if (r<0 || r==_rows) {
                if (c<0 || c==_cols)
                    x='+';
                else if (r == -2)
                    x = (c % 10) + '0';
                else
                    x='-';
            } else if (c<0 || c==_cols) {
                if (c == -2)
                    x = (r % 10) + '0';
                else
                    x = '|';
            } else {
                //ag130301: first check visibility
                bool isInVis = false;
                if (useVisib) isInVis = !isVisible(viewR,viewC,r,c); //ag130301: use function, since it generates the visibility map

                //show map item

                if (seekerPos.row == r && seekerPos.col==c) {
                    if (isInVis)
                        x='s';
                    else
                        x='S';
                    break;
                } else if (seekerPos.row == r && seekerPos.col==c) {
                    if (isInVis)
                        x='h';
                    else
                        x='H';
                    break;
                } else if (_map[r][c]==GMAP_OBSTACLE) {
                    if (isInVis)
                        x='x';
                    else
                        x='X';
                    break;
                } else if (_map[r][c]==GMAP_BASE) {
                    if (isInVis)
                        x='b';
                    else
                        x='B';
                    break;
                } else if (_map[r][c]==GMAP_FREE_SPACE) {
                    if (isInVis)
                        x='.';
                    else
                        x=' ';
                    break;
                } else {
                    x='?';
                }
            }

            //cout<<x;
            if (x=='.')
                cout <<"·";
            else
                cout <<x;
        }
        /*if (x=='.')
            cout <<"·";
        else
            cout <<endl;*/
        cout << endl;
    }
}


void GMap::createAStarPathPlanner() {
    _pathPlanner = new AStarPathPlanner(_map, _rows, _cols);
}

void GMap::createPropDistPlanner() {
    //DEBUG_HS(cout<<"sp: in createPropDistPlanner()"<<endl;);
    _pathPlanner = new PropDistPlanner(_map, _rows, _cols,8);
}

/*float GMap::distance(int x1,int y1,int x2,int y2) {
    DEBUG_MAP(cout<<"sp: in distance()   ";);
    DEBUG_MAP(cout<<"from ("<< x1<<","<<y1<<") to ("<<x2<<", "<<y2<<")"<<endl;);
    float d = _pathPlanner->distance(x1,y1,x2,y2);
    return d;
}

QPoint GMap::nextStep(int x1, int y1, int x2, int y2) {
    return _pathPlanner->nextStep(x1,y1,x2,y2);
}*/

//AG TODO: for bigger maps make a 'Sparse' matrix !!
void GMap::initDistMat() {
    //ag130227: init visible mat
    _distMat = AllocateDynamicArray<float>(_rows,_cols,_rows,_cols);
    FOR(r1,_rows) {
        FOR(c1,_cols) {
            FOR(r2,_rows) {
                FOR(c2,_cols) {
                    if (_map[r1][c1]==GMAP_OBSTACLE || _map[r2][c2]==GMAP_OBSTACLE) {
                        _distMat[r1][c1][r2][c2] = GMap::DMAT_OBST;
                    } else {
                        _distMat[r1][c1][r2][c2] = GMap::DMAT_UNSET;
                    }
                }
            }
        }
    }
}


float GMap::distance(int r1,int c1,int r2,int c2) {
    DEBUG_MAP(cout<<"sp: in distance()   ";);
    DEBUG_MAP(cout<<"from ["<<r1<<","<<c1<<"] to ["<<r2<<", "<<c2<<"]"<<endl;);

    if (_pathPlanner==NULL) {
        createPropDistPlanner(); //ag130304: create pathplanner if not exists
    }

    //ag130617: check if caching here or in pathplanner
    if (_pathPlanner->doesCacheResults()) {
        return _pathPlanner->distance(c1,r1,c2,r2);
    } else {
        //ag130509: cache distsance results
        if (_distMat==NULL) {
            initDistMat();
        }

        if (_distMat[r1][c1][r2][c2]==DMAT_UNSET) {
            _distMat[r1][c1][r2][c2] = _pathPlanner->distance(c1,r1,c2,r2);
            //AG130615: also opposite
            _distMat[r2][c2][r1][c1] = _distMat[r1][c1][r2][c2];
        }

        return _distMat[r1][c1][r2][c2];
    }
}

float GMap::distance(Pos &p1, Pos &p2) {
    return distance(p1.row,p1.col,p2.row,p2.col);
}

Pos GMap::nextStep(int r1,int c1,int r2,int c2) {
    Pos retPos;
    _pathPlanner->nextStep(c1,r1,c2,r2,&retPos.col,&retPos.col);
    return retPos;
}

int GMap::numFreeCells() {
    /*if (_mapFixed && _numFreeCells>=0) {
        return _numFreeCells;
    }

    //calculate free cells
    int f=0;
    for (int r=0; r<_rows; r++) {
        for (int c=0; c<_cols; c++) {
            if (_map[r][c] != GMAP_OBSTACLE) f++;
        }
    }

    if (_mapFixed) {
        _numFreeCells = f;
    }

    return f;*/

    return _rows*_cols - numObstacles();
}


/*int GMap::numtObstacles() {
    return _rows*_cols - numFreeCells();
}*/




//get the index of the state (used by APPL in its POMDP and policy file)
//note: obstacels are NOT counted as states and therefore skipped at the index!
/*int GMap::getPosIndex(int pr, int pc) {
    assert(pr>=0 && pr<_rows);
    assert(pc>=0 && pc<_cols);

    int i=0;
    for (int r=0; r<_rows; r++) {
        for (int c=0; c<_cols; c++) {

            if (pr == r && pc==c) {
                if (_map[r][c]==GMAP_OBSTACLE) {
                    cout << "WARNING: gmap.getPosIndex: ("<<pc<<","<<pr<<") is obstacle and chosen to get index!!!!!"<<endl;
                }
                return i;
            }

            //only increase index when NOT an obstacle
            if (_map[r][c]!=GMAP_OBSTACLE) i++;
        }
    }

    return -1;
}

int GMap::getPosIndex(Pos pos) {
    return getPosIndex(pos.x,pos.y);
}*/

//AG130228: disabled -> use visibleMat
/*
//returns if pos visible
bool GMap::isVisible(Pos pos) {
    //ag TODO CHECK:
    //- use x=r,y=c??
    //- can i only check for 0 or should i incorporate 9??
    //- cna we assume _visible exists?

    return (_visible[pos.x][pos.y] != 0);

}

bool GMap::isVisible(int r, int c) {
    //ag TODO CHECK:
    //- can i only check for 0 or should i incorporate 9??
    //- cna we assume _visible exists?
    return (_visible[r][c] != 0);
}
*/

//AG130228: disabled
/*
int GMap::getInvisibleCount() {
    int c=0;
    for(int r=0; r<_rows; r++) {
        for(int c=0; c<_cols; c++) {
            if (_visible[r][c]==0) c++;
        }
    }
    return c;
}
*/



//returns if is obstacle
bool GMap::isObstacle(int r, int c) {
    assert(IS_POS_IN_MAP(r,c));
    return _map[r][c]==GMap::GMAP_OBSTACLE;
}
bool GMap::isObstacle(Pos pos) {
    assert(IS_POS_IN_MAP(pos.row,pos.col));
    return _map[pos.row][pos.col]==GMap::GMAP_OBSTACLE;
}

//ag120418: check if is base
bool GMap::isBase(int r, int c) {
    assert(IS_POS_IN_MAP(r,c));
    return (_base.row==r && _base.col==c);
}


//AG130228: disabled
/*
//count invisible cells
int GMap::invisibleCells() {
    assert(_visible!=NULL);

    int invCells = 0;

    for (int r=0; r<_rows; r++) {
        for (int c=0; c<_cols; c++) {
            if (_visible[r][c] == 0) invCells++;
        }
    }

    return invCells;
}*/

//ag120227: create index map
void GMap::createIndexMap() {
    assert(_rows>0 && _cols>0 && _map!=NULL);

    DEBUG_MAP(cout<<"GMap.createIndexMap:"<<endl;);

    _indexMap = AllocateDynamicArray<int>(_rows,_cols); //ag120416: reversed - (_cols,_rows); //new int[_rows][_cols];
    _coordVector.clear(); //ag130304:use coord vector

    //generate index
    int i=0;
    for (int r=0; r<_rows; r++) {
        for (int c=0; c<_cols; c++) {
            if (_map[r][c] == GMap::GMAP_OBSTACLE) {
                //an obstacle
                _indexMap[r][c] = -1;
            } else {
                //add pos
                _coordVector.push_back(Pos(r,c));

                //not an obstacle
                _indexMap[r][c] = i;
                i++;
            }
            DEBUG_MAP(cout << _indexMap[r][c] <<" ";);
        }
        DEBUG_MAP(cout <<endl;);
    }

}


int GMap::getIndexFromCoord(int r, int c) {
    //assert(_indexMap != NULL);
    //AG130301: require user to fix map first
    assert(_mapFixed);

    if (_indexMap==NULL) {
        createIndexMap();
    }

    return _indexMap[r][c];
}

Pos GMap::getCoordFromIndex(int i) {
    assert(_mapFixed);

    if (_indexMap==NULL) {
        createIndexMap();
    }

    return _coordVector[i];
}


//ag130227: is visible for a matrix
bool GMap::isVisible(int r1, int c1, int r2, int c2) {
    assert(r1>=0 && c1>=0 && r1<_rows && c1<_cols);
    assert(r2>=0 && c2>=0 && r2<_rows && c2<_cols);

    if (_visibleMat == NULL) {
        initVisMat();
    }
    if (_visibleMat[r1][c1][r2][c2] == VMAT_UNSET) {
        //not yet set
        char vis = raytrace(_visibleMat[r1][c1],r1,c1,r2,c2 );
        //also set from other way
        _visibleMat[r2][c2][r1][c1] = vis;
        //cout << "calc visibmat: "<<(int)vis<<endl;

        //AG130614: also opposite has same visiblity
        _visibleMat[r1][c1][r2][c2] = vis;

        assert(vis!=VMAT_UNSET);
    }

    return (_visibleMat[r1][c1][r2][c2]==VMAT_VIS);
}

bool GMap::isVisible(Pos p1, Pos p2) {
    return isVisible(p1.row,p1.col,p2.row,p2.col);
}


//AG120227: adapted version
//TODO: replace completely old (chryso) visibility check/matrix
//TODO: could be more efficient, setting all in path that are vissible
char GMap::raytrace(char **m, int x0, int y0, int x1, int y1)
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

    //cout << "raytrace-new: "<<flush;//tmp

    for (; n > 0; --n)
    {
        //cout<<"("<<x<<","<<y<<") "<<flush;//tmp
        if ( m[x][y] == VMAT_OBST ) {
            m[x1][y1] = VMAT_NOT_VIS;
            //cout << " NOT_VIS"<<endl;
            return VMAT_NOT_VIS;
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

    m[x1][y1]=VMAT_VIS;

    //cout<< " VIS"<<endl;//tmp

    return m[x1][y1];
}


vector<Pos> GMap::getInvisiblePoints(Pos p) {
    return getInvisiblePoints(p.row, p.col);
}


vector<Pos> GMap::getInvisiblePoints(int r, int c) {
    assert(r>=0 && c>=0 && r<_rows && c<_cols);

    vector<Pos> posVector;
    FOR(r2,_rows) {
        FOR(c2,_cols) {
            if ( _map[r2][c2]!=GMAP_OBSTACLE && !isVisible(r,c,r2,c2)) {
                posVector.push_back(Pos(r2,c2));
            }
        }
    }

    return posVector; //TODO TEST AND FINISH GMAP (remove _visible)
}

bool GMap::isPosInMap(Pos &pos) {
    return IS_POS_IN_MAP(pos.row,pos.col);//pos.row>=0 && pos.row<_rows && pos.col>=0 && pos.col<_cols;
}

bool GMap::isPosInMap(int r, int c) {
    return IS_POS_IN_MAP(r,c); // r>=0 && r<_rows && c>=0 && c<_cols;
}



// unit tests

void GMap::testVisibility() {
    bool b1,b2;
    unsigned int inc=0,tc=0,viscount=0;
    cout << "Testing visiblitiy: "<<endl;
    FORs(r1,_rows) {
        FORs(c1,_cols) {
            if (!isObstacle(r1,c1)) {
                FORs(r2,_rows) {
                    FORs(c2,_cols) {
                        if (!isObstacle(r2,c2)) {
                            b1 = isVisible(r1,c1,r2,c2);
                            b2 = isVisible(r2,c2,r1,c1);
                            if (b1!=b2) {
                                inc++;
                                cout <<  "r"<<r1<<"c"<<c1<<"-r"<<r2<<"c"<<c2<<": "<<b1<<" - other way: "<<b2<<endl;
                            }
                            if (b1||b2) viscount++;
                            tc++;
                        }
                    }
                }
            }
        }
    }
    if (inc>0)
        cout << "ERROR: "<<inc<<" of "<<tc<<" inconsistent"<<endl;
    else
        cout << "OK! all "<<tc<<" states ok, visible: "<<viscount<<endl;
}

void GMap::testDistance() {
    float b1,b2;
    //int n = 0;
    unsigned int inc=0,tc=0;
    int n=numFreeCells();
    long n2=n*n;
    Timer timer;
    //long n3=n2/2;
    long i=0;
    long tot =0;
    long maxD = 0;

    cout <<"States: "<<n<<"; combis:"<<n2<<endl;

    cout << "Testing distance: "<<endl;
    /*
    FORs(r1,_rows) {
        FORs(c1,_cols) {
            if (!isObstacle(r1,c1)) {
                FORs(r2,_rows) {
                    FORs(c2,_cols) {
     */
    int start= timer.startTimer();
    for(int r1=0;r1<_rows-1;r1++) {
        for(int c1=0;c1<_cols-1;c1++) {
            if (!isObstacle(r1,c1)) {
                for(int r2=r1+1;r2<_rows;r2++) {
                    for(int c2=c1+1;c2<_cols;c2++) {
                        if (!isObstacle(r2,c2)) {
                            int timerID = timer.startTimer();
                            b1 = distance(r1,c1,r2,c2);
                            b2 = distance(r2,c2,r1,c1);

                            long t = timer.stopTimer(timerID);
                            i++;

                            /*if (i*t % 10000000 ==0) {
                                cout << t <<" "<< flush;
                            }*/
                            tot+=t;
                            if (b1!=b2) {
                                inc++;
                                cout <<  "r"<<r1<<"c"<<c1<<"-r"<<r2<<"c"<<c2<<": "<<b1<<" - other way: "<<b2<<endl;
                            }
                            if (b1>maxD) maxD = b1;
                            if (b2>maxD) maxD = b2;
                            tc++;
                        }
                    }
                }
            }
        }
    }
    long tot2 = timer.stopTimer(start);
    cout <<"total time: "<<tot<<" s = "<<(tot/3600.0)<<"h"<<endl;
    cout <<"total time: "<<tot2<<" s = "<<(tot2/3600.0)<<"h"<<endl;
    cout <<"max distance: "<<maxD<<endl;
    if (inc>0)
        cout << "ERROR: "<<inc<<" of "<<tc<<" inconsistent"<<endl;
    else
        cout << "OK! all "<<tc<<" states ok"<<endl;
}



// ---- POS ----

string Pos::toString() {
    std::stringstream sstr;
    sstr << "(" << row << "," << col << ")";
    return sstr.str();
}

bool Pos::equals(Pos p) {
    return row==p.row && col==p.col;
}


void Pos::set(Pos &p) {
    row = p.row;
    col = p.col;
}

bool Pos::isSet() {
    return (row!=-1 && col!=-1);
}

void Pos::clear() {
    row = col = -1;
}
