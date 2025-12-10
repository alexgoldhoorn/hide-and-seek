#include "POMCP/history.h"

#include <cassert>

using namespace pomcp;
using namespace std;

History::~History(){
    for(size_t i = 0; i<_historyVector.size(); i++) {
        delete _historyVector[i].observation;
    }
}

size_t History::getLength() {
    return _historyVector.size();
}

size_t History::size() {
    return _historyVector.size();
}

History::HistoryItem& History::operator[](int t) {
    assert(t>=0 && t<(int)_historyVector.size());
    return _historyVector[t];
}

History::HistoryItem& History::getLast() {
    assert(_historyVector.size()>0);
    return _historyVector.back();
}

void History::truncate(size_t size) {    
    if (size < _historyVector.size()) {
        //delete items first
        for(size_t i = size; i<_historyVector.size(); i++) {
            delete _historyVector[i].observation;
        }

        _historyVector.resize(size);
    }
}

void History::add(int action, State* obs) {
    History::HistoryItem item(action, obs);
    _historyVector.push_back(item);
}
