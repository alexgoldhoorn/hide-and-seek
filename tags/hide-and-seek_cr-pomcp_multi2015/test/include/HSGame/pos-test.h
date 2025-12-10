#ifndef POSTEST_H
#define POSTEST_H

#include <QtTest/QtTest>

class PosTest: public QObject
{
    Q_OBJECT
private slots:

    void createPos();

    void compareTest();

    void streamTest();

    void bposTest();

    void streamTestBPos();

private:
};



#endif
