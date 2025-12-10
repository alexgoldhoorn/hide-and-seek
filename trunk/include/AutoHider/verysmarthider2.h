#ifndef VERYSMARTHIDER2_H
#define VERYSMARTHIDER2_H

#include "AutoHider/verysmarthider.h"

/*!
 * \brief The VerySmartHider2 class is a renewed smart hider with a slightly different score formula and
 */
class VerySmartHider2 : public VerySmartHider
{
public:
//    static constiexpr double SCORE_DEFAULT_D2 = 0;
    static constexpr double SCORE_DEFAULT_D2 = 0;
    static constexpr double SCORE_DEFAULT_F1 = 0.5;
    static constexpr double SCORE_DEFAULT_F2 = -0.2;
    static constexpr double SCORE_DEFAULT_K  = 0;
    static constexpr double SCORE_DEFAULT_F3 = 0;

    VerySmartHider2(SeekerHSParams* params, bool allKnowing = false, char scoreTypeHidden = SCORE_AVG, char scoreTypeSeekerActions = SCORE_AVG);

    virtual ~VerySmartHider2();

    void setParameters2(double D2 = SCORE_DEFAULT_D2, double f1 = SCORE_DEFAULT_F1, double f2 = SCORE_DEFAULT_F2, double k = SCORE_DEFAULT_K,
                        double f3 = SCORE_DEFAULT_F3, bool allKnowing = false, char scoreTypeHidden = SCORE_AVG, char scoreTypeSeekerActions = SCORE_AVG);

    //virtual void setMap(GMap* map);

    virtual std::string getName() const;

protected:
    //overwrite scoreForAction: use score type to decide using avg, min, max
    //virtual double scoreForState(int sr, int sc, int hr, int hc, int hrp, int hcp);
    virtual double scoreForState(const Pos& seekerPos, const Pos& hiderPos, const Pos& hiderPrevPos);

    //use new score formula
    //virtual double score(int sr, int sc, int hr, int hc, bool visib);
    virtual double score(const Pos& seekerPos, const Pos& hiderPos);

    char _scoreTypeSeekerActions;

    double _D2, _f1, _f2, _k, _f3;

    double _maxScore;
};

#endif // VERYSMARTHIDER2_H
