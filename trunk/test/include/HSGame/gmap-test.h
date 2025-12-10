#ifndef GMAPTEST_H
#define GMAPTEST_H

#include <QtTest/QtTest>

#include "HSGame/gmap.h"

class GMapTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();

    void generateMapNoSize();
    //! generates a map and does tests
    void generateMapWithSize();
    //! loads map and does tests
    void loadMap();

    void streamTest();

    void moveTest();

    void cleanupTestCase();

    //AG160503
    //! test visibility
    void visibTest();

//AG160216: TODO put these, they were in GMap
#ifdef TO_FIX
    // unit tests
    //! test if visiblity(x,y)==visibility(y,x)
    //! if true-> we can put it in the matrix
    void testVisibility();
    //! distance test
    void testDistance();
#endif

private:
    void doMapTests(GMap* gmap, int rows, int cols);

    SeekerHSParams* _params;

    QString _mapPath;
};

/*QTEST_MAIN(GMapTest)
#include "GMapTest.moc"*/

#endif
