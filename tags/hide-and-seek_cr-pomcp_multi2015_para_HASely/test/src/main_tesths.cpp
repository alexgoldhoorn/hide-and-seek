/*  TestHS
 *  ------
 * Unit test using QTest.
 *
 * This class starts the unit tests.
 * To compile:
 * add 'CONFIG+=USE_TEST_HS' to the Qt Project file hsmomdp.pro.
 *
 * The Qt Test parameters can be passed by default.
 * There is a list of tests which can be done, by default all these tests are executed.
 * To pass a filter, the first parameter should be: ~text
 * i.e. starting with '~', and then the text to filter.
 *
*/
#include <QtCore/QCoreApplication>

//ag120208: added to prevent atof() being dependend on the systems locale, later set with setlocale()

#include "exceptions.h"

#include <QDebug>
#include <QtTest>
#include <QList>

#include "test/include/HSGame/gmap-test.h"
#include "test/include/HSGame/pos-test.h"

#include "test/include/POMCP/hsstate-test.h"
#include "test/include/POMCP/hsobservation-test.h"
#include "test/include/POMCP/node-test.h"
#include "test/include/POMCP/simulator-test.h"
#include "test/include/POMCP/mcts-test.h"
#include "test/include/Server/clientsocket-test.h"
#include "test/include/Smart/multiseekerhbexplorer-test.h"
#include "test/include/Base/playerinfowrapper-test.h"

#include "Utils/consoleutils.h"
#include "Utils/timer.h"


//! result struct for test objects
struct ObjTestResult {
    ObjTestResult() {
        testObj = NULL;
        numTestFuncFailed = 0;
        runTime_ms = 0;
    }

    QObject* testObj;
    int numTestFuncFailed;
    long runTime_ms;
};


int main(int argc, char *argv[])
{
    //timer
    Timer timer;
    int timerID;

    //create test objects
    GMapTest gmapTest;
    PosTest posTest;
    HSStateTest hsStateTest;
    HSObservationTest hsObservationTest;
    PomcpNodeTest pomcpNodeTest;
    SimulatorTest simulatorTest;
    MCTSTest mctsTest;
    ClientSocketTest clientSocketTest;
    MultiSeekerHBExplorerTest multiSeekerHBExplorerTest;
    PlayerInfoWrapperTest playerInfoWrapperTest;


    //list of objects to test
    QList<QObject*> allTestObjects = {&posTest, &gmapTest, &hsStateTest, &hsObservationTest, &pomcpNodeTest,
                                     &simulatorTest, &mctsTest, &clientSocketTest, &multiSeekerHBExplorerTest,
                                     &playerInfoWrapperTest};

    //selected objects
    QList<QObject*> testObjects;


    //check arguments and filter test objects
    if (argc>1 && argv[1][0]=='~') {
        //filter
        QString filterName = QString::fromLatin1(argv[1]).toLower();
        filterName = filterName.right(filterName.length()-1);
        qDebug() << "Test filter: "<<filterName;
        for (QObject* testObj : allTestObjects) {
            QString className = QString::fromLatin1(testObj->metaObject()->className()).toLower();
            if (className.contains(filterName)) {
                testObjects.append(testObj);
            }
        }

        //now remove this param
        for(int i=1; i<argc-1;i++)
            argv[i]=argv[i+1];
        argc--;
    } else {
        //all objects
        testObjects = allTestObjects;
    }


    //counter for failed
    int failedObjTot = 0;
    int failedTestFuncTot = 0;

    qDebug() << endl<<"---| STARTING "<<testObjects.size()<<" TESTS |---"<<endl;

    //list of test object results
    QList<ObjTestResult> resList;

    //test all
    for(QObject* testObj : testObjects) {
        ObjTestResult objRes;
        objRes.testObj = testObj;

        timerID = timer.startTimer();

        try {
            //execute QTest
            objRes.numTestFuncFailed = QTest::qExec(testObj, argc, argv);
        } catch(CException& ce) {
            qDebug() << "-->" << CONS_RED_BOLD<<"CException: "<<CONS_RESET<<QString::fromStdString(ce.what());
            objRes.numTestFuncFailed = 1;
        }

        objRes.runTime_ms = timer.getTime_ms(timerID);

        if (objRes.numTestFuncFailed > 0) {
            //failed
            qDebug() << "-->" << CONS_RED_BOLD<< testObj->metaObject()->className() <<" FAILED "<<objRes.numTestFuncFailed
                     <<" TEST FUNCTIONS"<<CONS_RESET<< endl;
            failedTestFuncTot+=objRes.numTestFuncFailed ;
            failedObjTot++;
        }

        resList.append(objRes);
    }

    //summary
    if (failedObjTot == 0) {
        qDebug() <<endl<<"---| "<<CONS_GREEN_BOLD<< "ALL "<< testObjects.size()<<" TEST OBJECTS PASSED"<<CONS_RESET" |---"<< endl;
    } else {
        qDebug() <<endl<<"---| "<<CONS_RED_BOLD<< failedTestFuncTot << " TEST FUNCTIONS OF "<<failedObjTot<<" / "<< testObjects.size()
                   << " TEST OBJECTS FAILED" <<CONS_RESET" |---";
    }

    //show results
    qDebug() << "Tests:";
    for(int i=0; i<resList.size(); i++) {
        ObjTestResult& res = resList[i];
        qDebug() << "  "<<i<<") "<< res.testObj->metaObject()->className()<< ": run "<<res.runTime_ms/1000.0<<" s, "
                 <<res.numTestFuncFailed<<" failed functions";
    }
    qDebug();

    return failedTestFuncTot;
}

