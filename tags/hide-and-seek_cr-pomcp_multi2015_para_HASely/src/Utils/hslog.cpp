#include "Utils/hslog.h"

#include "Utils/generic.h"

#include <ctime>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

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
    _ostream = stream;
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

void HSLog::print(unsigned long v){
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

void HSLog::print(const Pos& p){
    printTime();
    if (p.hasDouble()) {
        (*_ostream)<<","<<p.rowDouble()<<","<<p.colDouble();
    } else {
        (*_ostream)<<","<<p.row()<<","<<p.col();
    }
}

void HSLog::print(const BPos& p){
    printTime();
    print((const Pos&)p);
    (*_ostream)<<","<<p.b;
}

void HSLog::print(const PosXY& p, bool logOrient, bool logBelief){
    printTime();
    (*_ostream)<<","<<p.x<<","<<p.y;
    if (logOrient) (*_ostream)<<","<<p.orientation;
    if (logBelief) (*_ostream)<<","<<p.b;
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

void HSLog::printLine(unsigned long v){
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

void HSLog::printLine(const Pos& p){
    print(p);
    _newLine=true;
}

void HSLog::printLine(const BPos& p){
    print(p);
    _newLine=true;
}

void HSLog::printLine(const PosXY& p, bool logOrient, bool logBelief){
    print(p, logOrient, logBelief);
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


    struct timeval tv;
    time_t curtime;
    char buf[256];
    char buf1[256];

    gettimeofday(&tv, NULL);
    curtime=tv.tv_sec;

    //http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    strftime(buf1, 256,"%Y/%m/%d %T.", localtime(&curtime)); //"%Y/%m/%d %H:%M:%S"
    sprintf(buf,"%s%ld",buf1,tv.tv_usec);
    //printf("%s%ld\n",buf1,tv.tv_usec);

    //cout<<"time: '"<<buf<<"'"<<endl;

    time_t tt;
    time(&tt);

    /*char buf[256];
    struct tm * timeinfo;
    time_t tt;
    time(&tt);
    timeinfo = localtime(&tt);

    //http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    strftime(buf, 256,"%Y/%m/%d %H:%M:%S", timeinfo);*/

    (*_ostream) << "[" << buf  << "],"<<tt;
}





ofstream* HSLog::getOFStream() {
    return &_ofstream;
}

ostream* HSLog::getOStream() {
    return _ostream;
}
