#include <QtCore/QCoreApplication>

#include <iostream>
#include <limits>

#include "Utils/generic.h"
#include "HSGame/gmap.h"

#include <QSharedMemory>
#include <QBuffer>
#include <QVector>

 #include <unistd.h>

using namespace std;

void showHelp() {

    cout << "hsclient [ ! | -O | mapfile cell-size zoomOutF | -c width height | -? | -h ]"<<endl<<endl
         << "Without parameters the server:port will be asked to the user."<<endl
         << "The -O or ! parameter allows the user to pass another observation than the correct one (for debug purpose)."<<endl
         << "The other parameters:"<<endl
         << "-O | !     pass an other observation than the correct one"<<endl
         << "-q         click next position"<<endl
         << "-? | -h    this help"<<endl
         << "-c w h     create a new map with the size of w x h"<<endl<<endl;
    exit(0);
}

void writeshm(QSharedMemory& sharedMemory) {
    cout << "Writing: ";

    //int a[5]={1,2,3,4,5};

    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);

    //write item by item
    cout<<"inserting: ";
    for (int i=0; i<10; i++) {
        int x=i*i;
        cout<<" "<<x;
        out << x; //a[i];
    }
    cout<<endl;

    int size = buffer.size();

   /* if (sharedMemory.isAttached()) {
        if (!sharedMemory.detach()) {
            cout << "Unable to detach attached shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
            return;
        } else {
            cout << "Deattached successfully."<<endl;
        }
    }*/

    if (!sharedMemory.create(size)) {
        cout << "Unable to create shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
        return;
    }
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    const char *from = buffer.data().data();
    memcpy(to, from, qMin(sharedMemory.size(), size));

    sharedMemory.unlock();


    cout<<"Writing done"<<endl;
    /*for (int x : a) cout <<" "<<x;
    cout <<endl;*/
}


void readshm(QSharedMemory& sharedMemory, char id) {
    cout<<"Reading "<<id<<endl;
    sharedMemory.attach();
    QBuffer buffer;
    QDataStream in(&buffer);

    sharedMemory.lock();
    buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);

    int len = sharedMemory.size()/sizeof(int);
    int* a = new int[len];

    for (int i=0; i< len; i++) {
        in >> a[i];
    }

    sharedMemory.unlock();


    cout<<"Read: "<<endl;
    for(int i=0;i<len;i++) cout <<" "<<a[i];
    cout << endl;
    delete a;
}


void readshm2(QSharedMemory& sharedMemory, char id) {
    int i = id-'0';
    cout<<"Reading "<<i<<endl;
    if (!sharedMemory.attach()) {
        cout <<"Failed to attach"<<endl;
        return;
    }
    QBuffer buffer;
    QDataStream in(&buffer);

    sharedMemory.lock();
    buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);

    //sharedMemory.lock();
    const int* data = (const int*)sharedMemory.data();
    cout <<" reading index "<<i<<": "<<data[i]<<endl;

    int len = sharedMemory.size()/sizeof(int);
    int* a = new int[len];

    for (int i=0; i< len; i++) {
        in >> a[i];
        cout<<" "<<a[i];
    }
    cout<<endl;

    sharedMemory.unlock();


    //cout<<"Read: "<<x<<endl;
    /*for(int i=0;i<len;i++) cout <<" "<<a[i];
    cout << endl;
    delete a;*/
}

void writeshmQV(QSharedMemory& sharedMemory) {
    cout << "Writing: ";

    //int a[5]={1,2,3,4,5};

    QVector<int> vec;

    //vec << 1<<2<<3<<4<<5;
    for (int i=0; i<10; i++) {
        int x=i*i;
        vec << x;
    }

    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);

    //write item by item
    out << vec;

    int size = buffer.size();

   /* if (sharedMemory.isAttached()) {
        if (!sharedMemory.detach()) {
            cout << "Unable to detach attached shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
            return;
        } else {
            cout << "Deattached successfully."<<endl;
        }
    }*/

    if (!sharedMemory.create(size)) {
        cout << "Unable to create shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
        return;
    }
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    const char *from = buffer.data().data();
    memcpy(to, from, qMin(sharedMemory.size(), size));

    sharedMemory.unlock();


    cout<<"Writing done"<<endl;
    for (int x : vec) cout <<" "<<x;
    cout <<endl;
}


void readshmQV(QSharedMemory& sharedMemory, char id) {
    cout<<"Reading "<<id<<endl;
    sharedMemory.attach();
    QBuffer buffer;
    QDataStream in(&buffer);

    sharedMemory.lock();
    buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);

    int len = sharedMemory.size()/sizeof(int);
    QVector<int> vec;

    in>>vec;

    sharedMemory.unlock();


    cout<<"Read: "<<endl;
    for (int x : vec) cout <<" "<<x;
    cout << endl;

}


void readshmQV2(QSharedMemory& sharedMemory, char id) {
    cout<<"Reading "<<id<<endl;
    sharedMemory.attach();
    QBuffer buffer;
    QDataStream in(&buffer);

    sharedMemory.lock();
    buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);

    int len = sharedMemory.size()/sizeof(int);
    QVector<int> vec;

    in>>vec;

    /*cout << "casting data:"<<flush;
    const QVector<int>* vec = (const QVector<int>*)sharedMemory.constData();
    cout<<"ok"<<endl<<"data:"<<endl;

    int i=(int)(id-'0');
    cout <<"data at index "<<i<<": "<<(*vec)[i]<<endl;
*/



    sharedMemory.unlock();


    cout<<"Read: "<<endl;
    for (int x : vec) cout <<" "<<x;
    cout << endl;

}

int main2(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QSharedMemory shMem("test2");


    if (argc>1) {
        readshmQV2(shMem, argv[1][0]);

    } else {
        writeshmQV(shMem);
        return a.exec();
    }
    return 0;
}


class Test {
public:
    int id;
    int test;
    int n;

    const int DIM2=2;

    double** arr;

    Test() : id(-1),test(0),n(0),arr(NULL) {}

    void init(int n, int id, int test) {
        this->id=id;
        this->test=test;
        this->n = n;
        arr = new double*[DIM2];

        //if (n>0) arr[0]=test;
        for(int j=0;j<DIM2;j++) {
            arr[j]=new double[n];
            arr[j][0]=test;
            for(int i=1;i<n;i++)
                arr[j][i]=arr[j][i-1]*test;
            test+=1;
        }
    }

    ~Test() {
        //delete [] arr;
    }


    void show() const {
        cout <<" test id="<<id<<" test="<<test<<", arr[#"<<n<<"]: "<<endl;
        for(int j=0;j<DIM2;j++) {
            for(int i=0;i<n;i++)
                cout<<arr[j][i]<<" "<<flush;
            cout<<endl;
        }
        cout <<endl;
    }

    void inc() {
        for(int i=0;i<n;i++)
            arr[1][i]+=0.1;
    }

    void writeOLD(QSharedMemory& sharedMemory) {
        int size = sizeof(id)+sizeof(test)+sizeof(n)+n*sizeof(double);

        cout << "Writing, size="<<size<<endl;

        if (!sharedMemory.create(size)) {
            cout << "Unable to create shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
            return;
        }

        sharedMemory.lock();
        QBuffer buffer;

        cout<<"buf init pos:"<<buffer.pos()<<endl;

        buffer.setData((const char*)sharedMemory.data(),size);
        buffer.open(QBuffer::ReadWrite);

        cout<<"buf in setdata pos:"<<buffer.pos()<<endl;

        buffer.write((char*)&id,sizeof(id));
        buffer.write((char*)&test,sizeof(test));
        /*buffer.write((char*)&n,sizeof(n));
        buffer.write((char*)arr,n*sizeof(double));

        cout<<"buf end pos:"<<buffer.pos()<<endl;
        cout<<"copy array to mem: "<<flush;
        char *to = (char*)sharedMemory.data();
        const char *from = (const char*)(arr + buffer.pos());
        memcpy(to,from,n*sizeof(double));*/

        //for (int i=0;i<n;i++)
        /*char *to = (char*)sharedMemory.data();
        const char *from = (const char*)tarr; //buffer.data().data();
        memcpy(to, from, qMin(sharedMemory.size(), size))*/

        sharedMemory.unlock();

        //TODO: check data
        int tid,ttest;
        cout<<"testing from start, to start: "<<buffer.seek(0)<<endl;
        buffer.read((char*)&tid,sizeof(tid));
        buffer.read((char*)&ttest,sizeof(ttest));
        cout<<"Res: tid="<<tid<<";ttest="<<ttest<<endl;
    }

    void readOLD(QSharedMemory& sharedMemory) {
        cout<<"reading.."<<endl;

        sharedMemory.lock();

        int size = sharedMemory.size();
        //int len=size/sizeof(Test);
        cout<<"size="<<size<<endl;

        if (size==0) {
            cout <<"No data in shared mem, size=0"<<endl;
            return;
        }

        QBuffer buffer;
        buffer.setData((char*)sharedMemory.data(), sharedMemory.size());
        buffer.open(QBuffer::ReadWrite); //Only);

        id=test=n=0;

        cout<<"buffer pos:"<<buffer.pos()<<endl;
        buffer.seek(0);

        buffer.read((char*)&id,sizeof(id));
        buffer.read((char*)&test,sizeof(test));
        //buffer.read((char*)&n,sizeof(n));
        //buffer.read((char*)arr,n*sizeof(double));

        int i1,i2;
        char* data=(char*)sharedMemory.data();
        i1=*((int*)data++);
        i2=*((int*)data++);

        cout<<"i1="<<i1<<",i2="<<i2<<endl;

        cout<<"reading array.."<<flush;

        //arr = (double*)((char*)sharedMemory.data()+buffer.pos());

        cout<<"ok"<<endl;
        sharedMemory.unlock();

        //cout <<"Result:"<<endl;


    }

    void write(QSharedMemory& sharedMemory) {
        int size1 = sizeof(id)+sizeof(test)+sizeof(n);
        int size2 = DIM2*n*sizeof(double);
        int size = size1+size2;


        if (!sharedMemory.create(size)) {
            cout << "Unable to create shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
            return;
        }


        if (arr==NULL) {
            cout << "ERROR: arr not set"<<endl;
            return;
        }

        sharedMemory.lock();

        /*QBuffer buffer;
        buffer.setData((char*)sharedMemory.data(), size);
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);

        //write item by item
        //out << vec;

        out << id << test << n;

        cout<<"buffer seek:"<<buffer.pos()<<endl;
*/

        int *da = (int*)sharedMemory.data();



        cout << "data address: "<<(qint64)da<<endl;

        da[0] = id;
        da[1] = test;
        da[2] = n;

        double* narrStart = (double*)(&da[3]);
        double** newarr = (double**)(narrStart);
        int p=0;
        for(int j=0;j<DIM2;j++) {
            newarr[j]=(double*)&narrStart[p];
            for(int i=0;i<n;i++,p++) {
                //newarr[j][i]=arr[j][i];
                narrStart[p]=arr[j][i];
            }
            delete [] arr[j];
        }

        delete arr;
        arr=newarr;


//http://www.qtcentre.org/threads/31983-(noob-question)-write-qint64-into-qsharedmemory

        sharedMemory.unlock();

        cout << "read as 1 array: ";
        for(int i=0;i<n*DIM2;i++)
            cout<<narrStart[i]<<" ";
        cout<<endl;

        //TODO: check data
        /*int tid,ttest;
        cout<<"testing from start, to start: "<<buffer.seek(0)<<endl;
        buffer.read((char*)&tid,sizeof(tid));
        buffer.read((char*)&ttest,sizeof(ttest));
        cout<<"Res: tid="<<tid<<";ttest="<<ttest<<endl;*/
    }

    void read(QSharedMemory& sharedMemory) {
        cout<<"reading.."<<endl;

        sharedMemory.lock();

        int size = sharedMemory.size();
        //int len=size/sizeof(Test);
        cout<<"size="<<size<<endl;

        if (size==0) {
            cout <<"No data in shared mem, size=0"<<endl;
            return;
        }

        //sharedMemory.lock();

        int *a = ((int*)sharedMemory.data());

        cout << "data address: "<<(qint64)a<<endl;

        /*for(int i=0;i<3;i++,a++)
            cout << "-> "<<*a<<endl;

        a = ((int*)sharedMemory.data());
        for(int i=0;i<3;i++)
            cout << "-> "<<a[i]++;
        cout <<endl;*/

        id = a[0];
        test = a[1];
        n = a[2];


        if (arr!=NULL) {
            cout<<"deleting arr"<<endl;
            delete arr;
        }

        double* narrStart = (double*)(&a[3]);
        arr = (double**)(narrStart);
        for(int j=0;j<DIM2;j++)
            arr[j]=(double*)&narrStart[j*n];


        sharedMemory.unlock();


        cout << "read as 1 array: ";
        for(int i=0;i<n*DIM2;i++)
            cout<<narrStart[i]<<" ";
        cout<<endl;


        /*QBuffer buffer;
        buffer.setData((char*)sharedMemory.data(), sharedMemory.size());
        buffer.open(QBuffer::ReadWrite); //Only);

        id=test=n=0;

        cout<<"buffer pos:"<<buffer.pos()<<endl;
        buffer.seek(0);

        buffer.read((char*)&id,sizeof(id));
        buffer.read((char*)&test,sizeof(test));
        //buffer.read((char*)&n,sizeof(n));
        //buffer.read((char*)arr,n*sizeof(double));

        int i1,i2;
        char* data=(char*)sharedMemory.data();
        i1=*((int*)data++);
        i2=*((int*)data++);

        cout<<"i1="<<i1<<",i2="<<i2<<endl;

        cout<<"reading array.."<<flush;

        //arr = (double*)((char*)sharedMemory.data()+buffer.pos());

        cout<<"ok"<<endl;
        sharedMemory.unlock();
*/
        //cout <<"Result:"<<endl;
    }








    void write3(QSharedMemory& sharedMemory) {
        cout << "Writing: ";

        cout <<" data pointer: "<<(qint64)sharedMemory.data()<<", const: "<<(qint64)sharedMemory.constData()<<endl;

        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);

        //write item by item
        cout<<"inserting: ";
        for (int i=0; i<10; i++) {
            int x=i*i;
            cout<<" "<<x;
            out << x; //a[i];
        }
        cout<<endl;

        int size = buffer.size();

        if (!sharedMemory.create(size)) {
            cout << "Unable to create shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
            return;
        }
        sharedMemory.lock();
        char *to = (char*)sharedMemory.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(sharedMemory.size(), size));

        cout <<" data pointer: "<<(qint64)sharedMemory.data()<<", const: "<<(qint64)sharedMemory.constData()<<endl;

        sharedMemory.unlock();
    }

    void read3(QSharedMemory& sharedMemory) {
        cout<<"Reading "<<endl;
        sharedMemory.attach();
        QBuffer buffer;
        QDataStream in(&buffer);

        sharedMemory.lock();

        cout <<" data pointer: "<<(qint64)sharedMemory.data()<<", const: "<<(qint64)sharedMemory.constData()<<endl;

        buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
        buffer.open(QBuffer::ReadOnly);

        int len = sharedMemory.size()/sizeof(int);
        int* a = new int[len];

        const int *b = (const int*)sharedMemory.constData();

        for (int i=0; i< len; i++) {
            in >> a[i];
        }

        cout<<"Read b: "<<endl;
        for(int i=0;i<len;i++) cout <<" "<<b[i];
        cout << endl;

        buffer.seek(0);
        cout<<"Read buffer: "<<endl;
        for(int i=0;i<len;i++) {
            int x=-1;
            int nb=buffer.read((char*)&x,sizeof(int));
            cout <<" "<<x<<"["<<nb<<"]";
        }
        cout << endl;

        cout <<" data pointer: "<<(qint64)sharedMemory.data()<<", const: "<<(qint64)sharedMemory.constData()<<endl;
        sharedMemory.unlock();
        cout <<" data pointer: "<<(qint64)sharedMemory.data()<<", const: "<<(qint64)sharedMemory.constData()<<endl;


        cout<<"Read: "<<endl;
        for(int i=0;i<len;i++) cout <<" "<<a[i];
        cout << endl;
        delete a;



    }
};



void writeshmGMap(QSharedMemory& sharedMemory, GMap* gmap) {
    cout << "Writing: "<<endl;

    //int a[5]={1,2,3,4,5};

    /*QVector<int> vec;

    //vec << 1<<2<<3<<4<<5;
    for (int i=0; i<10; i++) {
        int x=i*i;
        vec << x;
    }*/

    /*QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);*/

    //write item by item
    /*out << *gmap; // vec;*/


    /*QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);
    out.*/

    //int size = buffer.size();

    Test* tarr = new Test[2];

    tarr[0].init(3,1,2); //id = 1;
    //tarr[0].test = 11;
    tarr[1].init(20,2,3); //id = 21;
    //tarr[1].test = 211;

    /*for(int i=0; i<2; i++) {
        tarr[i].show();
    }*/

    if (sharedMemory.isAttached()) {
        if (!sharedMemory.detach()) {
            cout << "Unable to detach attached shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
            return;
        } else {
            cout << "Deattached successfully."<<endl;
        }
    }
//TODO same method with gmap

    /*int size=sizeof(Test)*2;

    cout<<"size="<<size<<"; sizeoftarr:"<<sizeof(tarr)<<"; sizoofT: "<<sizeof(tarr[0])<<"; sizeofT1: "<<sizeof(tarr[1])<<endl;
*/

    Test* wt=&tarr[0];

    cout<<"setting:"<<endl;
    wt->show();

    wt->write(sharedMemory);
    cout<<"--done"<<endl;

    /*if (!sharedMemory.create(size)) {
        cout << "Unable to create shared memory segment: "<<sharedMemory.errorString().toStdString()<<endl;
        return;
    }
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    const char *from = (const char*)tarr; //buffer.data().data();
    memcpy(to, from, qMin(sharedMemory.size(), size));

    sharedMemory.unlock();*/


    cout<<"Writing done"<<endl;


    for(int i=0;i<100;i++) {
        wt->show();
        //test.inc();
        sleep(1);
    }


    /*for (int x : vec) cout <<" "<<x;
    cout <<endl;*/
}

GMap* readshmGmap(QSharedMemory& sharedMemory) {
    cout<<"Reading "<<endl;
    if (!sharedMemory.attach()) {
        cout <<"Failed to attach"<<endl;
        return NULL;
    }


    /*QBuffer buffer;
    QDataStream in(&buffer);*/

    //sharedMemory.lock();
    /*buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);*/

    /*int len = sharedMemory.size()/sizeof(int);
    QVector<int> vec;

    in>>vec;*/

    /*cout << "casting data:"<<flush;
    const QVector<int>* vec = (const QVector<int>*)sharedMemory.constData();
    cout<<"ok"<<endl<<"data:"<<endl;

    int i=(int)(id-'0');
    cout <<"data at index "<<i<<": "<<(*vec)[i]<<endl;
*/


 /*   int size = sharedMemory.size();
    int len=size/sizeof(Test);
    cout<<"size="<<size<<", len="<<len<<endl;

    const Test* tarr = (const Test*)sharedMemory.constData();

    sharedMemory.unlock();

    cout <<"Reading:"<<endl;
    for(int i=0;i<len;i++,tarr++)
        tarr->show();
*/

    Test test;

    test.read(sharedMemory);

    cout<<"result:"<<endl;

    for(int i=0;i<10;i++) {
        test.show();
        test.inc();
        sleep(1);
    }




/*
    cout<<"Read: "<<endl;
    for (int x : vec) cout <<" "<<x;
    cout << endl;*/

    return NULL;

}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

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
    QSharedMemory shMem("test3");


    //GMapWidget* gmapWidget = NULL;

    cout << "Hide & Seek MemServer"<<endl;
    try {
        /*bool useMouseObs = false;
        bool allowClickNextPos = false;
        int width = 0;
        int height = 0;
        GMap* map = NULL;*/

        if (argc>2) { // && !useMouseObs) {
            cout << "Opening map: "<<argv[1]<<endl;


            GMap* gmap = NULL;

            //GMap gmap(argv[1]);
            //QString mapFStr = QString::fromLatin1(argv[1]);

            gmap = new GMap(argv[1],NULL);


            cout << "Map size: "<<gmap->rowCount()<<"x"<<gmap->colCount()<<endl;
            cout << "Obstacles: "<<gmap->numObstacles()<<", Free Cells: "<<gmap->numFreeCells()<<endl;
            if (gmap->colCount()<=50 && gmap->rowCount()<=50) {
                gmap->printMap();
            }

            cout << "ok"<<endl;
            //gmapWidget = new GMapWidget(gmap);

            cout << "Loading dist map: "<<flush;
            string dmatF = argv[2];
            gmap->readDistanceMatrixToFile(dmatF);

            //gmap->printMap();
            writeshmGMap(shMem,gmap);
            //gmap->writeToSharedMemory(shMem);

            //todo delete gmap

            return a.exec();
        } else {

            //cout <<"MemServer requieres two parameters: map-file.txt map-dist.dmat.txt"<<endl;

            GMap* gmap = readshmGmap(shMem);
            /*GMap gmap;
            gmap.readFromSharedMemory(shMem);*/

            if (gmap!=NULL)  {
                cout << "Map size: "<<gmap->rowCount()<<"x"<<gmap->colCount()<<endl;
                cout << "Obstacles: "<<gmap->numObstacles()<<", Free Cells: "<<gmap->numFreeCells()<<endl;
                if (gmap->colCount()<=50 && gmap->rowCount()<=50) {
                    gmap->printMap();
                }
            } else {
                cout <<"Gmap is null"<<endl;
            }
        }


    } catch(exception &e) {
        cout << "Exception: " << e.what() << endl ;
    }

    return 0;
}
