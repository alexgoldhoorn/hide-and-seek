
#include "hsserver.h"
#include "hstcpserver.h" //ag
#include <QtNetwork>

#include "hsgamelogdb.h"

/*HSServer::HSServer(int port) {
    init(port);
}
HSServer::HSServer(QString ip, int port) {
    init(ip,port);
}*/

/*HSServer::HSServer(int port)
{
    _hstcpserver=new HSTCPServer;
    _hstcpserver->setServer(this);


    if (!_hstcpserver->listen(QHostAddress::Any,port)) {
        cout<<"Threaded Fortune Server. Unable to start the server: "<<endl;
        //close();
        exit(-1);//ag111201
        return;
    }
    QString ipAddress;
    //QString port;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address())
            ipAddress = ipAddressesList.at(i).toString();
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    cout<<"The server is running on IP: "<<ipAddress.toStdString()<<"    Port: "<<(int)_hstcpserver->serverPort()<<endl;

    port =(_hstcpserver->serverPort());  /// port.number
    _hstcpserver->setip(ipAddress);
    _hstcpserver->setport(port);
    cout<<port<<endl;
}

HSServer::HSServer(QString ip, int port)
{
    _hstcpserver=new HSTCPServer;
    _hstcpserver->setServer(this);
    _hstcpserver->setip(ip);
    _hstcpserver->setport(port);
    QHostAddress adr;
    adr.setAddress( ip );
    adr.toIPv4Address();
    //quint16 por = p.toUInt();


    if (!_hstcpserver->listen(QHostAddress::Any,port)) {
        cout<<" Server. Unable to start the server, exiting... "<<endl;
        //close();
        exit(-1);
        return;
    }

    cout<<"The server is running on IP: "<<ip.toStdString()<<"    Port: "<<(int)_hstcpserver->serverPort()<<endl;


}*/


HSServer::HSServer(QString xmlConfigFile) {
    //parse config
    _config = new HSServerConfig();
    _config->parsConfigXML(xmlConfigFile);

    _gameLog = NULL;

    QString ip = _config->getIP();
    cout << "IP: "<<ip.toStdString()<<endl;

    if (ip.isEmpty()) ip = _config->getDefaultIP();
    quint16 port = _config->getPort();
    if (port==0) port = HSServerConfig::DEFAULT_PORT;

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
    } else {
        cout << "Log path:      " << _config->getLogPath().toStdString()<<endl;

        //TODO: log to file
        //_gameLog
    }
    cout << "Action path:   "<<_config->getRecordedActionsPath().toStdString()<<endl;


    //create server
    _hstcpserver=new HSTCPServer(_config, _gameLog);
    _hstcpserver->setServer(this);
    _hstcpserver->setip(ip);
    _hstcpserver->setport(port);
    QHostAddress adr;
    adr.setAddress( ip );
    adr.toIPv4Address();

    if (!_hstcpserver->listen(QHostAddress::Any,port)) {
        cout<<" Server. Unable to start the server, exiting... "<<endl;
        //close();
        if (_gameLog!=NULL) {
            _gameLog->close();
        }

        exit(-1);
        return;
    }

    cout<<"The server is running on IP: "<<ip.toStdString()<<"    Port: "<<(int)_hstcpserver->serverPort()<<endl;


}

/*
void HSServer::init(int port)
{
    _hstcpserver=new HSTCPServer;
    _hstcpserver->setServer(this);


    if (!_hstcpserver->listen(QHostAddress::Any,port)) {
        cout<<"Threaded Fortune Server. Unable to start the server: "<<endl;
        //close();
        exit(-1);//ag111201
        return;
    }
    QString ipAddress;
    //QString port;


    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address())
            ipAddress = ipAddressesList.at(i).toString();
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();




    cout<<"The server is running on IP: "<<ipAddress.toStdString()<<"    Port: "<<(int)_hstcpserver->serverPort()<<endl;

    port = //port.number_hstcpserver->serverPort());
    _hstcpserver->setip(ipAddress);
    _hstcpserver->setport(port);
    cout<<port<<endl;
}


void HSServer::init(QString ip, int port)
{
    _hstcpserver=new HSTCPServer;
    _hstcpserver->setServer(this);
    _hstcpserver->setip(ip);
    _hstcpserver->setport(port);
    QHostAddress adr;
    adr.setAddress( ip );
    adr.toIPv4Address();
    //quint16 por = p.toUInt();


    if (!_hstcpserver->listen(QHostAddress::Any,port)) {
        cout<<" Server. Unable to start the server, exiting... "<<endl;
        //close();
        exit(-1);
        return;
    }

    cout<<"The server is running on IP: "<<ip.toStdString()<<"    Port: "<<(int)_hstcpserver->serverPort()<<endl;


}

*/
