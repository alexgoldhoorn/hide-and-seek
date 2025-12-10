#ifndef GAMENOSERVER_H
#define GAMENOSERVER_H

#include "game.h"
#include "seekerhs.h"

class GameNoServer : public Game
{
    Q_OBJECT

protected:
    virtual void moveToThreadVariables(QThread* thread);

public:
    GameNoServer(SeekerHS* seekerHS);

    virtual bool startGame();


private:
    SeekerHS* _seekerHS;
};

#endif // GAMENOSERVER_H
