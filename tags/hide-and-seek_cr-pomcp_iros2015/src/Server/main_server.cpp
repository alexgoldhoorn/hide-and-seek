
#include <QCoreApplication>
#include <QTcpSocket>
#include <iostream>


#include "Server/hsserver.h"
#include "Server/hsgamelogdb.h"
#include "Server/hsserverconfig.h"
#include "Server/hstcpserver.h"
#include "Server/hsserverclientcommunication.h"

//iriutils
#include "exceptions.h"

#include "hsglobaldata.h"


//opencv
#include <opencv/cv.h>

using namespace std;


void showVersion() {
    cout << "Hide&Seek Server v"<<HS_VERSION<<endl<<endl;
    cout << "Build: "<<__DATE__<<" "<<__TIME__<<endl<<endl;
    cout << "Compiled with: "<<endl;
    cout << " - Qt " << QT_VERSION_STR <<endl;
    cout << " - OpenCV " << CV_MAJOR_VERSION << "." << CV_MINOR_VERSION<<endl<<endl;
    cout << "GNU compiler version: " <<__VERSION__ << " (" << _MACHINEBITS << " bits)" <<endl;//<<__GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__<<endl;
    if( __cplusplus == 201103L ) std::cout << "C++11\n" ;
    else if( __cplusplus == 199711L ) std::cout << "C++98\n" ;
    else std::cout << "pre-standard C++\n" ;
    cout << " (C++ version: "<<__cplusplus<<")"<<endl;
#if __cplusplus>=201103
    cout <<"New C++, 2011"<<endl;
#else
    cout <<"Old C++"<<endl;
#endif
    cout << endl << "Debug flags: ";
    DEBUG_HS(cout <<"debug (DEBUG_HS); ";);
    DEBUG_HS_INIT(cout <<"init debug (DEBUG_HS_INIT); ";);
    DEBUG_HS1(cout << "detailed debug (DEBUG_HS1); ";);
    DEBUG_MAP(cout << "map debugging (DEBUG_MAP); ";);
    DEBUG_SEGMENT(cout << "segment debug (DEBUG_SEGMENT); ";);
    DEBUG_AUTOHIDER(cout << "auto hider debug (DEBUG_AUTOHIDER); ";);
#ifdef AUTOHIDER_OLD
    cout << "Using AUTOHIDER_OLD, i.e. external auto hider"<<endl;
#endif
    cout << endl;
}

void sendStopMessage(QString ip, int port) {
    QTcpSocket tcpSocket;
    cout << "Connecting to "<<ip.toStdString()<<":"<<port<<" ... "<<flush;
    tcpSocket.connectToHost(ip,port);
    cout << "ok"<<endl;

    if (tcpSocket.waitForConnected(3000)) {
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out.setVersion(HSGlobalData::DATASTREAM_VERSION);

        out << (BLOCK_SIZE)0; //block size
        out << (MESSAGE_TYPE_SIZE)MT_StopServer;

        out.device()->seek(0);
        out << (BLOCK_SIZE)(byteArray.size() - sizeof(BLOCK_SIZE));

        //write
        cout << "Sending stop signal: "<<flush;
        tcpSocket.write(byteArray);
        tcpSocket.flush();
        cout << "ok"<<endl;
    } else {
        cout << "Could not connect to the server."<<endl;
    }
    //close

    if (tcpSocket.isOpen()) tcpSocket.close();
}

void showHelp() {
    cout << "Error: expected 1 parameter, XML configuration file"<<endl
         << "or -v                  for version details."<<endl
         << "or -stop port [ip]     to stop the server on the given port, ip default is 'localhost'" <<endl;
    exit(-1);
}

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

    //ag140521: reset locale after Qt sets to system's local (http://qt-project.org/doc/qt-5/QCoreApplication.html#locale-settings)
    setlocale(LC_NUMERIC,"C");
    //ag120208: set locale to en-US such that the atof uses decimal dots and not comma
    setlocale(LC_NUMERIC,"en_US");

    cout <<endl
        << "+-----------------------------------+" << endl
        << "| Automatic Seeker Hide&Seek SERVER |" << endl
        << "+-----------------------------------+" << endl<<endl;

    try {

        if (argc<2) {
            showHelp();
        }        

        QString arg1(argv[1]);

        if (argv[1][0]=='-') {
            if (argv[1][1]=='v') {
                //version
                showVersion();
                return 0;
            } else if (arg1.compare("-stop")==0) {
                if (argc<3) {
                    cout << "Required a second parameter: the port number of the server to stop"<<endl;
                    exit(-1);
                }
                //get port
                QString portStr = QString::fromLatin1(argv[2]);
                bool ok = false;
                int port = portStr.toInt(&ok);
                if (!ok) {
                    cout << "The second parameter should be a port number."<<endl;
                    exit(-1);
                }
                QString ip = "localhost";
                if (argc>3) {
                    ip = QString::fromLatin1(argv[3]);
                }

                sendStopMessage(ip,port);

                return 0;
            } else {
                showHelp();
            }
        }

        HSServer server(arg1);
	//AG140615: added before closing of {} to make sure that it is run BEFORE the HSServer class is destroyed
    	return a.exec();
    }
    catch(CException &ce) {
        cout << "CException: " << ce.what() << endl ;
    }
    catch(exception &e) {
        cout << "Exception: " << e.what() << endl ;
    }

    return -1;

    return -1;//a.exec();
}
