#ifndef RUNGAMETHREAD_H
#define RUNGAMETHREAD_H

#include <QThread>
//#include "gameconnector.h"
#include "Base/game.h"


class RunGameThread : public QThread
{
    Q_OBJECT
public:
    explicit RunGameThread(Game* game, QObject *parent = 0);
    
signals:
    
public slots:

protected:
    void run();

private:
    Game* _game;
    
};

#endif // RUNGAMETHREAD_H
