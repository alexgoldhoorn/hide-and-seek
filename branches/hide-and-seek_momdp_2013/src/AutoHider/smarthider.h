#ifndef SMARTHIDER_H
#define SMARTHIDER_H

#include "randomhider.h"


class SmartHider : public RandomHider
{
public:
    //AG130206: score parameters
    static const double SCORE_DEFAULT_DHS_FACTOR = 0.4;
    static const double SCORE_DEFAULT_RAND_STD = 2;
    static const double SCORE_DEFAULT_LESS_RAND_DIST = 3;


    /*
     * \brief SmartHider
     * \param assumeAlwaysVisible when true it does not take into account visiblity and always knows both positions
     */
    SmartHider(bool allKnowing = false);

    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible);

    //virtual Pos getInitPos(GMap *gmap);

    /*!
     * \brief setParameters set parameters
     * \param scoreRandStd
     * \param scoreDhsFactor
     * \param scoreStdLessRandomDist
     */
    void setParameters(double scoreRandStd=SCORE_DEFAULT_RAND_STD, double scoreDhsFactor=SCORE_DEFAULT_DHS_FACTOR,
            double scoreStdLessRandomDist=SCORE_DEFAULT_RAND_STD);

    virtual std::string getName() {
        return _name;
    }

    virtual int getHiderType() {
        return _allKnowing ? HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING : HSGlobalData::OPPONENT_TYPE_HIDER_SMART;
    }

protected:
    //! get score for this pos, visib: if true than it assumes full visibility if false not and only uses distance to base
    virtual double score(int sr, int sc, int hr, int hc, bool visib);

    //! score for action, returns score, and a bool indicating if movement possible (then score is not valued if false)
    virtual bool scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double & score);

    //! if it is allknowing it doesn't care for any noise
    bool _allKnowing;


    //params for the scoring


    //! std deviation for score
    double _scoreRandStd;

    //! d_hs factor
    double _scoreDhsFactor;

    //! std less random dist: distance from which the noise std decreases
    double _scoreStdLessRandDist;


    std::string _name;

};

#endif // SMARTHIDER_H
