#ifndef HSSERVERCONFIG_H
#define HSSERVERCONFIG_H

#include <QString>

/*!
 * \brief The HSServerConfig class stores the configuration of the Hide&Seek server. It loads the
 * configuration from an XML file using the XMLConfig class.
 */
class HSServerConfig
{
    //Q_OBJECT

public:    

    static const char MAX_ACT_CALC_TYPE1 = 1;
    static const char MAX_ACT_CALC_TYPE2 = 2;
    static const char MAX_ACT_CALC_FIXED = 3;

    HSServerConfig(QString xmlfile = "");
    //HSServerConfig();
    HSServerConfig(QString ip, quint16 port);

    //AG120514: functions to set map log path
    void setMapPath(QString mapPath);
    void setLogPath(QString logPath);

    void parseConfigXML(QString xmlfile);

    //get config
    QString getIP();
    quint16 getPort();
    QString getLogPath();
    QString getMapPath();
    //ag120903: path of recorded actions
    QString getRecordedActionsPath();
    QString getServerProg();
    QString getAutoHiderProg();

    //AG131030: get max action calc type
    quint16 getMaxActionsFixed();
    char getMaxActCalcType();

    //ag131211
    bool getWinIfCrossed();

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

    //AG131030
    char _maxActCalcType;
    quint16 _maxActionsFixed;

    //! win if crossed
    bool _winIfCrossed;
};

#endif // HSSERVERCONFIG_H
