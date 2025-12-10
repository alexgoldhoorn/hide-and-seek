#include <QtGui/QApplication>
#include "simmaingui.h"
#include "gmapwidget.h"
//#include "HSGame/gmap.h"

#include <QPainter>
#include <iostream>


using namespace std;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GMapWidget* w;

    cout << "Hide & Seek client"<<endl;
    try {
        if (argc>1) {
            cout << "Opening map: "<<argv[1]<<endl;
            GMap gmap(argv[1]);
            cout << "ok"<<endl;
            GMapWidget w(&gmap);
            w.show();

            if (argc>2) {
                QString wstr = QString::fromAscii(argv[2]);
                bool ok = false;
                int width = wstr.toInt(&ok);
                if (ok) w.resize(width,width);
            }

            return a.exec();
        } else {
            GMapWidget w;
            w.resize(100,100);
            //used for test of action list
            //w._actionFile = QString::fromAscii(argv[1]);
            w.show();
            return a.exec();
        }


    }
    catch(exception &e)
    {
        cout << "Exception: " << e.what() << endl ;
    }

    return 0;
}
