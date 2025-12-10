#include "HSGame/idpos.h"

#include <sstream>

using namespace std;

IDPos::IDPos(int row_,int col_, int id) : Pos(row_,col_), _id(id) {
}

IDPos::IDPos(unsigned int row_, unsigned int col_, int id) : Pos(row_,col_), _id(id) {
}

IDPos::IDPos(double row_,double col_, int id) : Pos(row_, col_), _id(id) {
}

IDPos::IDPos(const IDPos& p) : Pos(p){
    //operator =(p);
    _id = p._id;
}

IDPos::IDPos(const Pos &p, int id) : Pos(p), _id(id) {
}

IDPos::IDPos(const std::vector<double>& vec, int id) : Pos(vec), _id(id) {
}

IDPos::IDPos(int id) : Pos(), _id(id) {
}

IDPos::IDPos() : Pos(), _id(-1) {
}

#ifdef USE_QT
IDPos::IDPos(const QPoint& p, int id) : Pos(p), _id(id) {
}
#endif

IDPos::~IDPos() {
}

std::string IDPos::toString() const {
    stringstream ss;
    ss << "<"<<_id<<">"<<Pos::toString();
    return ss.str();
}

bool IDPos::equalsWithID(const IDPos& p) const {
    return _id==p._id && Pos::equals(p);
}

bool IDPos::equalsWithID(double row, double col, int id) const {
    return _id==id && Pos::equals(row,col);
}

bool IDPos::equalsIntWithID(const IDPos& p) const {
    return _id==p._id && Pos::equalsInt(p);
}

bool IDPos::equalsIntWithID(int row, int col, int id) const {
    return _id==id && Pos::equalsInt(row,col);
}


IDPos& IDPos::operator = (const IDPos& pos) {
     Pos::operator =(pos);
     _id = pos._id;
     return *this;
}

IDPos& IDPos::operator = (const Pos& pos) {
     Pos::operator =(pos);
     return *this;
}

void IDPos::set(int row_, int col_, int id) {
    Pos::set(row_,col_);
    _id = id;
}

void IDPos::set(unsigned int row_, unsigned int col_, int id) {
    Pos::set(row_,col_);
    _id = id;
}

void IDPos::set(double row_, double col_, int id) {
    Pos::set(row_,col_);
    _id = id;
}

void IDPos::set(const Pos &pos) {
    Pos::operator =(pos);
}

int IDPos::id() const {
    return _id;
}

#ifdef USE_QT
void IDPos::readPosFromStream(QDataStream& in, bool useDouble) {
    Pos::readPosFromStream(in,useDouble);
    qint16 id;
    in >> id;
    _id = id;
}

void IDPos::writePostoStream(QDataStream& out, bool useDouble) {
    Pos::writePostoStream(out,useDouble);
    out << (qint16)_id;
}

#endif
