#include "guiloader.h"
#include <iostream>

using namespace std;

GuiLoader::GuiLoader(QMainWindow* window, QObject *parent) :
    QObject(parent)
{
    _window = window;
}


bool GuiLoader::event(QEvent *ev)  {
    if( ev->type() == QEvent::User) {
        //w = new QWidget;
        cout << "SHOW WINDOW:"<<flush;
        _window->show();
        cout << "OK"<<endl;
        return true;
    }
    return false;
}
