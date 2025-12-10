#include "test/include/HSGame/gmap-test.h"

#include <QProcessEnvironment>
#include <iostream>
#include <QDebug>

#include "Base/hsglobaldata.h"

using namespace std;

/* AG150625 TODO: things to test (more/better):
 * - distances (but already tested thoroughly before)
 * - dyn. obstacles + influences (idem)
 */

void GMapTest::initTestCase() {
    _params = new SeekerHSParams();
    _mapPath = QProcessEnvironment::systemEnvironment().value("HSPATH",".") + "/data/maps/";

    visibTest();
}

void GMapTest::cleanupTestCase() {
    delete _params;
}

void GMapTest::generateMapNoSize() {
    _params = new SeekerHSParams();
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
    QString mapName = _mapPath+"map3_10x10.txt";
    qDebug() << "Loading map: "<<mapName;
    _params->useContinuousPos = true;
    GMap gmap(mapName.toStdString(), _params);

    doMapTests(&gmap,10,10);

    QVERIFY(!gmap.isVisible(Pos(0,0),Pos(9,8),false,_params->simNotVisibDist));
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
            QVERIFY(gmap->isObstacle(r,c) || gmap->isVisible(r,c,r,c,false,_params->simNotVisibDist));
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
    //QString mapName = _mapPath+"/map4_40x40.txt";
    QString mapName = _mapPath+"/brl/master29e.txt";
    _params->useContinuousPos = true;
    GMap gmap(mapName.toStdString(), _params);

    doMapTests(&gmap, 75, 69);

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
    QCOMPARE(gmap2.getCellSize_m(),gmap.getCellSize_m());


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

void GMapTest::visibTest() {
    GMap gmap(11,5, _params);
    gmap.addObstacle(1,2);
    _params->useContinuousPos = true;
    gmap.setMapFixed();
    _params->setNotMaxVisDist(1.0);
    QCOMPARE(gmap.getCellSize_m(), 1.0);
    QCOMPARE(_params->cellSizeM,1.0);

    QCOMPARE(_params->simNotVisibDist_maxX, 8.0);

    //visible, dist=2
    QVERIFY(gmap.isVisible(Pos(0.5,4.5),Pos(2.5,4.5),true,true));
    //visible, dist=8
    QVERIFY(gmap.isVisible(Pos(0.5,4.5),Pos(8.39,4.5),true,true));
    //not visib, obst
    QVERIFY(!gmap.isVisible(Pos(0.5,2.5),Pos(2.5,2.5),true,true));
    //not visib, distance (>8)
    QVERIFY(!gmap.isVisible(Pos(0.5,4.5),Pos(9.5,4.5),true,true));

    //visible prob, dist=2
    QCOMPARE(gmap.getVisibilityProb(Pos(0.5,4.5),Pos(2.5,4.5),true),0.85);
    //not visib prob, obst
    QCOMPARE(gmap.getVisibilityProb(Pos(0.5,2.5),Pos(2.5,2.5),true),0.0);
    //not visib prob, distance (>8)
    QCOMPARE(gmap.getVisibilityProb(Pos(0.5,4.5),Pos(9.49,4.5),true),0.);

    //dyn. obst
    vector<IDPos> dynObstVec;
    dynObstVec.push_back(IDPos(2.0,4.5,1));
    QVERIFY(!gmap.isVisible(Pos(0.5,4.5),Pos(2.5,4.5),false,true,&dynObstVec));
    QCOMPARE(gmap.getVisibilityProb(Pos(0.5,4.5),Pos(2.5,4.5),false,&dynObstVec),0.);
    //now set dyn obst in gmap
    gmap.setDynamicObstacles(dynObstVec);
    QVERIFY(!gmap.isVisible(Pos(0.5,4.5),Pos(2.5,4.5),true,true));
    QCOMPARE(gmap.getVisibilityProb(Pos(0.5,4.5),Pos(2.5,4.5),true),0.);

    //now change cell size
    _params->setNotMaxVisDist(0.8);
    gmap.setCellSize_m(0.8);
    QCOMPARE(_params->cellSizeM,0.8);
    QCOMPARE(_params->simNotVisibDist_maxX,10.);
    QCOMPARE(_params->simNotVisibDist_x0,3.75);

    //visible, dist=2
    QVERIFY(gmap.isVisible(Pos(0.5,4.5),Pos(2.5,4.5),false,true));
    //visible, dist=8
    QVERIFY(gmap.isVisible(Pos(0.5,4.5),Pos(10.39,4.5),false,true));
    //not visib, distance (>8)
    QVERIFY(!gmap.isVisible(Pos(0.5,4.5),Pos(10.9,4.5),false,true));

    //visible prob, dist=2
    QCOMPARE(gmap.getVisibilityProb(Pos(0.5,4.5),Pos(4.2,4.5),false),0.85);
    //not visib prob, distance (>8)
    QCOMPARE(gmap.getVisibilityProb(Pos(0.5,4.5),Pos(10.9,4.5),false),0.);
}

#ifdef TO_FIX

//--- unit tests ---

void GMap::testVisibility() {
    bool b1,b2;
    unsigned int inc=0,tc=0,viscount=0;
    cout << "Testing visiblitiy: "<<endl;
    FORs(r1,_rows) {
        FORs(c1,_cols) {
            if (!isObstacle(r1,c1)) {
                FORs(r2,_rows) {
                    FORs(c2,_cols) {
                        if (!isObstacle(r2,c2)) {
                            b1 = isVisible(r1,c1,r2,c2,false,false);
                            b2 = isVisible(r2,c2,r1,c1,false,false);
                            if (b1!=b2) {
                                inc++;
                                cout <<  "r"<<r1<<"c"<<c1<<"-r"<<r2<<"c"<<c2<<": "<<b1<<" - other way: "<<b2<<endl;
                            }
                            if (b1||b2) viscount++;
                            tc++;
                        }
                    }
                }
            }
        }
    }
    if (inc>0)
        cout << "ERROR: "<<inc<<" of "<<tc<<" inconsistent"<<endl;
    else
        cout << "OK! all "<<tc<<" states ok, visible: "<<viscount<<endl;
}

void GMap::testDistance() {
    float b1,b2;
    //int n = 0;
    unsigned int inc=0,tc=0;
    int n=numFreeCells();
    long n2=n*n;
    Timer timer;
    //long n3=n2/2;
    long i=0;
    long tot =0;
    long maxD = 0;

    cout <<"States: "<<n<<"; combis:"<<n2<<endl;

    cout << "Testing distance: "<<endl;
    /*
    FORs(r1,_rows) {
        FORs(c1,_cols) {
            if (!isObstacle(r1,c1)) {
                FORs(r2,_rows) {
                    FORs(c2,_cols) {
     */
    int start= timer.startTimer();
    for(int r1=0;r1<_rows-1;r1++) {
        for(int c1=0;c1<_cols-1;c1++) {
            if (!isObstacle(r1,c1)) {
                for(int r2=r1+1;r2<_rows;r2++) {
                    for(int c2=c1+1;c2<_cols;c2++) {
                        if (!isObstacle(r2,c2)) {
                            int timerID = timer.startTimer();
                            b1 = distance(r1,c1,r2,c2);
                            b2 = distance(r2,c2,r1,c1);

                            long t = timer.stopTimer(timerID);
                            i++;

                            /*if (i*t % 10000000 ==0) {
                                cout << t <<" "<< flush;
                            }*/
                            tot+=t;
                            if (b1!=b2) {
                                inc++;
                                cout <<  "r"<<r1<<"c"<<c1<<"-r"<<r2<<"c"<<c2<<": "<<b1<<" - other way: "<<b2<<endl;
                            }
                            if (b1>maxD) maxD = b1;
                            if (b2>maxD) maxD = b2;
                            tc++;
                        }
                    }
                }
            }
        }
    }
    long tot2 = timer.stopTimer(start);
    cout <<"total time: "<<tot<<" s = "<<(tot/3600.0)<<"h"<<endl;
    cout <<"total time: "<<tot2<<" s = "<<(tot2/3600.0)<<"h"<<endl;
    cout <<"max distance: "<<maxD<<endl;
    if (inc>0)
        cout << "ERROR: "<<inc<<" of "<<tc<<" inconsistent"<<endl;
    else
        cout << "OK! all "<<tc<<" states ok"<<endl;
}

#endif
