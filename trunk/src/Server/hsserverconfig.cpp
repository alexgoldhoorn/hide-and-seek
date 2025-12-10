#include "Server/hsserverconfig.h"
#include "XMLConfig/xmlconfigparser.h"

#include <QtNetwork>
#include <QDir>

#include <iostream>

#include "Utils/generic.h"
#include "Base/hsglobaldata.h"

using namespace std;

HSServerConfig::HSServerConfig(QString xmlfile) {
    if (xmlfile.isEmpty()) {
        _ip = QString::fromStdString(HSGlobalData::DEFAULT_SERVER_IP);
        _port = HSGlobalData::DEFAULT_SERVER_PORT;
        _xmlFile.clear();
    } else {
        parseConfigXML(xmlfile);
    }
}

HSServerConfig::HSServerConfig(QString ip, quint16 port) {
    _port = port;
    _ip = ip;
}

void HSServerConfig::parseConfigXML(QString xmlfile) {
    //ag131202: check since now done in constructor if already passed
    if (xmlfile == _xmlFile) {
        cout << "HSServerConfig::parseConfigXML: WARNING parsing the same config file for the second time."<<endl;
    }

    _xmlFile = xmlfile;
    QFile xmlFileO(xmlfile);
    if (!xmlFileO.exists()) { //AG121015
        cout << "Configuration file "<<xmlfile.toStdString()<<" does not exist."<<endl;
        QCoreApplication::exit(-1);
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

    //AG131030: get max actions type
    node = root->getFirstChild("max-actions");
    _maxActionsFixed = 0;
    if (node==NULL) {
        _maxActCalcType = MAX_ACT_CALC_TYPE2;
    } else {
        QString val = node->getAttribute("type");
        if (val == "type1" || val == "2r+c") {
            _maxActCalcType = MAX_ACT_CALC_TYPE1;
        } else if (val == "type2" || val == "r*c") {
            _maxActCalcType = MAX_ACT_CALC_TYPE2;
        } else if (val == "fixed") {
            _maxActCalcType = MAX_ACT_CALC_FIXED;
            bool ok = false;
            _maxActionsFixed = node->getValue().toInt(&ok);
            if (!ok) {
                cout << "Unexpected maximum actions value"<<endl;
                QCoreApplication::exit(1);
            }
        } else {
            cout << "Error: unknown max actions calculation type value (<max-actions>) in the config file: '"<<val.toStdString()<<"'."<<endl;
            QCoreApplication::exit(1);
        }
    }

    //AG131211: win if crossed
    _winIfCrossed = true;
    QString winIfCrossedStr = root->getFirstChildValue("win-if-crossed");
    if (!winIfCrossedStr.isEmpty()) {
        bool ok = false;
        _winIfCrossed = hsutils::qstringToBool(winIfCrossedStr,&ok);
        if (!ok) {
            cout << "Win if crossed (<win-if-crossed>) value is not a boolean: "<<winIfCrossedStr.toStdString()<<endl;
            QCoreApplication::exit(1);
        }
    }

    //AG160123: put noise or not (default true)
    _simNotVisibDist = true;
    node = root->getFirstChild("noise-params");
    if (node!=NULL) {
        QString simVisibDistStr = node->getFirstChildValue("sim-visib-dist");
        bool ok = false;
        _simNotVisibDist = hsutils::qstringToBool(simVisibDistStr,&ok);
        if (!ok) {
            cout << "Sim. visib. dist. (<sim-visib-dist>) value is not a boolean: "<<simVisibDistStr.toStdString()<<endl;
            QCoreApplication::exit(1);
        }
    }

}

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


quint16 HSServerConfig::getMaxActionsFixed() {
    return _maxActionsFixed;
}

char HSServerConfig::getMaxActCalcType() {
    return _maxActCalcType;
}

bool HSServerConfig::getWinIfCrossed() {
    return _winIfCrossed;
}

bool HSServerConfig::simNotVisibDist() {
    return _simNotVisibDist;
}
