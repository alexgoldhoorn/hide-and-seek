#include "tester.h"

#include <cassert>
#include <unistd.h>

#include <iostream>
using namespace std;


Tester::Tester(FAFConnection *fafConn) {
    _fafConn = fafConn;
    _fafConn->setConnectionHandler(this);

        //virtual void sendGoals(std::vector<double> myGoalPos, double myGoalPosBelief, std::vector<double> otherGoalPos, double otherGoalPosBelief
    /*connect(_fafConn, SIGNAL(observationsReceived(std::vector<double>,std::vector<double>)), this, SLOT(handleObservationsReceived(std::vector<double>,std::vector<double>)));
    connect(_fafConn, SIGNAL(goalsReceived(std::vector<double>,double,std::vector<double>,double)), this, SLOT(handleGoalsReceived(std::vector<double>,double,std::vector<double>,double)));
    connect(_fafConn, SIGNAL(connected()), this, SLOT(gotConnected()));
    connect(_fafConn, SIGNAL(disconnected()), this, SLOT(gotDisconnected()));*/

    //cout<<"------__FP"<<endl;



    //_fafConn->setFP(&gotConnected, &gotDisconnected, &handleObservationsReceived, &handleGoalsReceived);
}

void Tester::handleObservationsReceived(std::vector<double> otherSeekerPos, std::vector<double> otherHiderPos) {
    cout <<"Tester::observationsReceived:"<<endl;
    cout <<" - other seeker pos: ";
    for(double d : otherSeekerPos) cout << d<<" ";
    cout <<endl<<" - other hiderp pos: ";
    for(double d : otherHiderPos) cout << d<<" ";
    cout<<endl;

    for(double &d : otherSeekerPos) d+=0.5;
    for(double &d : otherHiderPos) d+=0.5;


    _fafConn->sendGoals(otherSeekerPos, 0.666, otherHiderPos, -1.5);

}

void Tester::handleGoalsReceived(std::vector<double> myGoalPos, double myGoalPosBelief, std::vector<double> otherGoalPos, double otherGoalPosBelief) {
    cout <<"Tester::goalsReceived:"<<endl;
    cout <<" - my goal pos: ";
    for(double d : myGoalPos) cout << d<<" ";
    cout <<" - belief: "<< myGoalPosBelief<<endl;
    cout <<" - other goal pos: ";
    for(double d : otherGoalPos) cout << d<<" ";
    cout <<" - belief: "<< otherGoalPosBelief<<endl;
    cout<<endl;

    cout << "disconnecting in 1 s"<<endl;
    sleep(1);
    _fafConn->closeConnection();
}

void Tester::gotConnected() {
    cout << "** CONNECTED **"<<endl;
}

void Tester::gotDisconnected() {
    cout << "** DISCONNECTED **"<<endl;
    cout <<"Exiting in 1 s"<<endl;
    sleep(1);
    exit(0);
}
