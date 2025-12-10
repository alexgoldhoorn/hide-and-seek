
#include "Utils/generic.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>

#include <sys/stat.h>

#if __cplusplus>=201103
#include <random>
#endif


using namespace std;



char* charConstToCharArr(const char* chrs) {
    char* res = new char[strlen(chrs)+1];
    strcpy(res,chrs);
    return res;
}


char convertTextToNumChar(char c) {
    if (c == ' ' || c == '\t') {
          return 0;
    } else {
          return c  - '0';
    }
}


string currentTimeStamp() {
    char buf[256];
    struct tm * timeinfo;
    time_t tt;
    time(&tt);
    timeinfo = localtime(&tt);

    //http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    strftime(buf, 256,"%Y/%m/%d %H:%M:%S", timeinfo);
    //strcpy(buf,ctime(&tt));
    //buf[strlen(buf)-1]='\0';

    string timeStampStr(buf);
    return timeStampStr;
}


bool fileExists(const char *filename)
{
    struct stat buf;
    if (stat(filename, &buf) != -1 && S_ISREG(buf.st_mode)) //exists and is a file
    {
        return true;
    }
    return false;
}

#if __cplusplus>=201103 //new C++, new random

std::random_device __randomDevice;
std::mt19937 __randomGenerator(__randomDevice());
std::uniform_real_distribution<> __uniformRealDistr(0,1);
std::uniform_int_distribution<> __uniformIntDistr(0,100);

void initRandomizer() {
    //TODO: not necessary (if linux..)
    //srand (time(NULL));//TODO more random (use ms/ns or /dev/random)
}

int random(int max)
{
    decltype(__uniformIntDistr.param()) new_range (0, max);
    __uniformIntDistr.param(new_range);
    return __uniformIntDistr(__randomGenerator);
}

int random(int min, int max)
{
    decltype(__uniformIntDistr.param()) new_range (min, max);
    __uniformIntDistr.param(new_range);
    //printf("random(%d,%d), min=%d,max=%d\n",min,max,__uniformIntDistr.min(),__uniformIntDistr.max());
    return __uniformIntDistr(__randomGenerator);
}

double randomDouble(double min, double max)
{
    decltype(__uniformRealDistr.param()) new_range (min, max);
    __uniformRealDistr.param(new_range);
    return __uniformRealDistr(__randomGenerator);
}


#else //old random

void initRandomizer() {
    printf("WARNING: using old version of random functions, use C++11 (g++ >=4.8 and -std=c++11)");
                //cout << "WARNING: using old version of random functions, use C++11 (g++ >=4.8 and -std=c++11)"<<endl;
    srand (time(NULL));//TODO more random (use ms/ns or /dev/random)
}

//AG140102: changed max -> max+1, since in the C++ version, max DOES contain the maximum value (which is also applied by the name 'max')
int random(int max)
{
    return rand() % (max+1);
}

int random(int min, int max)
{
    return rand() % (max+1 - min) + min;
}

double randomDouble(double min, double max)
{
    return (double) rand() / RAND_MAX * (max - min) + min;
}
#endif

#ifdef USE_QT
bool qstringToBool(QString str, bool* ok) {
    bool value = false;
    if (str.compare("true",Qt::CaseInsensitive)==0 || str.compare("yes",Qt::CaseInsensitive)==0 ||
            str.compare("1",Qt::CaseInsensitive)==0) {

        if (ok!=NULL) *ok = true;
        value = true;
    } else if (str.compare("false",Qt::CaseInsensitive)==0 || str.compare("no",Qt::CaseInsensitive)==0 ||
               str.compare("0",Qt::CaseInsensitive)==0) {

       if (ok!=NULL) *ok = true;
       value = false;
    } else {
       if (ok!=NULL) *ok = false;
    }

    return value;
}

QString argsToQString(int argc, char *argv[]) {
    QString argStr;
    for(int i=0; i<argc; i++) {
        if (i>0) argStr += " ";
        QString s = QString::fromLatin1(argv[i]);
        if (s.contains(" ")) {
            argStr += '"'+s+'"';
        } else {
            argStr += s;
        }

        //cout <<" * "<<i<<": '"<<argv[i]<<"'"<<endl;
    }

    return argStr;
}

#endif

double sqrd(double x) {
    return x*x;
}

