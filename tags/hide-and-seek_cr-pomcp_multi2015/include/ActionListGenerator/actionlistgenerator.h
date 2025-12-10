#ifndef ACTIONLISTGENERATOR_H
#define ACTIONLISTGENERATOR_H

#include <QObject>
#include "../HSGame/gmap.h"
#include "../HSGame/gplayer.h"

/*!
 * Generates an init pos for a hider on a map (on an invisible place for the seeker, if available), and then a list of actions
 * that reach until the base is reached or a max_actions.
 *
 * \brief The ActionListGenerator class Generate an action list with random position and actions for a map (hider movements).
 */
class ActionListGenerator : public QObject
{
    Q_OBJECT
public:
    static const quint16 MAX_ACTIONS = 1000;

    ActionListGenerator(GMap* map, QObject *parent = 0);

    void generateFile(QString file, QString name);
    
signals:
    
public slots:
    
private:
    GMap* _gmap;
    Player* _player;

};

#endif // ACTIONLISTGENERATOR_H
