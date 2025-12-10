#ifndef BPOS
#define BPOS

#include "HSGame/pos.h"

/*!
 * \brief The BPos struct has a Pos and a belief. Note that this class has greater and smaller than comparison functions
 * only based on the belief value, which is such that we can use it for ordening.
 */
struct BPos : public Pos {
    BPos(int row_,int col_, double b=-1);
    BPos(unsigned int row_, unsigned int col_, double b=-1); //AG150122
    BPos(double row_,double col_, double b=-1);
    BPos(const Pos& p, double b=-1);
    BPos(const BPos& p);

    BPos();

    virtual ~BPos();

    virtual std::string toString() const;

#ifdef USE_QT
    //AG131211: functions to read/write map from stream in GMap to make them equal for all

    virtual void readPosFromStream(QDataStream& in, bool useDouble);

    virtual void writePostoStream(QDataStream& out, bool useDouble) const;
#endif

    virtual void clear();

    //! comparas only the belief, not the position
    bool operator <(const BPos& other) const;

    //! comparas only the belief, not the position
    bool operator >(const BPos& other) const;

    virtual BPos& operator = (const BPos& pos);

    virtual BPos& operator = (const Pos& pos);

    //! compares only the position, not the belief
    virtual bool operator ==(const BPos& other) const;

    //! compares only the position, not the belief
    virtual bool operator !=(const BPos& other) const;

    virtual bool operator ==(const Pos& other) const;

    virtual bool operator !=(const Pos& other) const;

    //belief of the pos
    double b;
};

#endif // BPOS

