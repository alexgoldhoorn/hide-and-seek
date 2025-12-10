#include "Segment/segment.h"
#include "Utils/generic.h"

#include <map>
#include <cmath>
#include <cassert>
#include <sstream>
#include <iostream>


using namespace std;


Segmenter::Segmenter(GMap* gmap, int neighb) {
    _gmap = gmap;
    _neighb = neighb;
    //index map required
    //_gmap->createIndexMap(); //AG130301: done on first call
    _rowPos = _colPos = -1;
}

Segmenter::Segmenter(int neighb) {
    _neighb = neighb;
    _rowPos = _colPos = -1;
}

Segmenter::~Segmenter() {
}

void Segmenter::setGMap(GMap* gmap) {
    _gmap = gmap;
    //_gmap->createIndexMap(); //AG130301: already done on first call
    _rowPos = _colPos = -1;
}

vector<int>* Segmenter::newRegVector(int size, bool init0) {
    //DEBUG_SEGMENT(cout << "new reg vector size: "<<size<<flush;);
    vector<int>* vecnew = new vector<int>();
    //cout << "ok"<<flush;
    vecnew->resize(size);

    if (init0) {
        for (unsigned int i=0; i<vecnew->size(); i++) {
            (*vecnew)[i] = 0;
        }
    }

    return vecnew;
}

/*void showintvect(vector<int>* vec,string name) {
    cout << name<<"vec,sz="<<vec->size()<<": "<<flush;
    for (unsigned int i=0; i<vec->size(); i++) {
        int v = (*vec)[i];
        DEBUG_SEGMENT(cout <<v<<", "<<flush;);
    }
    cout <<endl;
}*/


vector<int>* Segmenter::connectionAnalysis(vector<int>* vecSeg, bool ignoreObstacles) {
    //assert(_vecSeg != NULL && _vecSeg->size()>0 & _gmap!=NULL);
    DEBUG_SEGMENT(cout << "Segmenter.connectionAnalysis: "<<endl;);
    DEBUG_SEGMENT(cout << " ignoring obstacles: "<<(ignoreObstacles?"yes":"no")<<endl;);

    //generate new empty region map
    vector<int>* vecCA = newRegVector(vecSeg->size(),true);
    //showintvect(vecCA,"init");


    //# regions
    int regs = 0;
    //index used by vector (not vector states skip the obstacles)
    int i = 0;

    /*first loop up-down,left-right:
        only look at upper line (1 or 3 neighbours depending 4/8
        neighbourhood), and left neighbour
      - if map value same -> get same region as neighbour, getting lowest
      - if no neighbour with same -> new region
    */
    DEBUG_SEGMENT(cout << " loop 1:"<<endl;);
    for (int r=0; r<_gmap->rowCount(); r++) {
        for (int c=0; c<_gmap->colCount(); c++) {
            //check if obstacle
            if (!ignoreObstacles && _gmap->isObstacle(r,c)) {
                DEBUG_SEGMENT(cout << "[X]";);
                continue;
            }

            int r1 = r-1;
            int c1 = 0;
            //first upper row
            if (r1>=0) {                
                for (c1=c-1; c1<=c+1; c1++) {
                    caChangeItem(vecCA,vecSeg,r,c,r1,c1,i,ignoreObstacles);
                }
            }

            //now left cell
            r1 = r;
            c1 = c-1;
            if (c1>=0) {
                caChangeItem(vecCA,vecSeg,r,c,r1,c1,i,ignoreObstacles);
            }

            if ((*vecCA)[i]==0) {
                (*vecCA)[i] = ++regs;
            }

            DEBUG_SEGMENT(cout << "["<<(*vecCA)[i]<<"]";);

            i++;
        }
        DEBUG_SEGMENT(cout << endl;);
    }

    //showintvect(vecCA,"after loop1");

    //reset i
    i=0;

    DEBUG_SEGMENT(cout << " regs="<<regs<<endl<<" loop 2 (final):"<<endl;);
    //second loop: ceck all cells, all neighbours, get lowest
    for (int r=0; r<_gmap->rowCount(); r++) {
        for (int c=0; c<_gmap->colCount(); c++) {
            //check if obstacle
            if (_gmap->isObstacle(r,c)) {
                DEBUG_SEGMENT(cout << "[X]";);
                continue;
            }

            //loop all neighbours
            for (int r1=r-1; r1<=r+1; r1++) {
                for (int c1=c-1; c1<=c+1; c1++) {
                    caChangeItem(vecCA,vecSeg,r,c,r1,c1,i,ignoreObstacles);
                }
            }

            DEBUG_SEGMENT(cout << "["<<(*vecCA)[i]<<"]";);
            i++;
        }
        DEBUG_SEGMENT(cout<<endl;);
    }
    //showintvect(vecCA,"after loop2");


    return vecCA;

}


//used by connectionAnalysis
void Segmenter::caChangeItem(vector<int>* newRegVec, vector<int>* vecSeg, int r, int c, int r1, int c1, int i, bool ignoreObstacles) {
    //cout << " cachange: r "<<r<<", c "<<c<<", r1 "<<r1<<", c1 "<<c1<<", i "<<i<<flush;
    if (r1>=0 && r1<_gmap->rowCount() && c1>=0 && c1<_gmap->colCount() && //check coords
          (_neighb==8 || r1==r || c1==c) && //check neighbourhood
          (ignoreObstacles || !_gmap->isObstacle(r1,c1))) //neighbour is not an obstacle?
    {
        //get index of 2nd neighbour
        int i1 = -1;
        if (ignoreObstacles) {
            i1 = r1*_gmap->width() + c1;
        } else {
            i1 = _gmap->getIndexFromCoord(r1,c1);
        }
        //cout <<",i1 "<<i1<<flush;///

        if (
          (*newRegVec)[i1]>0 && //neighb has region
          (*vecSeg)[i]==(*vecSeg)[i1] && //same region
          ((*newRegVec)[i]==0 || (*newRegVec)[i1]<(*newRegVec)[i]) // get min region number
          ) {
                //cout <<" new:"<<flush;///
                //set region equal to neighbour
                (*newRegVec)[i] = (*newRegVec)[i1];
                //cout << (*newRegVec)[i1]<<endl;///

            }

    }
    //cout <<"  OK"<<endl;
}

//based on an integer vector, it changes segment ids such that the segment numbering
//starts at 0 and runs from there (not skipping numbers)
//The segmentCount returns the number
vector<int>* Segmenter::get0BasedSegments(vector<int>* vec, int &segmentCount) {
    DEBUG_SEGMENT(cout << "Segmenter.get0BasedSegments: "<<endl;);

    //map of old -> new ids
    map<int,int> oldNewIDMap;
    //new vector of 'state' i being in (new) region j: [i] -> j
    vector<int>* newVec = new vector<int>(vec->size());
    //iterator for fincing
    map<int,int>::iterator newIDFinder;
    //count of # segements
    int segments=0;

    /* Check all calculated segments and map them to a new segment ID, starting at 0.
      */


    //cout << "vec-siz="<<vec->size()<<endl;

    FOR(i,vec->size()) {
        //cout << " "<<i<<") "<<flush;///

        int oldReg = (*vec)[i];
        //cout <<"oldreg="<<oldReg<<flush;///

        //check if there is already a new segment id for the old id
        newIDFinder = oldNewIDMap.find(oldReg);

        if (newIDFinder==oldNewIDMap.end()) {
            //cout << " not found "<<oldReg<<", new: "<<flush; ///
            //segment id not found yet -> add as new
            //oldNewIDMap.insert(pair<int,int>(vec[i],segments));
            oldNewIDMap[oldReg] = segments;
            //update id in new map
            (*newVec)[i] = segments;

            //cout<<segments<<flush;///
            segments++;

        } else {
            //update id
            (*newVec)[i] = oldNewIDMap[oldReg];
            //cout << "found,reg="<<oldNewIDMap[oldReg]<<flush;///
        }

        DEBUG_SEGMENT(cout << " " <<oldReg << "->"<<(*newVec)[i]<<", "<<flush;);
    }
    DEBUG_SEGMENT(cout<<"|"<<endl;);

    segmentCount = segments;

    return newVec;
}


void Segmenter::showMap(vector<int> *segVec, int xState, int yState) {
    assert(_gmap!=NULL);
    int maxSeg = 0;
    for(unsigned int i=0;i<segVec->size();i++) {
        if ( (*segVec)[i] > maxSeg ) maxSeg = (*segVec)[i];        
    }

    if (maxSeg==0) {
        cout << "[showMap: no segmentations]"<<endl;
        return;
    }

    //get num chars needed to print
    int numChars = (int)floor(log10(maxSeg))+1;

    //obst string
    string obsStr;
    obsStr.resize(numChars,'X');    
    int i = 0;

    for (int r=0; r<_gmap->rowCount(); r++) {
        for (int c=0; c<_gmap->colCount(); c++) {
            //check if obstacle
            if (_gmap->isObstacle(r,c)) {
                //obst
                cout << '['<< obsStr <<']';
            } else {
                //not obst
                stringstream ss;
                ss<<(*segVec)[i];
                string s=ss.str();
                s.resize(numChars,' ');

                //type of cell
                char s1='[',s2=']';
                if (i==xState) {
                    //seeker pos
                    s1=s2='S';
                }
                if (i==yState) {
                    s2='H';
                    if (s1!='S') s1='H';
                }
                if (_gmap->isBase(r,c)) {
                    if ((s1=='[' && s2==']') || s1==s2) {
                        s2 = 'B';
                        if (s1=='[') s1='B';
                    } else {
                        s1=s1-('A'-'a');
                        s2=s2-('A'-'a');
                    }
                }

                cout << s1<< s <<s2;
                i++;
            }
        }
        cout<<endl;
    }
    cout << endl;
}


//AG TODO: this doesn't work correctly!
/*int Segmenter::countConnectedObstacles() {
    int count = _gmap->numObstacles();
    if (count==0) return 0;

    vector<int>* vec = newRegVector(_gmap->rowCount()*_gmap->colCount(), true);

    //generate vector of the map (0=free,1=obstacle)
    FOR(r,_gmap->rowCount()) {
        FOR(c,_gmap->colCount()) {
            if (_gmap->isObstacle(r,c)) {
                vec->push_back(1);
            } else {
                vec->push_back(0);
            }
        }
    }

    //do connection analysis to find connected obstacles
    vector<int>* vecCon = connectionAnalysis(vec, true);

    //now count number of connected components
    //int count=-1;
    //vector<int>* vecCon0 = get0BasedSegments(vecCon,count);

    //now count obstacles
    int i = 0;
    count = 0;
    vector<int> obstIndxVec;
    vector<int>::iterator res;
    FOR(r,_gmap->rowCount()) {
        FOR(c,_gmap->colCount()) {

            if (_gmap->isObstacle(r,c)) {
                //check if already seen this segment
                int s = (*vecCon)[i];
                cout << s << " ";
                res = find(obstIndxVec.begin(),obstIndxVec.end(),s);
                if (res==obstIndxVec.end()) {
                    //new obstacle found
                    count++;
                    obstIndxVec.push_back(s);
                }
            } else cout << "  ";

            i++;
        } cout << endl;
    }



    delete vec;
    delete vecCon;

    return count;
}
*/

void Segmenter::setPosition(int row, int col) {
    _rowPos = row;
    _colPos = col;
}

Pos Segmenter::getPosition() {
    return Pos(_rowPos,_colPos);
}
