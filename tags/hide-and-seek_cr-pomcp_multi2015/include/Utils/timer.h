#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <map>

#include <sys/time.h>





/*!
 * \brief The Timer class Keeps track of multiple timers. Each timer is assigned an id.
 */
class Timer {

public:
    Timer();

    /*!
     * \brief startTimer Start a new timer, returns the ID. ID is -1 if failed. The time is calculated by simple
     * subtraction of the current time - start time.
     * \return
     */
    int startTimer();

    /*!
     * \brief restartTimer Restart timer i, -1 if failed.
     * \param id
     * \return the time in s
     */
    int restartTimer(int id);

    /*!
     * \brief stopTimer Stop timer and returns time. -1 if failed.
     * \param id
     * \return the time in s
     */
    long stopTimer(int id);

    /*!
     * \brief getTime Get time. -1 if failed.
     * \param id
     * \return the time in s
     */
    long getTime(int id);

    /*!
     * \brief getTime_ms Get time. -1 if failed.
     * \param id
     * \return the time in ms
     */
    long getTime_ms(int id);

    /*!
     * \brief getTime_us Get time. -1 if failed.
     * \param id
     * \return the time in us (microseconds)
     */
    long getTime_us(int id);

private:    
    unsigned long getTimeFromMap_us(struct timeval& ltime);

    //! list of timers
    std::map<int,struct timeval> _timerMap;

    //! last timer id
    int _lastID;
};


#endif // TIMER_H
