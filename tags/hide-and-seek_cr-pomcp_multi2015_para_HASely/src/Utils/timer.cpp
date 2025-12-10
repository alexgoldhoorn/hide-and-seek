
#include "Utils/timer.h"

#include <cmath>
//#include <iostream>

using namespace std;

Timer::Timer() {
    _lastID = 1;
}


int Timer::startTimer() {
    timeval nowTime;
    gettimeofday(&nowTime, NULL);
    int id = _lastID;
    _timerMap.insert(pair<int,struct timeval>(id, nowTime));
    _lastID++;
    return id;
}

int Timer::restartTimer(int i) {
    timeval nowTime;
    gettimeofday(&nowTime, NULL);
    _timerMap[i] = nowTime;
    return i;
}

long Timer::stopTimer(int i) {
    map<int,struct timeval>::iterator it = _timerMap.find(i);

    if (it == _timerMap.end()) {
        return -1;
    } else {
        long res = (long) round(getTimeFromMap_us(it->second)/1000000.0);
        _timerMap.erase(it);
        return res;
    }
}

long Timer::getTime(int i) {
    map<int,struct timeval>::iterator it = _timerMap.find(i);
    if (it == _timerMap.end()) {
        return -1;
    } else {
        return (long) round(getTimeFromMap_us(it->second)/1000000.0);
    }
}

long Timer::getTime_ms(int i) {
    map<int,struct timeval>::iterator it = _timerMap.find(i);
    if (it == _timerMap.end()) {
        return -1;
    } else {
        return (long) round(getTimeFromMap_us(it->second)/1000.0);
    }
}

long Timer::getTime_us(int i) { //TODO: check how many microsec long can store
    map<int,struct timeval>::iterator it = _timerMap.find(i);
    if (it == _timerMap.end()) {
        return -1;
    } else {
        return getTimeFromMap_us(it->second);
    }
}

inline unsigned long Timer::getTimeFromMap_us(struct timeval& ltime) {
    timeval nowTime;
    gettimeofday(&nowTime, NULL);
    return 1000000ul * (nowTime.tv_sec - ltime.tv_sec)  //sec
            +  (nowTime.tv_usec - ltime.tv_usec);
}



