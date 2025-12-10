#ifndef VERYSMARTHIDER_H
#define VERYSMARTHIDER_H

#include "AutoHider/smarthider.h"

/*!
 * \brief The VerySmartHider class uses the same score as the SmartHider, but the calculation of the score of each of the actions
 * is done as an average for each of the possible (next) positions of the seeker, including if the seeker is not visible to the hider.
 */
class VerySmartHider : public SmartHider
{
public:
    static const char SCORE_AVG = 0;
    static const char SCORE_MIN = 1;
    static const char SCORE_MAX = 2;

    VerySmartHider(SeekerHSParams* params, bool allKnowing = false, char scoreType = SCORE_AVG);

    virtual ~VerySmartHider();

    virtual int getHiderType() const;

    virtual std::string getName() const;

protected:
    //virtual bool scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double & score);
    virtual bool scoreForAction(const Pos& seekerPos, const Pos& hiderPos, int a, double & score);

    //AG131106: added hrp, hcp (previous), to detect
    //! score for a state, taking into account movement of the oponent
    //virtual double scoreForState(int sr, int sc, int hr, int hc, int hrp, int hcp);
    virtual double scoreForState(const Pos& seekerPos, const Pos& hiderPos, const Pos& hiderPrevPos);

    char _scoreType;

};

#endif // VERYSMARTHIDER_H
