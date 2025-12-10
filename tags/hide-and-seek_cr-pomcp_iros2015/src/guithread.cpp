#include "guithread.h"

#include <iostream>

using namespace std;

GUIThread::GUIThread(QMainWindow* window, QApplication *a, QObject *parent) :
    QThread(parent)
{
    _window = window;
    _a = a;
}

void GUIThread::run() {
    cout << "GUIThread, show.. "<<endl;
    _window->show();
    _a->exec();
    cout << "GUIThread finished"<<endl;

}
