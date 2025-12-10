#include "test/include/HSGame/gmap-test.h"

#include <QProcessEnvironment>
#include <iostream>

#include "Base/hsglobaldata.h"

using namespace std;

/* AG150625 TODO: things to test (more/better):
 * - distances (but already tested thoroughly before)
 * - dyn. obstacles + influences (idem)
 */

void GMapTest::initTestCase() {
    _params = new SeekerHSParams();
    _mapPath = QProcessEnvironment::systemEnvironment().value("HSPATH",".") + "/data/maps/";

}

void GMapTest::cleanupTestCase() {
    delete _params;
}

void GMapTest::generateMapNoSize() {
    GMap gmap(_params);
    gmap.createMap(10,12);
    gmap.addObstacle(1,2);
    gmap.addObstacle(Pos(2,3));
    gmap.setMapFixed();

    doMapTests(&gmap,10,12);

    QVERIFY(gmap.isObstacle(Pos(1,2)));
}

void GMapTest::generateMapWithSize() {
    GMap gmap(10,20, _params);
    gmap.setMapFixed();

    doMapTests(&gmap,10,20);

    QCOMPARE(gmap.distance(0,0,9,19),19.0);
}

void GMapTest::loadMap() {
    QString mapName = _mapPath+"/map3_10x10.txt";
    _params->useContinuousPos = true;
    GMap gmap(mapName.toStdString(), _params);

    doMapTests(&gmap,10,10);

    QVERIFY(!gmap.isVisible(Pos(0,0),Pos(9,8),false));
    QCOMPARE(gmap.distance(2,2,4,3),1.0+sqrt(2));
    QCOMPARE(gmap.distance(7,2,9,2),2.0*sqrt(2));
}

void GMapTest::doMapTests(GMap *gmap, int rows, int cols) {
    QVERIFY(gmap->isMapFixed());
    QCOMPARE(gmap->colCount(),cols);
    QCOMPARE(gmap->rowCount(),rows);

    for(int r=0;r<rows;r++) {
        for(int c=0;c<cols;c++) {
            QVERIFY(gmap->isPosInMap(r,c));
            QVERIFY(gmap->isObstacle(r,c) || gmap->isVisible(r,c,r,c,false));
        }
    }

    QVERIFY(!gmap->isPosInMap(-1,0));
    QVERIFY(!gmap->isPosInMap(rows,0));
    QVERIFY(!gmap->isPosInMap(0,cols));

    Pos base = gmap->getBase();
    if (base.row()<0 || base.col()<0 || base.rowDouble()<0 || base.colDouble()<0)
        QVERIFY(!base.isSet());
    else
        QVERIFY(base.isSet());

    gmap->printMap();
}

void GMapTest::streamTest() {
    QString mapName = _mapPath+"/map4_40x40.txt";
    _params->useContinuousPos = true;
    GMap gmap(mapName.toStdString(), _params);

    doMapTests(&gmap, 40, 40);

    //write
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(HSGlobalData::DATASTREAM_VERSION);
    gmap.writeMaptoStream(out);

    //read
    QDataStream in(block);
    GMap gmap2(_params);
    gmap2.readMapFromStream(in);

    //check if equal
    QCOMPARE(gmap2.rowCount(),gmap.rowCount());
    QCOMPARE(gmap2.colCount(),gmap.colCount());

    for(int r=0;r<gmap.rowCount();r++) {
        for(int c=0;c<gmap.colCount();c++) {
            QCOMPARE(gmap2.getItem(r,c),gmap.getItem(r,c));
            QCOMPARE(gmap2.isObstacle(r,c),gmap.isObstacle(r,c));
        }
    }
}

void GMapTest::moveTest() {
    GMap gmap(5,5, _params);
    _params->useContinuousPos = true;
    gmap.setMapFixed();
    Pos cpos(2.5,2.5);

    //gmap.tryMove(HSGlobalData::ACT_DIR_H, )
    Pos newPos = gmap.tryMoveDir(0, cpos, 1.0, true);
    QVERIFY(newPos.equals(1.5,2.5));
    newPos = gmap.tryMoveDir(M_PI_2, cpos, 1.0, true);
    QVERIFY(newPos.equals(2.5,3.5));

    cout <<" -,"<<cpos.rowDouble()<<","<<cpos.colDouble()<<endl;
    for(double a=0;a<=2.0*M_PI;a+=0.01) {
        newPos = gmap.tryMoveDir(a, cpos, 1.0, true);
        cout <<" "<<a<<", "<<newPos.rowDouble()<<","<<newPos.colDouble()<<endl; //TODO VERIFY, then random distances and verify new distance...
    }
    cout<<endl;

    //TODO: test trymove, and tryMoveDir

    //later for continuous/discr
}
