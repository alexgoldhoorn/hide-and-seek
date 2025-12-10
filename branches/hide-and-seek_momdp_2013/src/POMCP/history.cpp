#include "history.h"

#include <cassert>

using namespace pomcp;
using namespace std;

size_t History::getLength() {
    return _historyVector.size();
}

size_t History::size() {
    return _historyVector.size();
}

History::HistoryItem& History::operator[](int t) {
    assert(t>=0 && t<_historyVector.size());
    return _historyVector[t];
}

History::HistoryItem& History::getLast() {
    assert(_historyVector.size()>0);
    return _historyVector.back();
}

void History::truncate(size_t size) {
    if (size < _historyVector.size()) {
        _historyVector.resize(size);
    }
}

void History::add(int action, int obs) {
    History::HistoryItem item(action, obs);
    _historyVector.push_back(item);
}
