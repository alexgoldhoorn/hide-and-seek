#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <map>


using namespace std;

//! Class that contains a list of timers.
class Timer {

public:
    Timer();

    /*! Start a new timer, returns the ID.
      ID is -1 if failed.
      */
    int startTimer();

    /*! Restart timer i, -1 if failed.
      */
    int restartTimer(int i);

    /*! Stop timer and returns time.
        -1 if failed.
      */
    long stopTimer(int i);

    /*! Get time. -1 if failed.*/
    long getTime(int i);

private:
    map<int,time_t> _timerMap;
    int _lastID;
};


#endif // TIMER_H
