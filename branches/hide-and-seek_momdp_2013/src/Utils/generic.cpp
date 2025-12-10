
#include "generic.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>

#include <sys/stat.h>


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

/*bool fileExists(const char *file) {
    ifstream tFile(file);
    bool ok = (tFile
}*/

bool fileExists(const char *filename)
{
    struct stat buf;
    if (stat(filename, &buf) != -1 && S_ISREG(buf.st_mode)) //exists and is a file
    {
        return true;
    }
    return false;
}

void initRandomizer() {
    srand (time(NULL));//TODO more random (use ms/ns or /dev/random)
}


int random(int max)
{
    return rand() % max;
}

int random(int min, int max)
{
    return rand() % (max - min) + min;
}

double randomDouble(double min, double max)
{
    return (double) rand() / RAND_MAX * (max - min) + min;
}
