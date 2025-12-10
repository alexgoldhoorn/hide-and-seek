
#include <QCoreApplication>

#include "HSGame/gmap.h"


#include <iostream>

#include "ActionListGenerator/actionlistgenerator.h"

using namespace std;


int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);



    try
    {

        if (argc<4) {
            cout << "Error: expected 3 parameter: map_file list_out_file name"<<endl;
            exit(-1);
        }


        GMap map(argv[1]);
        //map.printMap();

        ActionListGenerator alg(&map);

        QString file(argv[2]);
        QString name = QString::fromLatin1(argv[3]);
        alg.generateFile(file,name);


    }
    catch(exception &e)
    {
        cout << "Exception: " << e.what() << endl ;
    }


    return 0;
}
