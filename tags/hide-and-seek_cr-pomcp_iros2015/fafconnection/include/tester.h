#ifndef Tester_H
#define Tester_H

#include <QObject>
#include "fafconnection.h"


class Tester : public QObject, public FAFConnectionHandler {
    Q_OBJECT

public:
    Tester(FAFConnection* fafConn);

//protected slots:
    virtual void handleObservationsReceived(std::vector<double> otherSeekerPos, std::vector<double> otherHiderPos);
    virtual void handleGoalsReceived(std::vector<double> myGoalPos, double myGoalPosBelief, std::vector<double> otherGoalPos, double otherGoalPosBelief);
    virtual void gotConnected();
    virtual void gotDisconnected();

protected:
    FAFConnection* _fafConn;

};

#endif
