#include "HSGame/pos.h"

#include <sstream>
#include <cmath>
#include <limits>
#include <cmath>

//#include <iostream>

//AG140527: WARNING!! THIS "using namespace std" is required! When removing this, the function std::numeric_limits<double> (used in equals*)
//          does NOT work correctly and causes the 'equals' functions to see differences of 0.1 to be 'equal'
using namespace std;

Pos::Pos(int row_,int col_) {
    set(row_,col_);
}
Pos::Pos(unsigned int row_, unsigned int col_) {
    set((int)row_, (int)col_);
}
Pos::Pos(double row_,double col_) {
    set(row_,col_);
}
Pos::Pos(const Pos& p) {
    operator =(p);
}

Pos::Pos(const std::vector<double>& vec) {
    set(vec[0],vec[1]);
}

Pos::Pos() {
    clear();
}

#ifdef USE_QT
Pos::Pos(const QPoint& p) {
    set(p.y(), p.x());
}
#endif

Pos::~Pos() {
}

int Pos::row() const {
    return _row;
}

int Pos::col() const {
    return _col;
}

double Pos::rowDouble() const {
    return _rowDouble;
}
double Pos::colDouble() const {
    return _colDouble;
}

std::string Pos::toString() const {
    std::stringstream sstr;
    //sstr << "(" << row << "," << col << ")";
    if (!isSet()) {
        sstr << "[not set]";
    } else if (_useDouble) {
        sstr << "r"<<_rowDouble<<"c"<<_colDouble;
        sstr << "[r"<<_row<<"c"<<_col<<"]";
    } else {
        sstr << "r"<<_row<<"c"<<_col;
    }
    return sstr.str();
}

/*void Pos::set(const Pos& p) {
    _row = p._row;
    _col = p._col;
    _rowDouble = p._rowDouble;
    _colDouble = p._colDouble;
    _useDouble = p._useDouble;
}*/

void Pos::set(int row_, int col_) {
    _row = row_;
    _col = col_;
    _rowDouble = row_;
    _colDouble = col_;
    _useDouble = false;
}

void Pos::set(unsigned int row_, unsigned int col_) {
    _row = (int)row_;
    _col = (int)col_;
    _rowDouble = row_;
    _colDouble = col_;
    _useDouble = false;
}

void Pos::set(double row_, double col_) {
    //ints have to be floored, e.g. 0.1 would be 0, since it is between 0-1
    _row = (int)floor(row_);
    _col = (int)floor(col_);
    //doubles we can copy
    _rowDouble = row_;
    _colDouble = col_;
    //use double if the int and double values are different
    _useDouble = (_rowDouble!=_row || _colDouble!=_col);
}

bool Pos::isSet() const  {
    return (_useDouble || (_row!=-1 && _col!=-1));
}

Pos& Pos::operator =(const Pos& p) {
    if (this != &p) {
        //std::cout << " >>pos.operator=: "<<toString()<<"="<<p.toString()<<endl;
        _row = p._row;
        _col = p._col;
        _rowDouble = p._rowDouble;
        _colDouble = p._colDouble;
        _useDouble = p._useDouble;
    }
    return *this;
}

void Pos::add(double ar, double ac) {
    _rowDouble += ar;
    _colDouble += ac;

    _row = (int)floor(_rowDouble);
    _col = (int)floor(_colDouble);

    _useDouble = (_rowDouble!=_row || _colDouble!=_col);
}

void Pos::add(int ar, int ac) {
    _rowDouble += ar;
    _colDouble += ac;
    _row += ar;
    _col += ac;
}

void Pos::clear() {
    _row = _col = -1;
    _rowDouble = _colDouble = -1;
    _useDouble = false;
}

bool Pos::operator <(const Pos& other) const {
    if (_useDouble) {
        return ( _rowDouble < other._rowDouble ||
                    ( abs(_rowDouble - other._rowDouble) < std::numeric_limits<double>::epsilon()
                      && _colDouble < other._colDouble) );
    } else {
        return (_row<other._row || (_row==other._row && _col<other._col));
    }
}

bool Pos::operator >(const Pos& other) const {
    if (_useDouble) {
        return ( _rowDouble > other._rowDouble ||
                 ( abs(_rowDouble - other._rowDouble) < std::numeric_limits<double>::epsilon()
                   && _colDouble > other._colDouble));
    } else {
        return (_row>other._row || (_row==other._row && _col>other._col));
    }
}

bool Pos::operator ==(const Pos& p) const {
    if (_useDouble || p._useDouble) {
        //ag140130: compare taking into account precision errors
        return abs(_rowDouble - p._rowDouble) < std::numeric_limits<double>::epsilon()
                && abs(_colDouble - p._colDouble) < std::numeric_limits<double>::epsilon();
    } else {
        return _row==p._row && _col==p._col;
    }
}

bool Pos::operator !=(const Pos& p) const {
    if (_useDouble || p._useDouble) {
        //ag140130: compare taking into account precision errors
        return abs(_rowDouble - p._rowDouble) > std::numeric_limits<double>::epsilon()
                || abs(_colDouble - p._colDouble) > std::numeric_limits<double>::epsilon();
    } else {
        return _row!=p._row || _col!=p._col;
    }
}

bool Pos::equals(const Pos& p) const {
    if (_useDouble || p._useDouble) {
        //ag140130: compare taking into account precision errors
        return abs(_rowDouble - p._rowDouble) < std::numeric_limits<double>::epsilon()
                && abs(_colDouble - p._colDouble) < std::numeric_limits<double>::epsilon();
    } else {
        return _row==p._row && _col==p._col;
    }
}

bool Pos::equals(double rw, double cl) const {
    //ag140130: compare taking into account precision errors

    //std::cout<< "Pos::equals: abs(_rowDouble - rw)="<<abs(_rowDouble - rw)<<"; abs(_colDouble - cl)="<<abs(_colDouble - cl)<<", eps="<<std::numeric_limits<double>::epsilon()<<std::endl;

    return abs(_rowDouble - rw) < std::numeric_limits<double>::epsilon()
            && abs(_colDouble - cl) < std::numeric_limits<double>::epsilon();
}

bool Pos::equalsInt(const Pos& p) const {
    return p._row==_row && p._col==_col;
}

bool Pos::equalsInt(int rw, int cl) const {
    return rw==_row && cl==_col;
}


std::string Pos::getHash() const {
    std::stringstream ss;
    if (_useDouble) {
        ss << _rowDouble << "," << _colDouble;
    } else {
        ss << _row << "," << _col;
    }

    /*ss << setw(5) << setfill(' ') << row;
    ss << ',';
    ss << setw(5) << setfill(' ') << col;*/

    return ss.str();
}

void Pos::convertValuesToInt() {
    if (_useDouble) {
        _useDouble = false;
        _rowDouble = _row;
        _colDouble = _col;
    }
}

 /*void Pos::toVector(std::vector<double>& vec) {
    //std::vector<double> vec;
    vec.push_back(_rowDouble);
    vec.push_back(_colDouble);
    //return vec;
}*/

std::vector<double> Pos::toVector() {
    std::vector<double> vec;
    vec.push_back(_rowDouble);
    vec.push_back(_colDouble);
    return vec;
}

bool Pos::hasDouble() const {
    return _useDouble;
}

double Pos::distanceEuc(const Pos &p2) const {
    return sqrt(pow(_rowDouble-p2._rowDouble, 2) + pow(_colDouble-p2._colDouble, 2));
}

#ifdef USE_QT
void Pos::readPosFromStream(QDataStream &in, bool useDouble) {
    if (useDouble) {
        in >> _rowDouble;
        in >> _colDouble;
        set(_rowDouble, _colDouble);
    } else {
        in >> _row;
        in >> _col;
        set(_row, _col);
    }
}

void Pos::writePostoStream(QDataStream &out, bool useDouble) const {
    if (useDouble) {
        out <<_rowDouble;
        out <<_colDouble;
    } else {
        out <<_row;
        out <<_col;
    }
}
#endif

