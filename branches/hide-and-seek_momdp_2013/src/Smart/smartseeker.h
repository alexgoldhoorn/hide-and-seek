#ifndef SMARTSEEKER_H
#define SMARTSEEKER_H

#include "autoplayer.h"

#include "HSGame/gplayer.h"
#include "seekerhs.h"

/*!
 * The SmartSeeker uses the triangle rule to calculate a score w for a certain state:
 *  w =  L - d_sh   if d_hb>=d_sb (hider not closer to base)
 *       -d_sh      otherwise
 *
 * with L = rows*cols ; a 'high' value, potential limit of steps
 *  d_xy: distance between x and y
 *  s: seeker, h: hider, b: base
 *
 * It calculates the score for each possible moves of the seeker. I.e.: it check what is the next seeker
 * position given the action, and based on that it check all the possible movements the hider can do. Over
 * all this positions (states) the an average score for that action will be calculated. In the end
 * the highest scored action will be chosen.
 * In the case of not seeing the hider it will check the score for all not visible hider positions and
 * average this.
 *
 * The scoreType is the score it uses when having multiple possible hider positions (i.e. hider being hidden):
 * average, minimum or maximum
 *
 * \brief The SmartSeeker class This class implements a Smart seeker simply following the triangle rule.
 */
class SmartSeeker : public AutoPlayer
{
public:
    static const char SCORE_AVG = 0;
    static const char SCORE_MIN = 1;
    static const char SCORE_MAX = 2;


    SmartSeeker(SeekerHSParams* params);//char scoreType=SCORE_AVG);

    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible);


    virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool hiderVisible);


    virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);


    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() {
        return false;
    }



private:
    /*!
     * used score:
     *  L - d_sh    if d_hb>=d_sh
     *  - d_sh      otherwise
     *
     * where: d_sh = distance seeker-hider;
     *        d_hb = distance hider-base
     *        L = max score, here: #rows * #cols
     *
     * \brief score score based on triangle reward
     * \param hiderPos
     * \param seekerPos
     * \return
     */
    double score(int sr, int sc, int hr, int hc);

    /*!
     * \brief scoreForAction score for 1 action of seeker with an avg of scores to all possible actions of hider
     * \param sr
     * \param sc
     * \param hr
     * \param hc
     * \param a
     * \return
     */
    bool scoreForAction(int sr, int sc, int hr, int hc, int a, bool visib, double& s);


    //! slightly more intelligent score: averages over all possible hider pos/actions
    double scoreForState(int sr, int sc, int hr, int hc);


    //! map
    GMap* _map;
    //! player to check movements
    Player _seekerPlayer;
    Player _hiderPlayer;
    //! max score
    int _maxScore;
    //! base
    Pos _base;
    /*//! score type
    char _scoreType;*/

    SeekerHSParams* _params;


};


#endif // SMARTSEEKER_H

