#include "test/include/Utils/posscoremat-test.h"

#include <string>

/*PosScoreMatTest::PosScoreMatTest() {

}*/

void PosScoreMatTest::initTestCase() {

}

void PosScoreMatTest::testScoreMat() {
    PosScoreMat posScoreMat(POSSCOREMATTEST_FILE1);
    posScoreMat.loadScores(4,4);

    //test all
    QCOMPARE(posScoreMat.getScore(0,0),0.00140786);
    QCOMPARE(posScoreMat.getScore(0,1),0.0014674);
    QCOMPARE(posScoreMat.getScore(0,2),-1.0);
    QCOMPARE(posScoreMat.getScore(1,1),1.0);
    QCOMPARE(posScoreMat.getScore(1,2),0.00000033);
    QCOMPARE(posScoreMat.getScore(2,2),0.993);
    QCOMPARE(posScoreMat.getScore(2,0),-1.0);
    QCOMPARE(posScoreMat.getScore(3,2),0.5);
    QCOMPARE(posScoreMat.getScore(3,3),0.0);

    /*
10,
[0,0][0.00140786]
[0,1][0.0014674]
[0,2][-1]
[1,1][1]
[1,2][0.00000033]
[2,2][0.993]
[2,0][-1]
[3,1][0.003]
[3,2][0.5]
[3,3][0]
     */

}

void PosScoreMatTest::cleanupTestCase() {

}
