#include "belief.h"

#include <cassert>
#include <sstream>

#include "Utils/generic.h"

using namespace pomcp;
using namespace std;

Belief::Belief() : _scoreSum(0) {
}

Belief::~Belief() {
    for(vector<State*>::iterator it = _sampleVector.begin(); it!=_sampleVector.end();it++) {
        delete *it;
    }
}

size_t Belief::size() {
    return _sampleVector.size();
}

State* Belief::operator[](int i){
    assert(i>=0 && i<_sampleVector.size());
    return _sampleVector[i];
}

void Belief::add(State* state, double score) {
    _sampleVector.push_back(state);
    _scoreVector.push_back(score);
    _scoreSum += score;
}

Belief* Belief::copy() {
    Belief *belief = new Belief();
    //copy belief
    //for(vector<State*>::iterator it = _sampleVector.begin(); it!=_sampleVector.end();it++) {
    for(size_t i = 0; i < _sampleVector.size(); i++) {
        /*belief->_sampleVector.push_back(_sampleVector[i]->copy());
        belief->_scoreVector.push_back(_scoreVector[i]);*/
        belief->add(_sampleVector[i]->copy(), _scoreVector[i]);
    }

    return belief;
}

void Belief::copyFrom(Belief *belief) {
    //for(vector<State*>::iterator it = belief->_sampleVector.begin(); it!=belief->_sampleVector.end();it++) {
    for(size_t i = 0; i < belief->size(); i++) {
        /*_sampleVector.push_back(belief->_sampleVector[i]->copy());
        _scoreVector.push_back(belief->_scoreVector[i]);*/
        add(belief->_sampleVector[i]->copy(), belief->_scoreVector[i]);
    }
}

bool Belief::isEmpty() {
    return (_sampleVector.size()==0);
}

State* Belief::getRandomSample(bool useScore) {
    if (_sampleVector.empty()) {
        return NULL;
    } else {
        size_t i = 0;

        if (_sampleVector.size()==1) {
            i = 0;
        } else if (useScore) {
            i = weightedRandomIndex();
        } else {
            i = random(_sampleVector.size());
        }

        return _sampleVector[i];
    }
}

string Belief::toString(int maxItems) {
    stringstream ss;
    ss << "[Bel,#="<<_sampleVector.size();
    if (_sampleVector.size()>0 && (maxItems>0 || maxItems==-1)) {
        ss << ":";
        int c = 0;
        FOREACH(State*,s,_sampleVector) {
            ss << (*s)->toString();
            c++;
            if (maxItems>0 && c>=maxItems) {
                break;
            }
        }
        if (maxItems>=0 && maxItems<_sampleVector.size())
            ss << "...";
    }
    ss<<"]";
    return ss.str();
}

size_t Belief::weightedRandomIndex() {
    double r = randomDouble(0,_scoreSum);
    double sum = 0;
    int index = 0;
    //search for index
    for(; index<_scoreVector.size(); index++) {
        //add to sum
        sum += _scoreVector[index];
        //if sum passed random value, then we have the item
        if (sum>r) {
            break;
        }
    }

    return index;
}
