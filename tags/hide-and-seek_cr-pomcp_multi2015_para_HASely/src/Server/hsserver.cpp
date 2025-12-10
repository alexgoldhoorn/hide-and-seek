#include "Server/hsserver.h"

#include <iostream>

#include <QtNetwork>

#include "Server/hstcpserver.h"

#include "Server/hsgamelogdb.h"
#include "Base/hsglobaldata.h"

using namespace std;

HSServer::HSServer(QString xmlConfigFile) {
    //parse config
    _config = new HSServerConfig(xmlConfigFile);

    _gameLog = NULL;

    QString ip = _config->getIP();
    cout << "IP: "<<ip.toStdString()<<endl;

    if (ip.isEmpty()) ip = _config->getDefaultIP();
    quint16 port = _config->getPort();
    if (port==0) port = HSGlobalData::DEFAULT_SERVER_PORT;

    cout << "Starting server ... "<<endl;
    cout << "Config:        "<<_config->getXMLFile().toStdString()<<endl;
    cout << "IP:            "<<ip.toStdString()<<" port:"<<port<<endl;
    cout << "Map path:      " << _config->getMapPath().toStdString()<<endl;
    cout << "Server:        " << _config->getServerProg().toStdString()<<endl;
    cout << "Random hider:  "<<_config->getAutoHiderProg().toStdString()<<endl;
    if (_config->useDBLog()) {
        QString dbServer,dbDB,dbUser,dbP;
        _config->getLogDB(dbServer,dbDB,dbUser,dbP);
        cout << "Log DB:        " << dbDB.toStdString() << "@" << dbServer.toStdString() << endl;
        _gameLog = new HSGameLogDB(_config);
        int id = _gameLog->getID();
        cout << "Session ID:    ";
        if (id<0) {
            cout << "NO session opened"<<endl;
        } else {
            cout << id<<endl;
        }

    } else {
        cout << "Log path:      " << _config->getLogPath().toStdString()<<endl;

        //TODO: log to file
        //_gameLog
    }
    cout << "Action path:   "<<_config->getRecordedActionsPath().toStdString()<<endl;
    cout << "Max. actions:  ";
    switch(_config->getMaxActCalcType()) {
    case HSServerConfig::MAX_ACT_CALC_TYPE1:
        cout << "type 1 ( 2*[rows+cols] )"<<endl;
        break;
    case HSServerConfig::MAX_ACT_CALC_TYPE2:
        cout << "type 2 ( rows*cols )"<<endl;
        break;
    case HSServerConfig::MAX_ACT_CALC_FIXED:
        cout << "fixed ( "<<_config->getMaxActionsFixed()<<" )"<<endl;
        break;
    default:
        cout << "unknown type"<<endl;
        break;
    }
    cout << "Seeker wins if crossing: "<<(_config->getWinIfCrossed()?"yes":"no")<<endl;
    cout << endl;

    //create server
    _hstcpserver=new HSTCPServer(_config, _gameLog);
    _hstcpserver->setServer(this);
    _hstcpserver->setip(ip);
    _hstcpserver->setport(port);
    QHostAddress adr;
    adr.setAddress( ip );
    adr.toIPv4Address();

    if (!_hstcpserver->listen(QHostAddress::Any,port)) {
        cout COUT_SHOW_ID<<"Server. Unable to start the server, exiting... "<<endl;
        //close();
        /*if (_gameLog!=NULL) {
            _gameLog->close();
        }        

        exit(-1);*/
        _hstcpserver->closeServer(-1,false);
        exit(-1); //to be sure
    }

    cout COUT_SHOW_ID<<"The server is running on IP: "<<ip.toStdString()<<"    Port: "<<(int)_hstcpserver->serverPort()<<endl;
}

