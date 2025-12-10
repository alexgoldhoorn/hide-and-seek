#ifndef SIMULATORTEST_H
#define SIMULATORTEST_H

#include <QtTest/QtTest>

class SeekerHSParams;
class GMap;
namespace pomcp {
class Simulator;
class Belief;
class HSObservation;
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

    void hsSimulatorTestDiscSimple();

    void hsSimulatorTestContSimple();

    void hsSimulatorTestDiscOther();

    void hsSimulatorTestContOther();

    void hsSimulatorContTestContSimple();

    void hsSimulatorContTestContOther();

    void cleanupTestCase();


private:
    void testSimulator(pomcp::Simulator* sim, GMap* map, int maxStepDistTest=MAX_STEP_DIST_TEST, int numOtherSeekers=OBS_NUM_OTHER_SEEKERS);

    pomcp::HSObservation* genObs(GMap* map, int xRow, int xCol, int yRow, int yCol, int nOther);

    GMap* generateGMap();

    GMap* loadMap(QString name);

    //run the test function or not
    bool skipTestFunc(const char* funcName);

    SeekerHSParams* _params;

    QString _mapPath;

    static int MAX_STEP_DIST_TEST;
    static int MAX_STEP_DIST_TEST_WNOISE;
    static int OBS_NUM_OTHER_SEEKERS;
    static QString MAP_OTHER_NAME;

    //! list of test functions to skip
    static QList<QString> SKIP_TEST_FUNCS_LIST;
};



#endif
