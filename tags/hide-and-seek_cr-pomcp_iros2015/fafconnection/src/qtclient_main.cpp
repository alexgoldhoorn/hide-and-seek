#include "qtclient.h"

#include <QCoreApplication>
#include <unistd.h>

#include "tester.h"

#include <iostream>
using namespace std;



int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    cout <<endl
        << "+-----------------------------+" << endl
        << "| ROS Seeker Hide&Seek CLIENT |" << endl
        << "+-----------------------------+" << endl<<endl;

    try {
        if (argc<2) {
            cout << "Requires parameters: "<<endl<<"  server-host server-port"<<endl;
        } else {
            //start server
            QString portStr = QString::fromLatin1(argv[2]);
            bool ok = false;
            unsigned int port = portStr.toUInt(&ok);
            if (!ok) {
                cout <<"Expected port as second parameter"<<endl;
                return -1;
            }

            QtClient client(string(argv[1]), port);

            Tester t(&client);

            client.connectToServer();

            vector<double> o1(2),o2(2);
            o1[0] = 1;
            o1[1] = 2;
            o2[0] = 3;
            o2[1] = 4;

            cout << "waiting to send..."<<flush;

            while (!client.isConnected()) {
                sleep(1);
                cout<<"."<<flush;
            }
            cout << "sending..."<<endl;
            client.sendObservations(o1,o2);

            return a.exec();
        }
    }
    catch(exception &e) {
        cout << "Exception: " << e.what() << endl ;
    }


    return -1;//a.exec();
}
