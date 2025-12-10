#ifndef POSSCOREMATTEST_H
#define POSSCOREMATTEST_H


#include <QtTest/QtTest>

#include "Utils/posscoremat.h"


/*!
 * \brief The PosScoreMat tester
 */
class PosScoreMatTest: public QObject
{
    Q_OBJECT

public:
    //PosScoreMatTest();

private slots:

    void initTestCase();

    void testScoreMat();

    void cleanupTestCase();

private:
    static constexpr auto POSSCOREMATTEST_FILE1 = "../../data/test/posscore1-test.txt";

};



#endif // POSSCOREMATTEST_H
