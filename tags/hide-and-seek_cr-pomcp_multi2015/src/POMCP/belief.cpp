#include "POMCP/belief.h"

#include <cassert>
#include <algorithm>
#include <sstream>
#include <algorithm>

#include "Utils/generic.h"

//for gen of hist
#include "POMCP/hsstate.h"
#include <iomanip>

#include <iostream>

using namespace pomcp;
using namespace std;



Belief::Belief()  {
    _beliefProbMatrix = NULL;
    _beliefProbMatrixRows = _beliefProbMatrixCols = 0;
    _beliefProbMatrixUptodate = false;
}

Belief::~Belief() {
    //delete samples
    for(vector<State*>::iterator it = _sampleVector.begin(); it!=_sampleVector.end(); it++) {
        delete *it;
    }

    //delete hist
    if (_beliefProbMatrix!=NULL) {
        FreeDynamicArray<double>(_beliefProbMatrix, _beliefProbMatrixRows);
    }
}

size_t  Belief::size() const {
    return _sampleVector.size();
}


State* Belief::operator[](size_t i){
    /*assert(i>=0 && i<_sampleMap.size());*/

    //get item
    return _sampleVector[i];
}

void Belief::add(State* state, bool deleteStatesAfterUsage) {
    if (deleteStatesAfterUsage) {
        _sampleVector.push_back(state);
    } else {
        _sampleVector.push_back(state->copy());
    }

    //AG140409: the belief matrix is not correct anymore
    _beliefProbMatrixUptodate = false;
}

void Belief::addAll(vector<State *>& states, bool deleteStatesAfterUsage) {
    for(vector<State*>::iterator it = states.begin(); it!=states.end(); it++) {
        add(*it, deleteStatesAfterUsage);

        //ag131104: delete since the vector was temporal to pass the data
        //AG140502: instead of deleting, I simply prevent it from making a copy
                //WARNING!! THIS IS NOT SAFE, but should be more efficient
        //if (deleteStatesAfterUsage) delete *it;
    }
}

void Belief::remove(size_t i) {
    //assert(i>=0 && i<_sampleMap.size());

    //first delete it
    delete _sampleVector[i];

    //then erase from vector
    _sampleVector.erase(_sampleVector.begin() + i);

    //AG140409: the belief matrix is not correct anymore
    _beliefProbMatrixUptodate = false;
}

void Belief::remove(State *state) {
    //get item in vector
    std::vector<State*>::iterator itItem = std::find(_sampleVector.begin(), _sampleVector.end(), state);
    //item should exist
    assert(itItem!=_sampleVector.end());

    //erase from vector
    _sampleVector.erase(itItem);

    //AG140424: delete the item (otherwise memleak)
    delete state;

    //AG140409: the belief matrix is not correct anymore
    _beliefProbMatrixUptodate = false;
}

/*
//TODO: change for map usage .. but not used now

Belief* Belief::copy() {
    Belief *belief = new Belief();
    //copy belief
    //for(vector<State*>::iterator it = _sampleVector.begin(); it!=_sampleVector.end();it++) {
    for(size_t i = 0; i < _sampleVector.size(); i++) {

        belief->add(_sampleVector[i]->copy(), _scoreVector[i]);
    }

    return belief;
}

void Belief::copyFrom(Belief *belief) {
    //for(vector<State*>::iterator it = belief->_sampleVector.begin(); it!=belief->_sampleVector.end();it++) {
    for(size_t i = 0; i < belief->size(); i++) {

        add(belief->_sampleVector[i]->copy(), belief->_scoreVector[i]);
    }
}
*/

bool Belief::isEmpty() {
    return (_sampleVector.empty());
}

State* Belief::getRandomSample() {
    size_t i = 0;

    if (_sampleVector.empty()) {
        return NULL;
    } else if (_sampleVector.size()==1) {
        i = 0;
    } else {
        i = random(_sampleVector.size()-1);
    }

    return operator [](i);
}


string Belief::toString(int maxItems) {
    stringstream ss;
    ss << "[Bel,#="<< _sampleVector.size();
    if (_sampleVector.size()>0 && (maxItems>0 || maxItems==-1)) {
        ss << ":";
        int c = 0;

        //for(map<string, pair<State*,double> >::iterator it = _sampleMap.begin(); it!=_sampleMap.end(); it++) {
        for(vector<State*>::iterator it = _sampleVector.begin(); it!=_sampleVector.end(); it++) {
            ss << (*it)->toString() << " ";  //it->second.first->toString() << "x" << it->second.second<<" ";
            c++;
            if (maxItems>0 && c>=maxItems) {
                ss << "...";
                break;
            }
        }

    }
    ss<<"]";
    return ss.str();
}


void Belief::updateMapHist(int rowCount, int colCount) {
    assert(rowCount>0 && colCount>0);

    /*if ((rowCount!=_beliefProbMatrixRows || colCount!=_beliefProbMatrixCols) && _beliefProbMatrix!=NULL) {
        FreeDynamicArray<double>(_beliefProbMatrix, _beliefProbMatrixRows);
    }*/

    //init matrix
    if (_beliefProbMatrix==NULL) {
        _beliefProbMatrixRows = (unsigned int)rowCount;
        _beliefProbMatrixCols = (unsigned int)colCount;
        _beliefProbMatrix = AllocateDynamicArray<double>(_beliefProbMatrixRows,_beliefProbMatrixCols);
    }

    //first fill all with 0
    FOR(r,rowCount) {
        FOR(c,colCount) {
            _beliefProbMatrix[r][c] = 0;
        }
    }

    //now fill matrix, summing the values in the weights for each row,col
    for (const auto& it : _sampleVector) {
        HSState* hsstate = static_cast<HSState*>(it);
        int r = hsstate->hiderPos.row();
        int c = hsstate->hiderPos.col();
        _beliefProbMatrix[r][c] += 1; //it.second.second;
    }

    if (_sampleVector.size()==0) {
        cout << "Belief::updateMapHist: WARNING there are NO samples in the belief"<<endl;
    }

    //normalize the values to get the probabilities
    double total = _sampleVector.size();
    FOR(r,rowCount) {
        FOR(c,colCount) {
            _beliefProbMatrix[r][c] /= total;
        }
    }

    //to know if the values of the matrix are consistent
    _beliefProbMatrixUptodate = true;
}

double Belief::getBeliefAvgAtPos(int row, int col, int rowCount, int colCount) {
    //assert(row>=0 && row<rowCount && col>=0 && col<colCount);

    if (!_beliefProbMatrixUptodate) updateMapHist(rowCount, colCount);

    //std::cout<<"row="<<row<<" col="<<col<<" rowco="<<rowCount<<" colCount="<<colCount<<std::endl;
    return _beliefProbMatrix[row][col];
}


