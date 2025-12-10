#include "rungamethread.h"
//iriutils
#include "exceptions.h"

//appl
#ifndef DO_NOT_USE_MOMDP
#include "GlobalResource.h"
#endif

#include <QDateTime>
#include <QApplication>

#include <cassert>
#include <iostream>

using namespace std;

RunGameThread::RunGameThread(Game* game, QObject *parent) :
    QThread(parent)
{
    _game = game;
    //move the object to this thread
    game->moveToThread( this );
    //game->tcpSocket.moveToThread( this );

}


void RunGameThread::run() {
    assert(_game != NULL);

    try  {

        cout << "===--- Starting at "<< QDateTime::currentDateTime().toString().toStdString() << " ---===" << endl;

        cout << "RunGameThread: Start running .."<<endl;

        //run and connnect
        _game->startGame();
//_game->moveToThread( this );

        /*while (true) { //todo: _game.isRunning
            sleep(1);
            cout << "."<<flush;
        }*/

        cout << "RunGameThread: Stop running."<<endl;

        cout << "===--- Stopped at "<< QDateTime::currentDateTime().toString().toStdString() << " ---===" << endl;

    }
#ifndef DO_NOT_USE_MOMDP
    catch(bad_alloc &e) {
        if(GlobalResource::getInstance()->solverParams.memoryLimit == 0)
        {
            cout << "Memory allocation failed. Exit." << endl;
        }
        else
        {
            cout << "Memory limit reached. Please try increase memory limit" << endl;
        }

    }
#endif
    catch(CException &ce) {
        cout << "CException: " << ce.what() << endl ;
    }
    catch(exception &e) {
        cout << "Exception: " << e.what() << endl ;
    }
}
