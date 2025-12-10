#ifndef FAFCONNECTIONHANDLER
#define FAFCONNECTIONHANDLER

#include <vector>

class FAFConnectionHandler {
public:
    virtual void handleObservationsReceived(std::vector<double> otherSeekerPos, std::vector<double> otherHiderPos)=0;
    virtual void handleGoalsReceived(std::vector<double> myGoalPos, double myGoalPosBelief, std::vector<double> otherGoalPos, double otherGoalPosBelief)=0;
    virtual void gotConnected()=0;
    virtual void gotDisconnected()=0;
};

#endif // FAFCONNECTIONHANDLER

