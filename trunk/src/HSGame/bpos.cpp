#include "HSGame/bpos.h"
#include <sstream>
//#include <iostream>

using namespace std;


BPos::BPos(int row_,int col_, double b) : Pos(row_, col_) {
    this->b = b;
}

BPos::BPos(unsigned int row_, unsigned int col_, double b) : Pos(row_,col_) {
    this->b = b;
}

BPos::BPos(double row_,double col_, double b) : Pos(row_,col_) {
    this->b = b;
}

BPos::BPos(const Pos& p, double b) : Pos(p) {
    this->b = b;
}

BPos::BPos(const BPos& p) : Pos(p){
    this->b = p.b;
}

BPos::BPos() : Pos() {
    this->b = -1;
}

BPos::~BPos() {
}


bool BPos::operator <(const BPos& other) const {
    return b<other.b;
}

bool BPos::operator >(const BPos& other) const {
    return b>other.b;
}

BPos& BPos::operator = (const BPos& pos) {    
     Pos::operator =(pos);
     b = pos.b;     
     return *this;
}

BPos& BPos::operator = (const Pos& pos) {
     Pos::operator =(pos);
     return *this;
}

bool BPos::operator ==(const Pos& pos) const  {
    return Pos::operator ==(pos);
}
bool BPos::operator !=(const Pos& pos) const  {
    return Pos::operator !=(pos);
}
bool BPos::operator ==(const BPos& pos) const  {
    return Pos::operator ==(pos);
}
bool BPos::operator !=(const BPos& pos) const {
    return Pos::operator !=(pos);
}

std::string BPos::toString() const {
    std::stringstream sstr;
    sstr << Pos::toString() << "(b="<<b<<")";
    return sstr.str();
}

#ifdef USE_QT
void BPos::readPosFromStream(QDataStream& in, bool useDouble) {
    Pos::readPosFromStream(in, useDouble);
    in >> b;
}

void BPos::writePostoStream(QDataStream& out, bool useDouble) const {
    Pos::writePostoStream(out, useDouble);
    out << b;
}
#endif

void BPos::clear() {
    Pos::clear();
    b = -1;
}
