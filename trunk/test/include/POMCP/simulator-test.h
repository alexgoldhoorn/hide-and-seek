#ifndef SIMULATORTEST_H
#define SIMULATORTEST_H

#include <QtTest/QtTest>

class SeekerHSParams;
class GMap;
namespace pomcp {
class Simulator;
class Belief;
class HSObservation;
class State;
class Observation;
class HSState;
}

/*!
 * \brief The HSStateTest class Test Simulators: HSSimulator, HSSimulatorCont
 */
class SimulatorTest: public QObject
{
    Q_OBJECT
private slots:

    void initTestCase();

    void testSkipFunc();

    void hsSimulatorTestConsistencyCheck();

    void hsSimulatorTestDiscSimple();

    void hsSimulatorTestContSimple();

    void hsSimulatorTestDiscOther();

    void hsSimulatorTestContOther();

    void hsSimulatorContTestContSimple();

    void hsSimulatorContTestContOther();

    void cleanupTestCase();


private:
    //! times to repeat probability functions
    const int REP_PROB_FUNCS = 100;
    //! amount of probabilistic true when winning checkConsistency
    const double CONSIST_CHECK_PROB = 0.7;

    void testSimulator(pomcp::Simulator* sim, GMap* map, int maxStepDistTest=MAX_STEP_DIST_TEST, int numOtherSeekers=OBS_NUM_OTHER_SEEKERS);

    pomcp::HSObservation* genObs(GMap* map, int xRow, int xCol, int yRow, int yCol, int nOther);

    //! check the consistency REP_PROB_FUNCS times, and return part (0-1) times it returned true
    double checkStateConsistentWithObs(pomcp::Simulator& sim, const pomcp::State& state, const pomcp::Observation* obs);

    //! check if it can be consistent, return prob.
    double checkStateConsistentWithObsProb(pomcp::Simulator& sim, const pomcp::HSState* hsstate, const pomcp::HSObservation* hsobs);

    //! check the step
    void stepCheck(pomcp::Simulator *sim, const pomcp::HSState &state, int a, bool genOutObs=true, bool useInObs=false /*const pomcp::HSObservation* obs=NULL*/,
                   int numOtherSeekers=OBS_NUM_OTHER_SEEKERS);

    GMap* generateGMap();

    GMap* loadMap(QString name);

    //run the test function or not
    bool skipTestFunc(const char* funcName);

    SeekerHSParams* _params;

    //! map globally avaialable (for easier argument passing, should be set for each test function)
    GMap* _map;

    QString _mapPath;

    static int MAX_STEP_DIST_TEST;
    static int MAX_STEP_DIST_TEST_WNOISE;
    static int OBS_NUM_OTHER_SEEKERS;
    static QString MAP_OTHER_NAME;

    //! list of test functions to skip
    static QList<QString> SKIP_TEST_FUNCS_LIST;
};



#endif
