#ifndef FAFCONNECTION_H
#define FAFCONNECTION_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QDataStream>

#include <vector>

#include "fafconnectionhandler.h"

class FAFConnection : public QObject {
    Q_OBJECT
public:
    FAFConnection(FAFConnectionHandler* handler = NULL);
    /*FAFConnection(void (*connectedFP)(void),
        void (*disconnectedFP)(void),
        void (*observationsReceivedFP)(std::vector<double>,std::vector<double>),
        void (*goalsReceivedFP)(std::vector<double>,double,std::vector<double>,double)
    );*/

    void setFP(void (*connectedFP)(void),
                       void (*disconnectedFP)(void),
                       void (*observationsReceivedFP)(std::vector<double>,std::vector<double>),
                       void (*goalsReceivedFP)(std::vector<double>,double,std::vector<double>,double)
                   );

    virtual ~FAFConnection();

    virtual void sendObservations(std::vector<double> mySeekerPos, std::vector<double> myHiderPos);
    virtual void sendGoals(std::vector<double> myGoalPos, double myGoalPosBelief, std::vector<double> otherGoalPos, double otherGoalPosBelief);

    virtual bool isConnected() const;

    virtual void closeConnection();

    void setConnectionHandler(FAFConnectionHandler *handler);

signals:
    void connected();
    void disconnected();
    void observationsReceived(std::vector<double> otherSeekerPos, std::vector<double> otherHiderPos);
    void goalsReceived(std::vector<double> myGoalPos, double myGoalPosBelief, std::vector<double> otherGoalPos, double otherGoalPosBelief);

protected:
    void sendVector(QDataStream& out, const std::vector<double>& vec);

    void readyReadGeneric();

    void readObservations(QDataStream& in);

    void readGoals(QDataStream& in);

    std::vector<double> readVector(QDataStream& in);


    //! connection to server
    QTcpSocket *_socket;


    //function pointers
    void (*_connectedFP)(void);
    void (*_disconnectedFP)(void);
    void (*_observationsReceivedFP)(std::vector<double>,std::vector<double>);
    void (*_goalsReceivedFP)(std::vector<double>,double,std::vector<double>,double);


    FAFConnectionHandler* _handler;
};

#endif
