#ifndef VERYSMARTHIDER_H
#define VERYSMARTHIDER_H

#include "smarthider.h"


class VerySmartHider : public SmartHider
{
public:
    static const char SCORE_AVG = 0;
    static const char SCORE_MIN = 1;
    static const char SCORE_MAX = 2;

    VerySmartHider(bool allKnowing = false, char scoreType = SCORE_AVG);


    virtual int getHiderType() {
        return _allKnowing ? HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING : HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART;
    }

protected:
    virtual bool scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double & score);

    virtual double scoreForState(int sr, int sc, int hr, int hc);

    char _scoreType;

};

#endif // VERYSMARTHIDER_H
