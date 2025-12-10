#include "qtserver.h"

#include "tester.h"

#include <QCoreApplication>

#include <iostream>
using namespace std;


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    cout <<endl
        << "+-----------------------------+" << endl
        << "| ROS Seeker Hide&Seek SERVER |" << endl
        << "+-----------------------------+" << endl<<endl;

    try {
        if (argc<1) {
            cout << "Requires parameters: "<<endl<<"  server-port"<<endl;
        } else {
            //start server
            QString portStr = QString::fromLatin1(argv[1]);
            bool ok = false;
            unsigned int port = portStr.toUInt(&ok);
            if (!ok) {
                cout <<"Expected port as parameter"<<endl;
                return -1;
            }

            QtServer server(port);

            Tester t(&server);

            cout << "waiting..."<<endl;


            return a.exec();

        }

    }
    catch(exception &e) {
        cout << "Exception: " << e.what() << endl ;
    }

    return -1;//a.exec();
}
