    #ifndef HSLOG_H
#define HSLOG_H

#include <iostream>
#include <fstream>
#include "HSGame/gmap.h"
#include "HSGame/bpos.h"
#include "Base/posxy.h"

using namespace std;

//! H&S Log class
/*! H&S Logger class. It stores information in a CSV way. [todo: allow to disable?]
  */
class HSLog { //todo: try to implement << and check when there is an endl at the end -> say _newLine=true
public:
    //! Start log with filename.
    HSLog(const char *filename, bool append=true, bool startTime=true);
    //! Start log with stream.
    HSLog(ostream* stream, bool startTime=true);

    ~HSLog();

    //! Close file stream
    void close();


    void print(string v);
    void print(int v);
    void print(long v);
    void print(unsigned long v);
    void print(float v);
    void print(double v);
    void print(const Pos& p);
    void print(const BPos& p);
    void print(const PosXY& p, bool logOrient, bool logBelief);


    void printLine(string v);
    void printLine(int v);
    void printLine(long v);
    void printLine(unsigned long v);
    void printLine(float v);
    void printLine(double v);
    void printLine(const Pos& p);
    void printLine(const BPos& p);
    void printLine(const PosXY& p, bool logOrient, bool logBelief);

    //AG121011: print line without time
    void printLineNoTime(string s);


    ofstream* getOFStream();

    ostream* getOStream();

private:
    //stream for output
    ostream* _ostream;
    //file stream
    ofstream _ofstream;
    //new line (to check if start with time)
    bool _newLine;
    //start time
    bool _startTime;

    void printTime();

};


#endif // HSLOG_H
