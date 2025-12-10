#ifndef HSSERVERCONFIG_H
#define HSSERVERCONFIG_H

#include <QString>

class HSServerConfig
{
    //Q_OBJECT

public:
    static const quint16 DEFAULT_PORT = 1120;

    HSServerConfig(QString xmlfile);
    HSServerConfig();
    HSServerConfig(QString ip, quint16 port = DEFAULT_PORT);

    //AG120514: functions to set map log path
    void setMapPath(QString mapPath);
    void setLogPath(QString logPath);

    void parsConfigXML(QString xmlfile);

    //get config
    QString getIP();
    quint16 getPort();
    QString getLogPath();
    QString getMapPath();
    //ag120903: path of recorded actions
    QString getRecordedActionsPath();
    QString getServerProg();
    QString getAutoHiderProg();

    //! Use DB log, otherwise file.
    bool useDBLog();

    void getLogDB(QString& dbServer, QString& dbDB, QString& dbUser, QString& dbPass);

    //! Gets the default IP (first non local, otherwise local)
    QString getDefaultIP();

    QString getXMLFile();



private:

    //ag120514: path of map
    QString _mapPath;
    //ag120514: path of log
    QString _logPath;
    //ag120903: path of recorded actions
    QString _recActPath;

    QString _xmlFile;

    QString _ip;
    quint16 _port;

    QString _serverProg;
    QString _autoHiderProg;

    bool _useDBLog;
    QString _dbServer;
    QString _dbDB;
    QString _dbUser;
    QString _dbPass;
};

#endif // HSSERVERCONFIG_H
