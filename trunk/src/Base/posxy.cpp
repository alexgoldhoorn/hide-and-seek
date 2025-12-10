#include "Base/posxy.h"
#include <cmath>
#include <sstream>

using namespace std;


PosXY::PosXY() {
    x=y=0;
    orientation=0;
    b=-1;
    id=-1;
}

PosXY::PosXY(double x, double y, int id, double orientation, double b) {
    this->x = x;
    this->y = y;
    this->orientation = orientation;
    this->id = id;
    this->b = b;
}

PosXY::PosXY(const Pos &pos, SeekerHSParams *params) {
    fromPos(pos, params);
}

PosXY::PosXY(const IDPos &pos, SeekerHSParams *params) {
    fromPos(pos, params);
}

bool PosXY::isSet() const {
    return x>=0 && y>=0; //x!=-1 && y!=-1; //AG150804: changed because in ROS after transforming and sending coordinates the received
			//value can be -0.9999.. and thus not -1
}

void PosXY::clear() {
    x = y = -1;
}

void PosXY::fromPos(const Pos &pos, SeekerHSParams *params, double orientation, double b) {
    if (params->useContinuousPos) {
        x = pos.rowDouble() * params->cellSizeM;
        y = pos.colDouble() * params->cellSizeM;
    } else {
        double halfCellSize = params->cellSizeM/2.0;
        x = pos.row() * params->cellSizeM + halfCellSize;
        y = pos.col() * params->cellSizeM + halfCellSize;
    }

    this->orientation = orientation;
    this->b = b;
}

void PosXY::fromIDPos(const IDPos &pos, SeekerHSParams *params, double orientation, double b) {
    if (params->useContinuousPos) {
        x = pos.rowDouble() * params->cellSizeM;
        y = pos.colDouble() * params->cellSizeM;
    } else {
        double halfCellSize = params->cellSizeM/2.0;
        x = pos.row() * params->cellSizeM + halfCellSize;
        y = pos.col() * params->cellSizeM + halfCellSize;
    }

    this->id = pos.id();
    this->orientation = orientation;
    this->b = b;
}

void PosXY::fromBPos(const BPos &pos, SeekerHSParams *params, double orientation) {
    if (params->useContinuousPos) {
        x = pos.rowDouble() * params->cellSizeM;
        y = pos.colDouble() * params->cellSizeM;
    } else {
        double halfCellSize = params->cellSizeM/2.0;
        x = pos.row() * params->cellSizeM + halfCellSize;
        y = pos.col() * params->cellSizeM + halfCellSize;
    }

    //this->id = pos.id();
    this->orientation = orientation;
    this->b = pos.b;
}

string PosXY::toString() const {
	stringstream sstr;
	sstr <<"("<<id<<"]"<<x<<","<<y<<","<< round(1800.0*orientation/M_PI)/10.0<<"deg;b="<<b<<")";
	return sstr.str();
}

Pos PosXY::toPos(SeekerHSParams *params) const {
    Pos outPos;

    if (isSet()) {
        if (params->useContinuousPos) {
            outPos.set( /*_map->rowCount() -*/ x / params->cellSizeM,
                                            /*_map->colCount() -*/ y / params->cellSizeM );
        } else {
            outPos.set( /*_map->rowCount() -*/ (int)floor(x / params->cellSizeM),
                                            /*_map->colCount() -*/ (int)floor(y / params->cellSizeM) );
        }
    }

    return outPos;
}

IDPos PosXY::toIDPos(SeekerHSParams *params) const {
    IDPos outPos;

    if (isSet()) {
        if (params->useContinuousPos) {
            outPos.set( /*_map->rowCount() -*/ x / params->cellSizeM,
                                            /*_map->colCount() -*/ y / params->cellSizeM, id );
        } else {
            outPos.set( /*_map->rowCount() -*/ (int)floor(x / params->cellSizeM),
                                            /*_map->colCount() -*/ (int)floor(y / params->cellSizeM), id );
        }
    }

    return outPos;
}

BPos PosXY::toBPos(SeekerHSParams *params) const {
    BPos outPos;

    if (isSet()) {
        if (params->useContinuousPos) {
            outPos.set( /*_map->rowCount() -*/ x / params->cellSizeM,
                                            /*_map->colCount() -*/ y / params->cellSizeM );
        } else {
            outPos.set( /*_map->rowCount() -*/ (int)floor(x / params->cellSizeM),
                                            /*_map->colCount() -*/ (int)floor(y / params->cellSizeM) );
        }
        outPos.b = b;
    }

    return outPos;
}

