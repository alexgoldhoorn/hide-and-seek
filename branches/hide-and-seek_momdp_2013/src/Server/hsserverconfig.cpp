#include "hsserverconfig.h"
#include "xmlconfigparser.h"

#include <QtNetwork>
#include <QDir>

#include <iostream>

using namespace std;

HSServerConfig::HSServerConfig(QString xmlfile) {
    _xmlFile = xmlfile;

}

HSServerConfig::HSServerConfig() {
    _ip = getIP();
    _port = DEFAULT_PORT;
}

HSServerConfig::HSServerConfig(QString ip, quint16 port) {
    _port = port;
    _ip = ip;
}

void HSServerConfig::parsConfigXML(QString xmlfile) {
    _xmlFile = xmlfile;
    QFile xmlFileO(xmlfile);
    if (!xmlFileO.exists()) { //AG121015
        cout << "Configuration file "<<xmlfile.toStdString()<<" does not exist."<<endl;
        exit(-1);
    }
    XMLConfigNode* root = XMLConfigParser::parseXMLConfigFile(&xmlFileO);

    _ip = root->getFirstChildValue("ip");
    _port = root->getFirstChildValue("port").toInt();

    XMLConfigNode* node = root->getFirstChild("path");
    if (node!=NULL) {
        _mapPath = node->getFirstChildValue("maps");
        _logPath = node->getFirstChildValue("logs");
        _recActPath = node->getFirstChildValue("actions");
    }

    node = root->getFirstChild("progs");
    if (node!=NULL) {
        _serverProg = node->getFirstChildValue("server");
        _autoHiderProg = node->getFirstChildValue("autohider");
    }

    node = root->getFirstChild("logdb");
    if (node!=NULL) {
        _dbServer = node->getFirstChildValue("server");
        _dbDB = node->getFirstChildValue("database");
        _dbUser = node->getFirstChildValue("user");
        _dbPass = node->getFirstChildValue("pass");
    }

    QString logMethod = root->getFirstChildValue("log-method");
    if (logMethod.isEmpty()) {
        if (_dbServer.isEmpty()) _useDBLog = false;
    } else {
        _useDBLog = (logMethod.toLower().compare("db")==0);
    }
}

/*
<Settings name="hsserver">

<ip>localhost</ip>
<port>1120</port>

<path>
    <maps>/home/agoldhoorn/MyProjects/Experiments/maps</maps>
    <logs>/home/agoldhoorn/MyProjects/Experiments/test-logs</logs>
</path>
<progs>
    <server>/home/agoldhoorn/MyProjects/HideSeekGame/hscomplete/build/release/server/hsserver</server>
    <autohider>/home/agoldhoorn/MyProjects/HideSeekGame/Marzo/Clientcopy1/HideSeekGame</autohider>
</progs>

</Settings>
  */


QString HSServerConfig::getDefaultIP() {
    QString ipAddress;
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

    return ipAddress;
}



//AG120514: functions for map log path
QString HSServerConfig::getMapPath() {
    if (_mapPath.isEmpty()) {
        QString dirStr = QDir::currentPath() + QDir::separator() + "maps";
        QDir dir(dirStr);
        if (dir.exists()==false) {
            //TODO: error handling
            cout << "ERROR: getMapPath: not found maps directory"<<endl;
        }
        _mapPath = dirStr;
    }


    return _mapPath;
}

QString HSServerConfig::getLogPath() {
    if (_logPath.isEmpty()) {
        QString dirStr = QDir::currentPath() + QDir::separator() + "logs";
        QDir dir(dirStr);
        if (dir.exists()==false) {
            //TODO: error handling
            cout << "ERROR: getMapPath: not found maps directory"<<endl;
        }
        _logPath = dirStr;
    }


    return _logPath;
}

QString HSServerConfig::getRecordedActionsPath() {
    return _recActPath;
}

void HSServerConfig::setMapPath(QString mapPath) {
    _mapPath = mapPath;
}

void HSServerConfig::setLogPath(QString logPath) {
    _logPath = logPath;
}


QString HSServerConfig::getIP() {
    return _ip;
}

quint16 HSServerConfig::getPort(){
    return _port;
}

QString HSServerConfig::getServerProg(){
    return _serverProg;
}

QString HSServerConfig::getAutoHiderProg(){
    return _autoHiderProg;
}


QString HSServerConfig::getXMLFile() {
    return _xmlFile;
}



//! Use DB log, otherwise file.
bool HSServerConfig::useDBLog() {
    return _useDBLog;
}

void HSServerConfig::getLogDB(QString& dbServer, QString& dbDB, QString& dbUser, QString& dbPass) {
    dbServer = _dbServer;
    dbDB = _dbDB;
    dbUser = _dbUser;
    dbPass = _dbPass;
}
