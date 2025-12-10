#ifndef SMARTHIDER_H
#define SMARTHIDER_H

#include "AutoHider/randomhider.h"

/*!
 * \brief The SmartHider class calculates a score based on the distance to the base and the seeker, it chooses the action
 *  that gives the highest score.
 */
class SmartHider : public RandomHider
{
public:
    //AG130206: score parameters
    static constexpr double SCORE_DEFAULT_DHS_FACTOR = 0; //NOT USED ANYMORE!! 0.4;
    static constexpr double SCORE_DEFAULT_RAND_STD = 0; //DEFAULT set to no noise //2
    static constexpr double SCORE_DEFAULT_LESS_RAND_DIST = 0; //DEFAULT no noise, // 3


    /*
     * \brief SmartHider
     * \param assumeAlwaysVisible when true it does not take into account visiblity and always knows both positions
     */
    SmartHider(SeekerHSParams* params, bool allKnowing = false);

    virtual ~SmartHider();

    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1);

    //virtual Pos getInitPos(GMap *gmap);

    /*!
     * \brief setParameters set parameters
     * \param scoreRandStd
     * \param scoreDhsFactor
     * \param scoreStdLessRandomDist
     */
    void setParameters(double scoreRandStd=SCORE_DEFAULT_RAND_STD, double scoreDhsFactor=SCORE_DEFAULT_DHS_FACTOR,
            double scoreStdLessRandomDist=SCORE_DEFAULT_RAND_STD);

    virtual std::string getName() const;

    virtual int getHiderType() const;

protected:
    //! get score for this pos, visib: if true than it assumes full visibility if false not and only uses distance to base
    //virtual double score(int sr, int sc, int hr, int hc, bool visib);
    virtual double score(const Pos& seekerPos, const Pos& hiderPos);

    //! score for action, returns score, and a bool indicating if movement possible (then score is not valued if false)
    //virtual bool scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double & score);
    virtual bool scoreForAction(const Pos& seekerPos, const Pos& hiderPos, int a, double & score);


    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);


    //! if it is allknowing it doesn't care for any noise
    bool _allKnowing;


    //params for the scoring


    //! std deviation for score
    double _scoreRandStd;

    //! d_hs factor
    double _scoreDhsFactor;

    //! std less random dist: distance from which the noise std decreases
    double _scoreStdLessRandDist;


    //std::string _name;

};

#endif // SMARTHIDER_H
