#include "test/include/HSGame/pos-test.h"
#include <QTest>

#include "Base/hsglobaldata.h"
#include "HSGame/bpos.h"

void PosTest::createPos() {
    //int pos
    Pos p1(1,2);
    QCOMPARE(p1.row(),1);
    QCOMPARE(p1.col(),2);
    QCOMPARE(p1.rowDouble(),1.0);
    QCOMPARE(p1.colDouble(),2.0);
    QVERIFY(!p1.hasDouble());

    //double pos
    Pos p2(1.5,2.5);
    QCOMPARE(p2.row(),1);
    QCOMPARE(p2.col(),2);
    QCOMPARE(p2.rowDouble(),1.5);
    QCOMPARE(p2.colDouble(),2.5);
    QVERIFY(p2.hasDouble());
}


void PosTest::compareTest() {
    Pos p1(1,2);
    Pos p2(10.0,20.6);
    Pos p3(1,3);
    Pos p4(1,2);

    //equals test
    QVERIFY(!p1.equalsInt(p2));
    QVERIFY(p1.equals(1,2));
    QVERIFY(p1.equalsInt(1,2));
    QVERIFY(p2.equals(10.0,20.6));
    QVERIFY(p2.equalsInt(10,20));
    QVERIFY(p1!=p2);
    QVERIFY(p1!=p3);
    QVERIFY(p1==p4);
    //compare
    QVERIFY(p1<p2);
    QVERIFY(p3>p1);
    //hash check
    QVERIFY(p1.getHash()!=p2.getHash());
    QVERIFY(p1.getHash()==p4.getHash());
    //distance
    QCOMPARE(p1.distanceEuc(p4), 0.0);
    QCOMPARE(p1.distanceEuc(p3), 1.0);
    QVERIFY(abs(p1.distanceEuc(p2)-20.663)<0.001);
    //conver to int
    QVERIFY(p2.hasDouble());
    p2.convertValuesToInt();
    QVERIFY(!p2.hasDouble());
    QVERIFY(p2.equals(10.0,20.0));
}

void PosTest::streamTest() {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);

    Pos p1(1,2);
    Pos p2(33.4,5553.3);
    //write
    p1.writePostoStream(out, false);
    p2.writePostoStream(out, true);

    //read
    QDataStream in(block);
    Pos p3,p4;
    p3.readPosFromStream(in,false);
    p4.readPosFromStream(in,true);

    //test
    QVERIFY(p1==p3);
    QVERIFY(p2==p4);
}

void PosTest::bposTest() {
    BPos p1(1,2,0.3);
    BPos p2(0.5,1.0,0.5);
    QVERIFY(p1<p2);
    QVERIFY(p2>p1);

    BPos p3=p2;
    QVERIFY(p3==p2);
    QCOMPARE(p2.b,p3.b);

    Pos px(10,20);

    Pos px2;
    px2=px;

    qDebug()<<"setting p1:"<< QString::fromStdString(p1.toString())<<" to "<<QString::fromStdString(px.toString());
    p1=px;
    qDebug()<<"result: "<<QString::fromStdString(p1.toString());
    QVERIFY(p1==px);
    QCOMPARE(p1.b,0.3);
    QCOMPARE(p1.row(),10);

    //test to set BPos to Pos
    Pos pc;
    pc = p1;
    QCOMPARE(pc.rowDouble(), p1.rowDouble());
    QCOMPARE(pc.colDouble(), p1.colDouble());

    BPos p4(0.5,1.0,0.5);
    qDebug() << "Comparing: "<<QString::fromStdString(p1.toString())<<" to "<<QString::fromStdString(p4.toString());

    QCOMPARE(p4,p2);
    QVERIFY(p4==p2);
}

void PosTest::streamTestBPos() {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);

    BPos p1(1,2,0.3);
    BPos p2(0.5,1.0,0.5);
    //write
    p1.writePostoStream(out, false);
    p2.writePostoStream(out, true);

    //read
    QDataStream in(block);
    BPos p3,p4;
    p3.readPosFromStream(in,false);
    p4.readPosFromStream(in,true);

    //test
    QVERIFY(p1==p3 && p1.b==p3.b);
    QVERIFY(p2==p4 && p2.b==p4.b);
}
