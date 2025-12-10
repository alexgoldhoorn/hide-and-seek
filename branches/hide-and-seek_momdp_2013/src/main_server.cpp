
#include <QCoreApplication>
#include <iostream>

///TODO: add the max steps field (or depend on map..)

#include "Server/hsserver.h"


using namespace std;


int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);

#ifdef TARGET_OS_MAC
    cout << "Mac" << endl;
#elif defined __linux__
    cout << "Linux" << endl;
#elif defined _WIN32 || defined _WIN64
    cout << "Windows" << endl;
#else
//#error "unknown platform"
#endif

    cout <<endl
        << "+-----------------------------------+" << endl
        << "| Automatic Seeker Hide&Seek SERVER |" << endl
        << "+-----------------------------------+" << endl<<endl;


    try
    {

        if (argc<2) {
            cout << "Error: expected 1 parameter: xml file"<<endl;
            exit(-1);
        }

        QString xmlFile(argv[1]);
        HSServer server(xmlFile);

    }
    catch(exception &e)
    {
        cout << "Exception: " << e.what() << endl ;
    }


    return a.exec();
}
