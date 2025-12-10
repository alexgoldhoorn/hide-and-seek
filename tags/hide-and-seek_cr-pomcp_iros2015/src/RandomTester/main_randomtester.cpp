
#include <QCoreApplication>

#include "HSGame/gmap.h"


#include <iostream>

#include "RandomTester/randomtester.h"

using namespace std;


int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);



    try
    {

        if (argc<3) {
            cout << "Error: expected 2 parameter: map_file n"<<endl;
            exit(-1);
        }


        GMap map(argv[1]);
        map.printMap();

        RandomTester rtester(&map);

        QString numStr(argv[2]);

        int n = numStr.toInt();

        double std,mean;
        mean=rtester.testMap(n,std);
        cout << endl << endl << mean << ","<<std<<endl;


    }
    catch(exception &e)
    {
        cout << "Exception: " << e.what() << endl ;
    }


    return 0;
}
