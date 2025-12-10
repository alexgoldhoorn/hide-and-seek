#ifndef RANDOMTESTER_H
#define RANDOMTESTER_H

#include <QObject>
#include "../HSGame/gmap.h"
#include "../HSGame/gplayer.h"

/*!
 * Generates an init pos for a hider on a map (on an invisible place for the seeker, if available), and then a list of actions
 * that reach until the base is reached or a max_actions.
 *
 * \brief The ActionListGenerator class Generate an action list with random position and actions for a map (hider movements).
 */
class RandomTester : public QObject
{
    Q_OBJECT
public:
    static const quint16 MAX_ACTIONS = 1000;

    RandomTester(GMap* map, QObject *parent = 0);

    //! Test map n times, using random movements, how fast it reaches the base, returns an average
    double testMap(int n, double &std);
    
    //! returns it for 1 time
    int testMapOnce();
signals:
    
public slots:
    
private:
    GMap* _gmap;
    Player* _player;

};

#endif // ACTIONLISTGENERATOR_H
