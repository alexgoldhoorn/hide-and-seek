#ifndef GAME_H
#define GAME_H

#include <QObject>

/*!
 * \brief The Game class interface for Game classes, they should handle the game either with or without server
 */
class Game : public QObject {
    Q_OBJECT

protected:
    virtual void moveToThreadVariables(QThread* thread)=0;

public:
    virtual bool startGame()=0;

    void moveToThread(QThread* thread) {
        QObject::moveToThread(thread);
        moveToThreadVariables(thread);
    }
};

#endif // GAME_H
