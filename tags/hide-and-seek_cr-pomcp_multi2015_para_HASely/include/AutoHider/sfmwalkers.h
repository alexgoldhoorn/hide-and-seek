#ifndef SFMWALKERS_H
#define SFMWALKERS_H

#include "AutoHider/autohider.h"
#include "AutoHider/autowalker.h"
#include "HSGame/pos.h"

#include <vector>

/*!
 * \brief The SFMWalkers class Different walkers through the field that use the Social-Force Model to move.
 * Data from Ferrer, Garrell, Herrero, Sanfeliu 2013?
 *
 * This class contains 1 or more 'persons' moving, only the 1st one is used as a player, i.e. actions are sent.
 * The others are sent as 'dynamic obstacles'. The getAllNextPos gives a vector of all next positions of the walkers/persons.
 *
 * When no specific goals are passed to the constructor positions on the map are chosen randomly. Note that we assume that
 * the map doesn't have any enclosed regions.
 */
class SFMWalkers : /*public AutoHider,*/ public AutoWalker {
public:
    //Social-Force Model parameters
    static constexpr double SFM_k = 4.9;  // k [s^-1]
    static constexpr double SFM_A = 10;   // A [m/s^2]
    static constexpr double SFM_B = 0.34; // B [m]
    static constexpr double SFM_d = 0.16; // d [m]

    //TODO: params: num_walkers, goals

    /*!
     * \brief SFMWalkers the constructor
     * \param params
     * \param nWalkers number of persons to be simulated
     * \param goals possible goals for the persons, randomly chosen
     */
    SFMWalkers(SeekerHSParams* params, std::size_t n, std::vector<Pos> goals);

    /*!
     * \brief SFMWalkers the constructor without goals, in this case random goals are chosen on the available map
     * \param params
     * \param nWalkers
     */
    SFMWalkers(SeekerHSParams* params, std::size_t n);

    virtual ~SFMWalkers();

    //virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);

    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1);

    virtual std::string getName() const;

    virtual int getHiderType() const;

    /*virtual bool hasDynamicObstacles();

    virtual std::vector<Pos> getDynamicObstacles();*/


    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos);

    /*virtual void setMap(GMap* map);

    virtual GMap* getMap() const;

    virtual SeekerHSParams* getParams() const;*/

protected:
    /*!
     * \brief movePerson moves person i following Social-Force Model. It changes the positions of nextPos and nextVel, using curPos and curVel
     * \param i
     */
    virtual void movePerson(int i);

    /*!
     * \brief initPersons initializes goals
     */
    virtual void initPersons();


    virtual bool initBeliefRun();

    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);


    //AG140416 -> moved to GMap.genRandomPos()
    //Pos randomMapPos();


    //! current position of all walkers
    //std::vector<Pos> _curPos;

    //! current speed of all walkers
    std::vector<Pos> _curVel;

    //! next position of all walkers
    std::vector<Pos> _nextPos;

    //! next speed of all walkers
    std::vector<Pos> _nextVel;

    //! goals of the persons
    std::vector<Pos> _curGoals;

    //! possible goals
    std::vector<Pos> _goals;

    //! number of persons to be simulated
    int _nWalkers;
};

#endif // SFMWALKERS_H
