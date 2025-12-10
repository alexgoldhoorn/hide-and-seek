#include <QApplication>
#include "GUI/gmapwidget.h"

#include <QPainter>
#include <QString>

#include <iostream>
#include <limits>
#include <QtTest>
#include "test/include/PathPlan/astarpathplanner-test.h"

#include "Utils/generic.h"
#include "GUI/hsclientmainwindow.h"
#include "Base/playerinfo.h"

using namespace std;
using namespace hsutils;

void showHelp() {

    cout << "hsclient [ ! | -O | mapfile cell-size zoomOutF | -c width height | -? | -h ]"<<endl<<endl
         << "Without parameters the server:port will be asked to the user."<<endl
         << "The -O or ! parameter allows the user to pass another observation than the correct one (for debug purpose)."<<endl
         << "The other parameters:"<<endl
         << "-O | !     pass an other observation than the correct one"<<endl
         << "-q         click next position"<<endl
         << "-? | -h    this help"<<endl
         << "-t         test"<<endl
         << "-c w h     create a new map with the size of w x h"<<endl<<endl;
    QCoreApplication::exit(0);
}

void doTest() {
    /*AStarPathPlannerTest test;
    QObject* obj = &test;
    int e = QTest::qExec(obj); //,argc,argv);*/

    //AStarPathPlannerTest test();
    //QTest::qExec(&test);
    //test.test();

}


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    //ag140521: reset locale after Qt sets to system's local (http://qt-project.org/doc/qt-5/QCoreApplication.html#locale-settings)
    setlocale(LC_NUMERIC,"C");
    //ag120208: set locale to en-US such that the atof uses decimal dots and not comma
    setlocale(LC_NUMERIC,"en_US");

/*    PlayerInfo info;
    cout << info.toString()<<endl;
    cout << info.toString(true,true,true)<<endl;
    info.setUserName("TEST");
    info.playerType=HSGlobalData::P_Seeker;
    info.currentPos.set(10,3);
    info.hiderObsPosWNoise.set(0,2);
    cout << info.toString()<<endl;
    cout << info.toString(true,true,true)<<endl;
*/


    GMapWidget* gmapWidget = NULL;

    cout << "Hide & Seek client"<<endl;
    try {
        bool useMouseObs = false;
        bool allowClickNextPos = false;
        int width = 0;
        int height = 0;
        GMap* map = NULL;

        if (argc>1 && (argv[1][0]=='!' || argv[1][0]=='-')) {

            useMouseObs = (argv[1][0]=='!');
            if (argv[1][0]=='-' && strlen(argv[1])==2) {
                switch (argv[1][1]) {
                    case 'O':
                        useMouseObs = true;
                        break;
                    case 'q':
                        allowClickNextPos = true;
                        break;
                    case '?':
                    case 'H':
                    case 'h':
                        showHelp();
                        break;
                    case 't':
                        doTest();
                        return 0;
                        break;
                    case 'c': {
                        QString intStr = QString::fromLatin1(argv[2]);
                        bool ok = false;
                        width = intStr.toInt(&ok);
                        if (!ok) {
                            cout << "expected width"<<endl;
                            showHelp();
                        }
                        intStr = QString::fromLatin1(argv[3]);
                        height = intStr.toInt(&ok);
                        if (!ok) {
                            cout << "expected height"<<endl;
                            showHelp();
                        }

                        cout <<"Creating map: "<<flush;
                        map = new GMap(height,width,new SeekerHSParams());
                        map->setMapFixed();
                        cout<<"ok"<<endl;
                        break;
                    }
                    default:
                        cout << "Unknown parameter!"<<endl;
                        showHelp();
                    break;
                }
            } else {
                showHelp();
            }

            if (useMouseObs || allowClickNextPos) {
                gmapWidget = new GMapWidget(useMouseObs,allowClickNextPos, argsToQString(argc,argv),"hsclient");
                /*w.show();
                return a.exec();*/
            } else {
                //assume map to create

                gmapWidget = new GMapWidget(map);
                /*w.show();
                return a.exec();*/
            }


        } else if (argc>1) { // && !useMouseObs) {
            cout << "Opening map: "<<argv[1]<<endl;

            int zoomOutF = 1;
            if (argc>3) {
                QString zstr = QString::fromLatin1(argv[3]);
                bool ok = false;
                zoomOutF = zstr.toInt(&ok);
            }

            GMap* gmap = NULL;

            //GMap gmap(argv[1]);
            QString mapFStr = QString::fromLatin1(argv[1]);
            bool isPGM = ( mapFStr.right(4).compare(".pgm",Qt::CaseInsensitive) == 0 );
            //cout << "'"<<mapFStr.right(4).toStdString()<<"' ispgm:"<<isPGM<<endl;
            if (isPGM) {
                int row, col;

                cout <<"Base: row: "<<flush;
                cin >> row;
                cout << endl<<"col: "<<flush;
                cin >> col;
                cout << endl;

                //gmap->setBase(row,col);
                Pos base(row,col);
                gmap = new GMap(argv[1],base,zoomOutF,NULL);
            } else {
                gmap = new GMap(argv[1],NULL);
            }

            cout << "Map size: "<<gmap->rowCount()<<"x"<<gmap->colCount()<<endl;
            cout << "Obstacles: "<<gmap->numObstacles()<<", Free Cells: "<<gmap->numFreeCells()<<endl;
            cout << "Cell size: "<<gmap->getCellSize_m()<<" m"<<endl;
            if (gmap->colCount()<=50 && gmap->rowCount()<=50) {
                gmap->printMap();
            }

            cout << "ok"<<endl;
            gmapWidget = new GMapWidget(gmap);

            if (argc>2) {
                QString wstr = QString::fromLatin1(argv[2]);
                bool ok = false;
                int width = wstr.toInt(&ok);
                if (ok) gmapWidget->resize(width,width);
            }

            //return a.exec();
        } else {
            gmapWidget = new GMapWidget(false,false, argsToQString(argc,argv),"hsclient");
        }

        if (gmapWidget==NULL) {
            cout << "ERROR: no gmap widget set"<<endl;
            return -1;
        } else {
            HSClientMainWindow mainW(gmapWidget);

            mainW.show();

            int r = a.exec();

            delete gmapWidget;

            return r;
        }


    } catch(exception &e) {
        cout << "Exception: " << e.what() << endl ;
    }

    return 0;
}
