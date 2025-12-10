#include "hslog.h"

#include "generic.h"

#include <ctime>


HSLog::HSLog(const char *filename, bool append, bool startTime) {
    if (append) {
        _ofstream.open(filename,ios_base::app);
    } else {
        _ofstream.open(filename);
    }
    _ostream = dynamic_cast<ostream*>(&_ofstream);
    _startTime = startTime;
    _newLine = true;
}

HSLog::HSLog(ostream* stream, bool startTime) {
    _startTime = startTime;
    _newLine = true;
}

HSLog::~HSLog() {
    //delete+close
    close();
}

void HSLog::close(){
    if (_ofstream.is_open()) {
        _ofstream.close();
    }

    _ostream = NULL;
}


void HSLog::print(string v){
    printTime();
    (*_ostream)<<","<<v;
}

void HSLog::print(int v){
    printTime();
    (*_ostream)<<","<<v;
}

void HSLog::print(long v){
    printTime();
    (*_ostream)<<","<<v;
}

void HSLog::print(float v){
    printTime();
    (*_ostream)<<","<<v;
}

void HSLog::print(double v){
    printTime();
    (*_ostream)<<","<<v;
}

void HSLog::print(Pos p){
    printTime();
    (*_ostream)<<","<<p.row<<","<<p.col;
}

void HSLog::printLine(string v){
    printTime();
    (*_ostream)<<","<<v<<endl;
    _newLine=true;
}

void HSLog::printLine(int v){
    printTime();
    (*_ostream)<<","<<v<<endl;
    _newLine=true;
}

void HSLog::printLine(long v){
    printTime();
    (*_ostream)<<","<<v<<endl;
    _newLine=true;
}

void HSLog::printLine(float v){
    printTime();
    (*_ostream)<<","<<v<<endl;
    _newLine=true;
}

void HSLog::printLine(double v){
    printTime();
    (*_ostream)<<","<<v<<endl;
    _newLine=true;
}

void HSLog::printLine(Pos p){
    printTime();
    (*_ostream)<<","<<p.row<<","<<p.col<<endl;
    _newLine=true;
}

void HSLog::printLineNoTime(string s) {
    if (!_newLine) (*_ostream)<<",";
    (*_ostream)<<s<<endl;
}

/*
basic_ostream& HSLog::operator<< (int& v ) {
    printTime();
    (*_ostream)<<","<<v;
}

//basic_ostream& operator<< (unsigned int& val );
basic_ostream& HSLog::operator<< (long& v ){
    printTime();
    (*_ostream)<<","<<v;
}
//basic_ostream& operator<< (unsigned long& val );
basic_ostream& HSLog::operator<< (float& v ){
    printTime();
    (*_ostream)<<","<<v;
}
basic_ostream& HSLog::operator<< (double& v ){
    printTime();
    (*_ostream)<<","<<v;
}*/


void HSLog::printTime() {
    if (_newLine) {
        _newLine = false;
    } else {
        return; //do not print time
    }

    char buf[256];
    struct tm * timeinfo;
    time_t tt;
    time(&tt);
    timeinfo = localtime(&tt);

    //http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    strftime(buf, 256,"%Y/%m/%d %H:%M:%S", timeinfo);
    //strcpy(buf,ctime(&tt));
    //buf[strlen(buf)-1]='\0';

    (*_ostream) << "[" << buf  << "],"<<tt;
}



/*


//todo: timer to new timer class (with multiple timers)
void HSMOMDP::restartTimer() {
    _timerStart = time(NULL);
}

int HSMOMDP::stopTimer() {
    return (int)(time(NULL)-_timerStart);
}
*/
