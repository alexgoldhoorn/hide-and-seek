#ifndef GUITHREAD_H
#define GUITHREAD_H

#include <QThread>
#include <QMainWindow>
#include <QApplication>


class GUIThread : public QThread
{
    Q_OBJECT
public:
    explicit GUIThread(QMainWindow* window, QApplication* a, QObject *parent = 0);
    
signals:
    
public slots:

private:
    void run();

    QMainWindow* _window;
    QApplication* _a;
    
};

#endif // GUITHREAD_H
