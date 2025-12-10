
#include "timer.h"

Timer::Timer() {
    _lastID = 0;
}


int Timer::startTimer() {
    int id = _lastID;
    _timerMap.insert(pair<int,long>(id, time(NULL)));
    _lastID++;
    return id;
}

int Timer::restartTimer(int i) {
    _timerMap[i] = time(NULL);
    return i;
}


long Timer::stopTimer(int i) {
    long t = time(NULL);

    map<int,time_t>::iterator it = _timerMap.find(i);

    if (it == _timerMap.end()) {
        return -1;
    } else {        
        long res = it->second;
        _timerMap.erase(it);
        return (t-res);
    }
}


long Timer::getTime(int i) {
    map<int,time_t>::iterator it = _timerMap.find(i);
    if (it == _timerMap.end()) {
        return -1;
    } else {
        return it->second;
    }
}

