
#include "HSGame/gmap.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cassert>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>

#include "exceptions.h"

#include "Utils/readwriteimage.h"

#include "hsconfig.h"

#include "Utils/generic.h"
#include "Utils/timer.h"
#include "Utils/readwriteimage.h"

#include "hsglobaldata.h"

#include "PathPlan/propdistplanner.h"
#include "PathPlan/astarplanner.h"

//! check if row,col is inside map
#define IS_POS_IN_MAP(r,c) (r>=0 && r<_rows && c>=0 && c<_cols)

using namespace std;




//test main
int main_map(int argc, char **argv) {
    if (argc < 2) {
        cout << "gfr: Expected a map file as parameter" << endl;
    } else {
        GMap gmap(argv[1], NULL);
        DEBUG_HS(gmap.printMap(););
    }
    return 0;
}

// --- constructors ---

GMap::GMap(SeekerHSParams* params) : _mapFixed(false), _editMode(false), _params(params) {
    //ag130227: disabled, REQUIRE createMap
    //createMap(MAX_ROWS,MAX_COLS);
    //_numFreeCells = -1;

    _indexMap = NULL;
    _pathPlanner = NULL;
    _visibleMat = NULL;
    _distMat = NULL;
    _actionsPosMat = NULL;
    _visibleMatDynObst = NULL;
}

GMap::GMap(int rows, int cols, SeekerHSParams* params) : _mapFixed(false), _editMode(false), _params(params) {
    createMap(rows,cols);
}

GMap::GMap(string fileName, SeekerHSParams* params) : _mapFixed(false), _editMode(false), _params(params) {
    _name = fileName;
    readMapFile(fileName);
    setMapFixed();
}

GMap::GMap(string pgmFileName, Pos base, unsigned int zoomOutFactor, SeekerHSParams* params) : _mapFixed(false), _editMode(false),
                        _params(params) {
    _name = pgmFileName;
    readPGMMapFile(pgmFileName,base,zoomOutFactor);
}

GMap::GMap(string yamlFileName, Pos base, double cellSize, double rowDist_m, double colDist_m, SeekerHSParams *params) {
    _name = yamlFileName;
    _params = params;
    readYAMLMapFile(yamlFileName, base, cellSize, rowDist_m, colDist_m);
}

void GMap::setParams(SeekerHSParams *params) {
    _params = params;
}

SeekerHSParams* GMap::getParams() const {
    return _params;
}

GMap::~GMap() {
    if (_map!=NULL)  {
        assert(_rows>0 && _cols>0);
        FreeDynamicArray<char>(_map, (unsigned int)_rows);
    }
    if (_indexMap!=NULL)  {
        FreeDynamicArray<int>(_indexMap, (unsigned int)_rows);
    }
    if (_pathPlanner!=NULL)  {
        delete _pathPlanner;
    }
    if (_visibleMat!=NULL) {
        FreeDynamicArray<char>(_visibleMat, (unsigned int)_rows,(unsigned int)_cols,(unsigned int)_rows);
    }
    if (_visibleMatDynObst!=NULL) {
        FreeDynamicArray<char>(_visibleMatDynObst, (unsigned int)_rows);
        //free(reinterpret_cast<char*>(_visibleMatDynObst));
    }
    if (_distMat!=NULL) {
        FreeDynamicArray<float>(_distMat, (unsigned int)_rows,(unsigned int)_cols,(unsigned int)_rows);
    }    
    if (_actionsPosMat!=NULL) {
        FreeDynamicArray<Pos>(_actionsPosMat, (unsigned int)_rows,(unsigned int)_cols);
    }
}


void GMap::readYAMLMapFile(string yamlFileName, const Pos &base, double cellSize, double rowDist_m, double colDist_m) {
    ifstream reader;
    reader.open(yamlFileName, ifstream::in);

    //read params
    if (reader.good()) {
        string pgmFile = "";
        double res=-1;
        double origX=-1, origY=-1;

        string line;

        //get params
        while (getline(reader,line)) {
            cout << line<<endl;
            if (line.substr(0,6)=="image:") {
                pgmFile = line.substr(7,line.length()-6);
                remove(pgmFile.begin(), pgmFile.end(), ' ');
            } else if (line.substr(0,11)=="resolution:") {
                res = stof(line.substr(12,line.length()-12));
            } else if (line.substr(0,7)=="origin:") {
                size_t foundBr = line.find("[");
                size_t foundComma1 = line.find(",", foundBr+1);
                size_t foundComma2 = line.find(",", foundComma1+1);

                if (foundBr==string::npos || foundComma1==string::npos || foundComma2==string::npos) {
                    cout <<"No legal numbers on orig"<<endl;
                    return;
                }

                origX = stof(line.substr(foundBr+1,foundComma1-foundBr-1));
                origY = stof(line.substr(foundComma1+1,foundComma2-foundComma1-1));
            }
        }

        //AG140530: check orig X,Y
        if (origX < 0) {
            cout << "ERROR: origin X should be 0 or less, but is: "<<origX<<endl;
            exit(-1);
        }
        if (origY < 0) {
            cout << "ERROR: origin Y should be 0 or less, but is: "<<origY<<endl;
            exit(-1);
        }

        //calc params
        unsigned int zoomOutFac = (unsigned int)round(cellSize/res);
        unsigned int beginRow = (unsigned int)round(-origY/res);
        unsigned int beginCol = (unsigned int)round(-origX/res);
        unsigned int endRow = beginRow + (unsigned int)round(rowDist_m/res);
        unsigned int endCol = beginCol + (unsigned int)round(colDist_m/res);

        //find path of file
        size_t foundLast = yamlFileName.rfind("/");
        string path="";
        if (foundLast!=string::npos) {
            path = yamlFileName.substr(0,foundLast+1);
        }

        cout << "YAML file:     "<<yamlFileName<<endl
             << "Path:          "<<path<<endl
             << "PGM file:      "<<pgmFile<<endl
             << "Resolution:    "<<res<<endl
             << "Origin:        "<<origX<<","<<origY<<endl
             << "Cell size:     "<<cellSize<<endl
             << "Zoom out fac:  "<<zoomOutFac<<endl
             << "Map size:      "<<rowDist_m<<" m x "<<colDist_m<<" m"<<endl
             << "Map row-cols:  r"<<beginRow<<"c"<<beginCol<<"-r"<<endRow<<"c"<<endCol<<endl;

        readPGMMapFile(path+pgmFile,base,zoomOutFac,beginRow,beginCol,endRow,endCol);
    }

    reader.close();
}


void GMap::readPGMMapFile(string pgmFileName, const Pos &base, unsigned int zoomOutFactor,
                          unsigned int beginRow, unsigned int beginCol, unsigned int endRow, unsigned int endCol) {
    PGMData pgmdata;
    DEBUG_MAP(cout << "reading PGM: "<<pgmFileName<<endl;);
    readPGM(pgmFileName.c_str(), &pgmdata);
    DEBUG_MAP(cout <<"ok"<<endl<<"size: "<<pgmdata.row <<"x"<<pgmdata.col << endl << " max gray: " << pgmdata.max_gray << endl
        << "data==null: "<< (pgmdata.matrix==NULL)<<endl;);

    //assume base doesn't need zoom, but already in right coords...
    _base = base;


    cout << "Image size: "<<pgmdata.row<<"x"<<pgmdata.col<<endl;

    unsigned int d = endRow-beginRow;
    endRow = pgmdata.row-1 - beginRow;
    beginRow = endRow - d;

    if (beginRow==0 && endRow==0 && beginCol==0 && endCol==0) {
        //bool foundMapInfo = false;
        beginRow = 0;
        endRow = pgmdata.row-1;
        beginCol = 0;
        endCol = pgmdata.col-1;
        //first check for ray lines (value 205)
        //first begin row
        /*for(;beginRow<pgmdata.row && !foundMapInfo;beginRow++) {
            for(unsigned int c=0;c<pgmdata.col && !foundMapInfo;c++) {
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
            for(unsigned int r=0; r<pgmdata.row && !foundMapInfo; r++) {
                foundMapInfo = (pgmdata.matrix[r][beginCol]!=PGM_NO_EXPLORED_GRAY);
            }
        }

        //end col
        foundMapInfo = false;
        for(;endCol>=0 && !foundMapInfo;endCol--) {
            for(unsigned int r=0; r<pgmdata.row && !foundMapInfo; r++) {
                foundMapInfo = (pgmdata.matrix[r][endCol]!=PGM_NO_EXPLORED_GRAY);
            }
        }*/
    }

    cout << "Filtered: rows: "<<beginRow<<" - "<<endRow << "; cols: "<<beginCol<<" - " <<endCol<<endl;

    //AG140530: added stop at limit of image size, even though passed row and column are higher
    if ((int)endRow>=pgmdata.row) {
        cout << "WARNING: the given end row "<<endRow<<" passes the image size of "<<pgmdata.row<<", so setting to this maximum."<<endl;
        endRow = pgmdata.row-1;
    }
    if ((int)endCol>=pgmdata.col) {
        cout << "WARNING: the given end col "<<endCol<<" passes the image size of "<<pgmdata.col<<", so setting to this maximum."<<endl;
        endCol = pgmdata.col-1;
    }

    //calc new rows,cols based on zoomout fact
    int rows = (int)round(1.0*(endRow - beginRow +1)/zoomOutFactor);
    int cols = (int)round(1.0*(endCol - beginCol +1)/zoomOutFactor);

    //create map
    createMap(cols,rows,/*rows,cols,*/ true);

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

            //search for obstacle in the group            
            for(int r1=rStart;r1<rStart+(int)zoomOutFactor && !isObs && r1<=(int)endRow; r1++) {
                for(int c1=cStart;c1<cStart+(int)zoomOutFactor && !isObs && c1<=(int)endCol; c1++) {
                    if (pgmdata.matrix[r1][c1]<220) {
                        isObs = true;
                    }
                }
            }

            if (isObs) {
                //add obstacle
                assert(isPosInMap(c,rows-1-r));
                addObstacle(c,rows-1-r);
                numObs++;
            }
            numTot++;

            cStart += zoomOutFactor;
        }

        rStart += zoomOutFactor;
    }

    cout <<endl<<"Obstacles: "<<numObs<<" / "<<numTot<<endl;
    cout << "GMap obs: "<<numObstacles()<< " / " << _rows*_cols << " - " << _rows<<"x"<<_cols<<endl;

    setMapFixed();
}

void GMap::setMapFixed() {
    //assert(!_mapFixed);
    _mapFixed = true;
    //TODO stuff that needs to be init. when map completely loaded
}



// --- init ---

void GMap::createMap(int rows, int cols, bool doInit) {
    //assert(_map==NULL); //AG120425: only to be called at init
    assert(!_mapFixed);
    assert(rows>0 && cols>0);

    _indexMap = NULL;
    //ag130227: set when requested
    _visibleMat = NULL;
    _visibleMatDynObst = NULL;
    _pathPlanner = NULL;
    _distMat = NULL;
    _actionsPosMat = NULL;

    _rows = rows;
    _cols = cols;
    //reserve map matrix
    _map = AllocateDynamicArray<char>((unsigned int)rows, (unsigned int)cols);

    //_visible = AllocateDynamicArray<char>(rows,cols);//(cols,rows);
    if (doInit) {
        //InitVisible(1); //AG130228: disable
        initMap();
    }
}

void GMap::initVisMat() {
    //ag130227: init visible mat
    DEBUG_MAP(cout<<"GMap.initVisMat "<<_rows<<"x"<<_cols<<endl;);
    _visibleMat = AllocateDynamicArray<char>((unsigned int)_rows,(unsigned int)_cols,(unsigned int)_rows,(unsigned int)_cols);
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


void GMap::initMap() {
    //initializes a map by putting zeros in all positions
    //??(except if their were obstacles)

    for(int r=0; r<_rows; r++) {
        for(int c=0; c<_cols; c++) {
            _map[r][c]=GMAP_FREE_SPACE;
        }
    }

}


//AG120416: refactored using Qt (due to problems with last line)
void GMap::readMapFile(string fileName) {
    assert(!_mapFixed); //ag130304

    DEBUG_MAP(cout << "Opening map "<<fileName<<" ... " <<flush;);    

    _name = fileName;

    ifstream reader;
    reader.open(fileName, ifstream::in);
    if (reader.good()) {

        //read first line
        string line;
        getline(reader,line);
        //int _rows=-1,_cols=-1;
        _rows=-1; _cols=-1;

        //find first comma, read #cols
        string::size_type i = line.find(",");
        if (i!=string::npos) {
            string cstr = line.substr(0,i);
            _cols = atoi(cstr.c_str());
        } else {
            cout <<"ERROR @ GMap::readMapFile: first parameter expected to be the number of columns."<<endl;
            exit(EXIT_FAILURE);
        }

        //second comma, read #rows
        string::size_type i2 = line.find(",",i+1);
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
                    _base.set(r,c);
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


void GMap::writeMapFile(string fileName) {
    assert(_mapFixed || _editMode); //ag130304

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


void GMap::writeDistanceMatrixToFile(string fileName) {
    assert(_mapFixed || _editMode); //ag130304

    cout<<"Calculating distances ... "<<flush;

    //open stream
    ofstream writer;
    writer.open(fileName);
    //time
    Timer t;
    int timerID = t.startTimer();

    //writer header
    writer << _cols << "," << _rows << ","<<endl;
    //calculate distances
    for(int r1=0; r1<_rows; r1++) {
        for(int c1=0; c1<_cols; c1++) {
            for(int r2=0; r2<_rows; r2++) {
                for(int c2=0; c2<_cols; c2++) {
                    writer << distance(r1,c1,r2,c2)<<endl;
                }
                //writer <<endl;
            }
            //writer <<endl;
        }
        //writer << endl;
    }
    //writer << endl;

    cout << "ok, took "<<t.getTime_ms(timerID)/1000<<" s"<<endl;

    writer.close();
}

//AG120416: refactored using Qt (due to problems with last line)
void GMap::readDistanceMatrixToFile(string fileName) {
    //assert(!_mapFixed); //ag130304

    DEBUG_MAP(cout << "Opening map distance file "<<fileName<<" ... " <<flush;);
    _name = fileName;

    ifstream reader;
    reader.open(fileName, ifstream::in);
    if (reader.good()) {

        //read first line
        string line;
        getline(reader,line);
        int rows=-1,cols=-1;

        //find first comma, read #cols
        string::size_type i = line.find(",");
        if (i!=string::npos) {
            string cstr = line.substr(0,i);
            cols = atoi(cstr.c_str());
        } else {
            cout <<"ERROR @ GMap::readDistanceMatrixToFile: first parameter expected to be the number of columns."<<endl;
            exit(EXIT_FAILURE);
        }

        //second comma, read #rows
        string::size_type i2 = line.find(",",i+1);
        if (i2!=string::npos) {
            string rstr = line.substr(i+1,i2-i+1);
            rows = atoi(rstr.c_str());
        } else {
            cout <<"ERROR @ GMap::readDistanceMatrixToFile: first parameter expected to be the number of rows."<<endl;
            exit(EXIT_FAILURE);
        }

        if (_rows!=rows || _cols!=cols) {
            cout <<"ERROR @ GMap::readDistanceMatrixToFile: map distance file "<<fileName<<" contains "<<rows<<"x"<<cols<<" cells, but "<<_rows<<"x"<<_cols<<" expected"<<endl;
            exit(EXIT_FAILURE);
        }

        if (_cols<=0 || _rows<=0 || _cols>MAX_COLS || _rows>MAX_ROWS) {
            cout <<"ERROR @ GMap::readDistanceMatrixToFile: #cols="<<_cols<<",#rows="<<_rows<<", expected #rows: [1,"<<MAX_ROWS
                 <<"], expected #cols: [1,"<<MAX_COLS<<"]" <<endl;
            exit(EXIT_FAILURE);
        }

        DEBUG_MAP(cout <<_rows<<" x "<<_cols<<endl<<"creating distance matrix... "<<flush;);
        _distMat = AllocateDynamicArray<float>((unsigned int)_rows,(unsigned int)_cols,(unsigned int)_rows,(unsigned int)_cols);
        DEBUG_MAP(cout <<"ok"<<"parsing ... "<<endl;);

        for(int r1=0; r1<_rows; r1++) {
            for(int c1=0; c1<_cols; c1++) {
                for(int r2=0; r2<_rows; r2++) {
                    for(int c2=0; c2<_cols; c2++) {
                        //writer << distance(r1,c1,r2,c2)<<endl;
                        getline(reader,line);
                        _distMat[r1][c1][r2][c2] = stof(line);
                        //cout <<"r"<<r1<<"c"<<c1<<"->r"<<r2<<"c"<<c2<<": '"<<line<<"'="<<d<<"="<<_distMat[r1][c1][r2][c2]<<endl;
                    }
                    //writer <<endl;
                }
                //writer <<endl;
            }
            //writer << endl;
        }
        //writer << endl;

        //close
        reader.close();



        DEBUG_MAP(cout<<"ok-dist map loaded"<<endl;);

    } else {
        cout << "ERROR @ GMap::readDistanceMatrixToFile: Map file could not be opened."<<endl;
        //exit(EXIT_FAILURE);
    }
}



void GMap::printMap(bool showDynObst, int sr,int sc, int hr, int hc) {
    //AG130228
    bool useVisib = sr>=0;


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
                if (useVisib) isInVis = !isVisible(sr,sc,r,c,showDynObst); //ag130301: use function, since it generates the visibility map

                char mapCode = _map[r][c];

                if (r==sr && c==sc) {
                    mapCode = GMAP_SEEKER;
                } else if (r==hr && c==hc) {
                    mapCode = GMAP_HIDER;
                }

                //show map item
                switch (mapCode) {
                case GMAP_FREE_SPACE:
                    if (showDynObst) {
                        for(const IDPos& pos : _dynObstVector) {
                            if (pos.equalsInt(r,c)) {
                                if (isInVis)
                                    x = 'o';
                                else
                                    x = 'O';
                            }
                        }
                    }
                    if (x!='O' && x!='o') {
                        if (isInVis)
                            x='.';
                        else
                            x=' ';
                    }
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


void GMap::printMap(const Pos& p, bool showDynObst) {
    printMap(showDynObst, p._row,p._col);
}

void GMap::printMap(const Pos& seekerPos, const Pos& hiderPos, bool showDynObst) {
    printMap(showDynObst, seekerPos._row, seekerPos._col, hiderPos._row, hiderPos._col);
}


void GMap::createPathPlanner() {
    //DEBUG_HS(cout<<"sp: in createPropDistPlanner()"<<endl;);

    if (_pathPlanner!=NULL) {
        cout << "Clearing previous path planner: "<<flush;
        delete _pathPlanner;
        cout <<"ok"<<endl;
    }

    switch(_params->pathPlannerType) {
        case SeekerHSParams::PATHPLANNER_PROPDIST:
            _pathPlanner = new PropDistPlanner(_map, _rows, _cols,8);
            break;
        case SeekerHSParams::PATHPLANNER_ASTAR:
            _pathPlanner = new AStarPlanner(_map, _rows, _cols,8);
            break;
        default:
            throw CException(_HERE_, "unkown path planner type");
            break;
    }


    _pathPlanner->setUseContinuousSpace(_params->useContinuousPos);
}

//AG TODO: for bigger maps make a 'Sparse' matrix !!
void GMap::initDistMat() {
    //ag130227: init visible mat
    _distMat = AllocateDynamicArray<float>((unsigned int)_rows,(unsigned int)_cols,(unsigned int)_rows,(unsigned int)_cols);
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


double GMap::distance(int r1,int c1,int r2,int c2) {
    DEBUG_MAP(cout<<"sp: in distance()   ";);
    DEBUG_MAP(cout<<"from ["<<r1<<","<<c1<<"] to ["<<r2<<", "<<c2<<"]"<<endl;);

    if (_pathPlanner==NULL && _distMat==NULL) {
        createPathPlanner(); //ag130304: create pathplanner if not exists
    }

    //ag130617: check if caching here or in pathplanner
    if (_distMat==NULL && _pathPlanner->doesCacheResults()) {
        return _pathPlanner->distance(c1,r1,c2,r2);
    } else {
        //ag130509: cache distsance results
        if (_distMat==NULL) {
            initDistMat();
        }

        if (_distMat[r1][c1][r2][c2]==DMAT_UNSET) {
            _distMat[r1][c1][r2][c2] = (float)_pathPlanner->distance(c1,r1,c2,r2);
            //AG130615: also opposite
            _distMat[r2][c2][r1][c1] = _distMat[r1][c1][r2][c2];
        }

        return _distMat[r1][c1][r2][c2];
    }
}

double GMap::distance(const Pos &p1, const Pos &p2) {
    return distance(p1._row,p1._col,p2._row,p2._col);
}

double GMap::distanceEuc(const Pos &p1, const Pos &p2) const {
    if (_params->useContinuousPos) {
        return sqrt(pow(p1._rowDouble-p2._rowDouble, 2) + pow(p1._colDouble-p2._colDouble, 2));
    } else {
        return sqrt(pow(p1._row-p2._row, 2) + pow(p1._col-p2._col, 2));
    }
}

/*constexpr double GMap::distanceEuc(double r1, double c1, double r2, double c2)  {
    return sqrt(pow(r1-r2,2) + pow(c1-c2,2));
}*/

double GMap::distanceCont(double r1, double c1, double r2, double c2) {
    //first distance asuming calculation is from center
    int r1i = (int)r1, c1i = (int)c1, r2i = (int)r2, c2i = (int)c2; //, rni = -1, cni = -1;

    double d = distance(r1i,c1i,r2i,c2i);
    //now check if less than 2, then we use eucledian distance

    //get next step to goal
    /*_pathPlanner->nextStep(c1i,r1i,c2i,r2i, &cni, &rni);
    cout <<"next step: r"<<rni<<"c"<<cni<<endl;*/

    //TODO: calc real continuous distance
    //until now we can use int distance
    /*
     Measured distance: 1 (grid cell)
     Worst case, worst error:

      +---+---b
      |   |   |
      +---+---+
      |   |   |
      a---+---+

     if real position are a and b -> distance, the max error is sqrt(8)-sqrt(2)=1.414

     general case:

     error: sqrt(2*(n+1)^2) - sqrt(2*n^2) = sqrt(2)

     */


    return d;
}


double GMap::distanceCont(const Pos &p1, const Pos &p2) {
    return distanceCont(p1._rowDouble,p1._colDouble,p2._rowDouble,p2._colDouble);
}

int GMap::numFreeCells() {

    return _rows*_cols - numObstacles();
}

//returns if is obstacle
bool GMap::isObstacle(int r, int c) const {
    assert(IS_POS_IN_MAP(r,c));
    return _map[r][c]==GMap::GMAP_OBSTACLE;
}
bool GMap::isObstacle(const Pos& pos) const  {
    assert(IS_POS_IN_MAP(pos._row,pos._col));
    return _map[pos._row][pos._col]==GMap::GMAP_OBSTACLE;
}
bool GMap::isObstacle(double r, double c) const {
    assert(IS_POS_IN_MAP(r,c));
    return _map[(int)floor(r)][(int)floor(c)]==GMap::GMAP_OBSTACLE;
}

//ag120418: check if is base
bool GMap::isBase(int r, int c) const  {
    assert(IS_POS_IN_MAP(r,c));
    return (_base._row==r && _base._col==c);
}

bool GMap::isBase(const Pos& p) const {
    return p._row==_base._row && p._col==_base._col;
}

//ag120227: create index map
void GMap::createIndexMap() {
    assert(_rows>0 && _cols>0 && _map!=NULL);

    DEBUG_MAP(cout<<"GMap.createIndexMap:"<<endl;);

    if (_indexMap==NULL) {
        _indexMap = AllocateDynamicArray<int>((unsigned int)_rows,(unsigned int)_cols);
    }
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
    //AG130301: require user to fix map first
    assert(_mapFixed || _editMode);

    if (_indexMap==NULL || _editMode) {
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
bool GMap::isVisible(int r1, int c1, int r2, int c2, bool checkDynObst) {
    assert(_mapFixed);
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

    bool isVisib = (_visibleMat[r1][c1][r2][c2]==VMAT_VIS);

    if (checkDynObst && isVisib && _dynObstVector.size()>0) {
        //AG140525: check if visible due to dyn. obst
        if (_visibleMatDynObst==NULL) {
            _visibleMatDynObst = AllocateDynamicArray<char>(_rows,_cols);
        }

        if (!_lastVisibOrigPos.isSet() || !_lastVisibOrigPos.equalsInt(r1,c1)) {
            //set all 'visible' to unset and set original obstacles
            FOR(r,_rows) {
                FOR(c,_cols) {
                    if (_visibleMat[r1][c1][r][c]==VMAT_OBST) {
                        _visibleMatDynObst[r][c] = VMAT_OBST;
                    } else {
                        _visibleMatDynObst[r][c] = VMAT_UNSET;
                    }
                }
            }
            //add dynamical obstacles
            for(const IDPos& pos : _dynObstVector) {
                _visibleMatDynObst[pos.row()][pos.col()] = VMAT_OBST;
            }

            //set this pos as last
            _lastVisibOrigPos.set(r1,c1);
        }
        if (_visibleMatDynObst[r2][c2] == VMAT_UNSET) {
            //do visib check
            char vis = raytrace(_visibleMatDynObst, r1,c1,r2,c2);
        }

        isVisib = (_visibleMatDynObst[r2][c2]==VMAT_VIS);
    }

    return isVisib;
}

bool GMap::isVisible(double r1, double c1, double r2, double c2, bool checkDynObst) {
    return isVisible((int)floor(r1),(int)floor(c1),(int)floor(r2),(int)floor(c2), checkDynObst);
}

bool GMap::isVisible(const Pos& p1, const Pos& p2, bool checkDynObst) {
    return isVisible(p1._row,p1._col,p2._row,p2._col, checkDynObst);
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


vector<Pos> GMap::getInvisiblePoints(const Pos& p, bool checkDynObst) {
    return getInvisiblePoints(p._row, p._col,checkDynObst);
}

vector<Pos> GMap::getInvisiblePoints(const Pos& p1, const Pos& p2, bool checkDynObst) {
    return getInvisiblePoints(p1._row, p1._col,p2._row, p2._col,checkDynObst);
}

vector<Pos> GMap::getInvisiblePoints(int r, int c, bool checkDynObst) {
    assert(r>=0 && c>=0 && r<_rows && c<_cols);

    vector<Pos> posVector;
    FORs(r2,_rows) {
        FORs(c2,_cols) {
            if ( _map[r2][c2]!=GMAP_OBSTACLE && !isVisible(r,c,r2,c2,checkDynObst)) {
                posVector.push_back(Pos(r2,c2));
            }
        }
    }

    return posVector; //TODO TEST AND FINISH GMAP (remove _visible)
}

vector<Pos> GMap::getInvisiblePoints(int r1, int c1, int r2, int c2, bool checkDynObst) {
    //AG150126: get invisible points from 2 locations
    assert(r1>=0 && c1>=0 && r1<_rows && c1<_cols);
    assert(r2>=0 && c2>=0 && r2<_rows && c2<_cols);

    vector<Pos> posVector;
    FORs(r,_rows) {
        FORs(c,_cols) {
            if ( _map[r][c]!=GMAP_OBSTACLE && !isVisible(r1,c1,r,c,checkDynObst) && !isVisible(r2,c2,r,c,checkDynObst)) {
                posVector.push_back(Pos(r,c));
            }
        }
    }

    return posVector;
}

bool GMap::isPosInMap(const Pos &pos) const {
    return IS_POS_IN_MAP(pos._rowDouble,pos._colDouble);//pos.row>=0 && pos.row<_rows && pos.col>=0 && pos.col<_cols;
}

bool GMap::isPosInMap(int r, int c) const {
    return IS_POS_IN_MAP(r,c); // r>=0 && r<_rows && c>=0 && c<_cols;
}

bool GMap::isPosInMap(double r, double c) const {
    return IS_POS_IN_MAP(r,c); // r>=0 && r<_rows && c>=0 && c<_cols;
}



void GMap::setEditMode(bool b) {
    _editMode = b;
}

bool GMap::getEditMode() const {
    return _editMode;
}



int GMap::rowCount() const {
    return _rows;
}
int GMap::colCount() const {
    return _cols;
}
int GMap::width() const {
    return _cols;
}
int GMap::height() const {
    return _rows;
}

Pos GMap::getBase() const {
    return _base;
}

bool GMap::isMapFixed() const {
    return _mapFixed;
}

char GMap::getItem(int row, int col) const {
    assert(row>=0 && row<_rows);
    assert(col>=0 && col<_cols);

    return _map[row][col];
}

void GMap::setItem(int row, int col, char c) {
    assert(row>=0 && row<_rows);
    assert(col>=0 && col<_cols);
    assert(c==GMAP_BASE || c==GMAP_FREE_SPACE || c==GMAP_OBSTACLE);

    if (c==GMAP_BASE) {
        setBase(row,col);
    } else {
        _map[row][col] = c;
    }
}



void GMap::setBase(int r, int c) {
    assert(!_mapFixed || _editMode); //ag130301: not allowed to change map when set fixed
    _base.set(r,c);
    //AG140423: added check for base is set, if we don't play H&S it CAN be without base
    if (_base.isSet()) _map[r][c]=GMAP_BASE;
}

void GMap::setBase(const Pos& base) {
    setBase(base._row,base._col);
}

void GMap::addObstacle(int r, int c) {
    assert(!_mapFixed || _editMode); //ag130301: not allowed to change map when set fixed
    assert(r>=0 && r<_rows);
    assert(c>=0 && c<_cols);

    _map[r][c]=GMAP_OBSTACLE;
    //_visible[x][y]=2;
    _obstacleVector.push_back(Pos(r,c)); //ag130301: added obstacle
}

void GMap::addObstacle(const Pos &p) {
    addObstacle(p._row,p._col);
}

void GMap::deleteItem(int x, int y) {
    if (_base._row==x &&_base._col==y)
        _map[x][y]=GMAP_BASE;
    else
        _map[x][y]=GMAP_FREE_SPACE;
}

double GMap::getDirection(const Pos &p1, const Pos &p2) const {
    //get direction and give it as goal
    double rd = p2.rowDouble() - p1.rowDouble();
    double cd = p2.colDouble() - p1.colDouble();

    static const double M_5PI_2 = 2*M_PI + M_PI_2;

    double ang = atan2(rd,cd);

    if (ang>=-M_PI_2) {
        ang += M_PI_2;
    } else {
        ang += M_5PI_2;
    }

    return ang;
}

Pos  GMap::tryMoveDir(double dir, Pos pos, double dist, bool visibCheck) {
    //use continuous
    if (!_params->useContinuousPos) pos.add(0.5,0.5);

    //calculate new pos
    Pos newPos(pos.rowDouble()-cos(dir)*dist, pos.colDouble()+sin(dir)*dist);

    //if legal position, not an obstacle and visible then we can reach it
    if (!isPosInMap(newPos) || isObstacle(newPos) || (visibCheck && !isVisible(pos,newPos,false)) //AG140526 note: the dyn. obst is NOT used as obstacle in nav.
#ifdef DYN_OBST_COLL
             || (_params->minDistToDynObstacle>0 && tooCloseToWithDynObst(newPos))
#endif
            ) {
        //we can't move here
        newPos.clear();
    } else if (!_params->useContinuousPos) {
        newPos.convertValuesToInt();
    }

    return newPos;
}

Pos  GMap::tryMoveDirStep(double dir, Pos pos, double dist, double stepSize, bool visibCheck) {
    assert(stepSize>0);

    //use continuous
    if (!_params->useContinuousPos) pos.add(0.5,0.5);

    //calculate new pos
    Pos newPos;
    bool continueSearch = false;
    bool triedCellCenter = false;

    do {
        //set new pos
        newPos.set(pos.rowDouble()-cos(dir)*dist, pos.colDouble()+sin(dir)*dist);
        //reduce dist
        dist -= stepSize;

        continueSearch = (!isPosInMap(newPos) || isObstacle(newPos) || (visibCheck && dist>1 && !isVisible(pos,newPos,false)) )
                //AG140523: added dist>1 in case of visibility check, because if already checked that it is not an obstacle,                
                //          and it is at maximum at distance 1, than we should be able to go there.
                //AG140526: dyn. obst. not taken into account for movement
#ifdef DYN_OBST_COLL
                || (_params->minDistToDynObstacle>0 && tooCloseToWithDynObst(newPos))
#endif
                ;

        if (continueSearch && !triedCellCenter) {
            /* AG140526: first try if can't move is to move the start position to the center of the cell.
                This works for following case:
                             G
                            SXXX

                where S=seeker, G=goal, X=obstacle
                When S is in the right-down corner of it's 'cell' and the pathplanner returns G as next step,
                then this function trys to move distance 1 in that direction, but this, with a direction of 45º is sqrt(0.5) movement in row and col
                which is a pos inside the obstacle.
              */
            pos.convertValuesToInt();
            pos.add(0.5,0.5);
        }

    } while (dist>0 && continueSearch);

    if (continueSearch) {
        DEBUG_CLIENT(cout <<"GMap::tryMoveDirStep: WARNING: from "<<pos.toString()<<" in dir. "<<(180*dir/M_PI)
                          <<"º could not be executed, returning same pos!"<<endl;);
        //not a legal pos
        newPos = pos;
    } else if (!_params->useContinuousPos) {
        newPos.convertValuesToInt();
    }

    return newPos;
}

Pos  GMap::tryMove(int action, Pos pos) {
    //AG130311: asserts
    assert(pos._row>=0 && pos._row<_rows && pos._col>=0 && pos._col<_cols);    
    assert(action>=0 && action<HSGlobalData::NUM_ACTIONS);

    if (action==HSGlobalData::ACT_H) {
        return pos;
    }


    //get new pos
    Pos newPos;

    if (!_params->useContinuousPos) {
        if (_actionsPosMat == NULL) { //AG140122: can't use buffer when having continuous pos !!
            //init matrix for caching
            _actionsPosMat = AllocateDynamicArray<Pos>((unsigned int)_rows,(unsigned int)_cols, (unsigned int) (HSGlobalData::NUM_ACTIONS-1));
            for (int r=0; r<_rows; r++) {
                for (int c=0; c<_cols; c++) {
                    for (int a=0; a<HSGlobalData::NUM_ACTIONS-1; a++) {
                        _actionsPosMat[r][c][a]._row = -2;
                    }
                }
            }
        }

        newPos = _actionsPosMat[pos._row][pos._col][action-1];
    }

    if (_params->useContinuousPos || newPos._row==-2) {
        //not yet calculated
        switch(action) {
        case HSGlobalData::ACT_N: {
            if (_params->useContinuousPos) {
                newPos.set(pos._rowDouble-1.0,pos._colDouble);
            } else {
                newPos.set(pos._row-1,pos._col);
            }
            if(newPos._row<0) {
                DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
                newPos.clear();
                break;
            }
            if(_map[newPos._row][newPos._col]==GMAP_OBSTACLE) { // ->isObstacle(newPos)
                DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
                newPos.clear();
                break;
            }
        }
        break; // move north
        case HSGlobalData::ACT_NE: {
            if (_params->useContinuousPos) {
                newPos.set(pos._rowDouble-M_SQRT1_2,pos._colDouble+M_SQRT1_2);
            } else {
                newPos.set(pos._row-1,pos._col+1);
            }
            if( (newPos._row<0) ||(newPos._col>=_cols) ) {
                DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
                newPos.clear();
                break;
            }
            if(_map[newPos._row][newPos._col]==GMAP_OBSTACLE) {
                DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
                newPos.clear();
                break;
            }
        }
        break; // move north-east
        case HSGlobalData::ACT_E: {
            if (_params->useContinuousPos) {
                newPos.set(pos._rowDouble,pos._colDouble+1.0);
            } else {
                newPos.set(pos._row,pos._col+1);
            }
            if(newPos._col>=_cols) {
                DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
                newPos.clear();
                break;
            }
            if(_map[newPos._row][newPos._col]==GMAP_OBSTACLE) {
                DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
                newPos.clear();
                break;
            }
        }
        break; // move east
        case HSGlobalData::ACT_SE: {
            if (_params->useContinuousPos) {
                newPos.set(pos._rowDouble+M_SQRT1_2,pos._colDouble+M_SQRT1_2);
            } else {
                newPos.set(pos._row+1,pos._col+1);
            }
            if( (newPos._row>=_rows) ||(newPos._col>=_cols) ) {
                DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
                newPos.clear();
                break;
            }
            if(_map[newPos._row][newPos._col]==GMAP_OBSTACLE) {
                DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
                newPos.clear();
                break;
            }
        }
        break; // move east-south
        case HSGlobalData::ACT_S: {
            if (_params->useContinuousPos) {
                newPos.set(pos._rowDouble+1.0, pos._colDouble);
            } else {
                newPos.set(pos._row+1, pos._col);
            }
            if( newPos._row >= _rows ) {
                DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
                newPos.clear();
                break;
            }
            if(_map[newPos._row][newPos._col]==GMAP_OBSTACLE) {
                DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
                newPos.clear();
                break;
            }
        }
        break; // move south
        case HSGlobalData::ACT_SW: {
            if (_params->useContinuousPos) {
                newPos.set(pos._rowDouble+M_SQRT1_2, pos._colDouble-M_SQRT1_2);
            } else {
                newPos.set(pos._row+1, pos._col-1);
            }
            if( (newPos._row>=_rows) ||(newPos._col<0) ) {
                DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
                newPos.clear();
                break;
            }
            if(_map[newPos._row][newPos._col]==GMAP_OBSTACLE) {
                DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
                newPos.clear();
                break;
            }
        }
        break; // move south-west
        case HSGlobalData::ACT_W: {
            if (_params->useContinuousPos) {
                newPos.set(pos._rowDouble,pos._colDouble-1.0);
            } else {
                newPos.set(pos._row,pos._col-1);
            }
            if(newPos._col<0) {
                DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
                newPos.clear();
                break;
            }
            if(_map[newPos._row][newPos._col]==GMAP_OBSTACLE) {
                DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
                newPos.clear();
                break;
            }
        }
        break; // move west
        case HSGlobalData::ACT_NW: {
            if (_params->useContinuousPos) {
                newPos.set(pos._rowDouble-M_SQRT1_2,pos._colDouble-M_SQRT1_2);
            } else {
                newPos.set(pos._row-1,pos._col-1);
            }
            if( (newPos._row<0) ||(newPos._col<0) ) {
                DEBUG_PLAYER(cout<<"out of borders!"<<endl;);
                newPos.clear();
                break;
            }
            if(_map[newPos._row][newPos._col]==GMAP_OBSTACLE) {
                DEBUG_PLAYER(cout<<"there is a wall there!"<<endl;);
                newPos.clear();
                break;
            }
        }
        break; // move west-north
        case HSGlobalData::ACT_H: {
            DEBUG_PLAYER(cout<<"the position of the player is: ("<<pos.toString()<<")"<<endl;);
            newPos = pos;
        }
        break; //don´t move
        default:
            cout << " (unknown command)";
        }
	DEBUG_PLAYER(cout << endl;);
    if (!_params->useContinuousPos) _actionsPosMat[pos._row][pos._col][action-1] = newPos;
    }
#ifdef DYN_OBST_COLL
    if (_params->minDistToDynObstacle>0 && tooCloseToWithDynObst(newPos)) {
        //too close, so don't allow
        newPos.clear();
    }
#endif
    return newPos;
}

void GMap::setUsingContActions(bool b) {
    assert(_params->useContinuousPos==b);//AG140509: now all through params, but we need to update base..
    //_params->useContinuousPos = b;
    if (_pathPlanner != NULL) _pathPlanner->setUseContinuousSpace(b);
    if (b && _base._row==_base._rowDouble && _base._col==_base._colDouble) {
        //set base to be centre of cell
        _base.add(0.5,0.5);
    }
}


#ifdef USE_QT

void GMap::writeMaptoStream(QDataStream& out) {
    //name
    out << QString::fromStdString(_name);
    //size
    out << (quint16)_rows;
    out << (quint16)_cols;
    //obstacles
    out << (quint32) numObstacles();

    for(vector<Pos>::iterator it = _obstacleVector.begin(); it != _obstacleVector.end(); it++) {
        out << (quint16)it->_row;
        out << (quint16)it->_col;
    }
    //base
    out << (qint16)_base._row;
    out << (qint16)_base._col;
}

void GMap::readMapFromStream(QDataStream& in) {
    quint16 mrows,mcols,orow,ocol;
    qint16 brow,bcol;
    quint32 mobst;

    //name
    QString name;
    in >> name;
    _name = name.toStdString();
    //size
    in >> mrows;
    in >> mcols;
    _rows = (int)mrows;
    _cols = (int)mcols;

    //reserve map space
    createMap(_rows,_cols,true);

    //obstacles
    in >> mobst;

    for(unsigned int i=0; i<mobst; i++) {
        in >> orow;
        in >> ocol;
        //add to map
        addObstacle(orow,ocol);
    }

    //base
    in >> brow;
    in >> bcol;

    setBase(brow,bcol);

    setMapFixed();
}

#endif


int GMap::getIndexFromCoord(const Pos& p) {
    return getIndexFromCoord(p._row,p._col);
}


Pos GMap::getObstacle(int i) const { //get the pos of the "i" obstacle
    return _obstacleVector[i];//ag130301: changed to vector //_obstacles[i];
}

//! Count # obstacles
int GMap::numObstacles() const {
    return _obstacleVector.size(); //ag130301: changed to vector //_obstnum;
}

string GMap::getName() const {
    return _name;
}


Pos GMap::genRandomPos() const {
    //todo: we can use a fixed random number generator, which in theo
    Pos ranPos;

    do {
        if (_params->useContinuousPos) {
            ranPos.set(randomDouble(0,_rows), randomDouble(0,_cols));
        } else {
            ranPos.set(random(0,_rows-1), random(0,_cols-1));
        }
    } while (!isPosInMap(ranPos) || isObstacle(ranPos));

    return ranPos;
}

void GMap::setDynamicObstacles(const std::vector<IDPos> &dynObstVec) {
    _dynObstVector = dynObstVec;
    _lastVisibOrigPos.clear(); //reset visib matrix for dyn. obst
}

void GMap::addDynamicObstacle(IDPos p) {
    _dynObstVector.push_back(p);
    _lastVisibOrigPos.clear(); //reset visib matrix for dyn. obst    
}

vector<IDPos> GMap::getDynamicObstacles() const {
    return _dynObstVector;
}

void GMap::removeDynamicObstacles() {
    _dynObstVector.clear();
}

#ifdef DYN_OBST_COLL
bool GMap::tooCloseToWithDynObst(const Pos &pos) const {
    assert(pos.isSet());

    bool tooClose = false;
    //check if any pos too close
    for(const Pos& doPos : _dynObstVector) {
        assert(doPos.isSet());
        if (pos.distanceEuc(doPos) < _params->minDistToDynObstacle) {
            tooClose = true;
            break;
        }
    }

    return tooClose;
}
#endif


PathPlanner* GMap::getPathPlanner() {
    return _pathPlanner;
}

Pos GMap::getNextStep(const Pos &p1, const Pos &p2) {
    if (_pathPlanner==NULL) {
        createPathPlanner();
    }

    return _pathPlanner->nextStep(p1, p2);
}


#ifdef GMAP_CAN_DELETE
void GMap::deleteRows(int startRow, int endRow) {
    cout<<"GMap::deleteRows: deleting rows "<<startRow<<"-"<<endRow<<endl;

    int delRows = endRow-startRow+1;
    int newRows = _rows - delRows;
    char** newMap = AllocateDynamicArray<char>((unsigned int)newRows, (unsigned int)_cols);

    for(int r=0; r<newRows; r++) {
        int nr = r;
        if (r>=startRow)
            nr += delRows;
        for(int c=0; c<_cols; c++) {
            newMap[r][c] = _map[nr][c];
        }
    }

    FreeDynamicArray<char>(_map, (unsigned int)_rows);
    _map = newMap;
    _rows = newRows;
}

void GMap::deleteCols(int startCol, int endCol) {
    cout<<"GMap::deleteCols: deleting cols "<<startCol<<"-"<<endCol<<endl;

    int delCols = endCol-startCol+1;
    int newCols = _cols - delCols;

    char** newMap = AllocateDynamicArray<char>((unsigned int)_rows, (unsigned int)newCols);

    for(int c=0; c<newCols; c++) {
        int nc = c;
        if (c>=startCol)
            nc += delCols;
        for(int r=0; r<_rows; r++) {
            newMap[r][c] = _map[r][nc];
        }
    }

    FreeDynamicArray<char>(_map, (unsigned int)_rows);
    _map = newMap;
    _cols = newCols;
}
#endif





//--- unit tests ---

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
                            b1 = isVisible(r1,c1,r2,c2,false);
                            b2 = isVisible(r2,c2,r1,c1,false);
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
