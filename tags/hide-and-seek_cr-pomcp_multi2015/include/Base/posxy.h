#ifndef POSXY
#define POSXY

#include "HSGame/idpos.h"
#include "HSGame/bpos.h"
#include "seekerhsparams.h"

#include <string>

//AG150618: added to remove vectors of doubles
/*!
 * \brief The PosXY struct contains the (x,y) location as used by the robot. The b represents the belief.
 */
struct PosXY {
    //! create empty PosXY
    PosXY();

    //! create an PosXY
    PosXY(double x, double y, int id=-1, double orientation=0, double b=0);

    //! create form Pos
    PosXY(const Pos& pos, SeekerHSParams* params);

    //! create form IDPos
    PosXY(const IDPos& pos, SeekerHSParams* params);

    //! read from Pos
    void fromPos(const Pos& pos, SeekerHSParams* params, double orientation=0, double b=-1);

    //! read from IDPos
    void fromIDPos(const IDPos& pos, SeekerHSParams* params, double orientation=0, double b=-1);

    //! read from IDPos
    void fromBPos(const BPos& pos, SeekerHSParams* params, double orientation=0);

    //! convert to Pos
    Pos toPos(SeekerHSParams* params) const;

    //! convert to IDPos
    IDPos toIDPos(SeekerHSParams* params) const;

    //! convert toBPos
    BPos toBPos(SeekerHSParams* params) const;

    //! clear all vars
    void clear();

    //! is set (checks if x and y are -1)
    bool isSet() const;
 
    //! this position as a string
    std::string toString() const;

    //! x location
    double x;
    //! y location
    double y;
    //! orientation
    double orientation;
    //! belief
    double b;
    //! id
    int id;

protected:
    /*static const int ROW_INDEX = 0; //1; was switched (1) for experimetn 140124
    static const int COL_INDEX = 1; //0; */
};

#endif // POSXY

