#include "test/include/POMCP/hsstate-test.h"
#include <QTest>


#include "POMCP/hsstate.h"

#include "Base/hsglobaldata.h"

using namespace std;
using namespace pomcp;

void HSStateTest::createTest() {
    HSState s1(1,2,3,4);
    HSState s2(Pos(3,4),Pos(5,6));
    Pos empty;
    HSState s3(empty,empty);

    QVERIFY(s1.hiderVisible());
    QVERIFY(s2.hiderVisible());
    QVERIFY(!s3.hiderVisible());
}

void HSStateTest::compareTest() {
    HSState s1(1,2,3,4);
    HSState s2(Pos(3,4),Pos(5,6));
    Pos empty;
    HSState s3(Pos(6,4),empty);
    HSState s4(Pos(1,2),Pos(3,4));

    QCOMPARE(s1,s4);
    QVERIFY(s1==s4);
    QVERIFY(s1!=s2);
    QVERIFY(s1<s2);
    QVERIFY(s2>s1);
    QVERIFY(s1.getHash()==s4.getHash());
    QVERIFY(s1.getHash()!=s2.getHash());

    QVERIFY(s2.hiderVisible());
    QVERIFY(!s3.hiderVisible());

    HSState s5(1.1,2.5,3.6,4.7);
    s5.convPosToInt();
    QVERIFY(s5==s1);

}
