#ifndef AUTOPLAYERTEST_H
#define AUTOPLAYERTEST_H

#include <QtTest/QtTest>
#include "Base/autoplayer.h"

/*!
 * \brief The AutoPlayerTest class Unit test for different type of AutoPlayer.
 *
 * TODO test:
 * - random movement of other players
 *
 */
class AutoPlayerTest: public QObject
{        
    Q_OBJECT

public:
    AutoPlayerTest(SeekerHSParams* params, GMap* map, AutoPlayer* autoPlayer);

private slots:

    void initTestCase();

    void autoPlayerTest();

    void cleanupTestCase();


private:
    //static int LAST_PLAYER_ID;
    static uint const OBS_NUM_OTHER_SEEKERS = 2;
    static uint const NUM_TEST_STEPS = 10;
    static double constexpr OBS_HIDER_NOT_SEEN_P = 0.3;
    static double constexpr MULTISEEK2_GOAL_NOBEL_P = 0.4;
    static double constexpr MULTISEEK2_CON_P = 0.8;

    //static int ;

    //! one seeker
    void autoPlayerSingleTest();
    //! multiple seekers
    void autoPlayerMultiTest();

    //! create other player, returns index of first hider, or -1 if none created
    int createPlayerInfoVec(std::vector<PlayerInfo*>& playerInfoVec, int nSeekers, int nHiders, double prob);

    //! delete playerInfo list
    void deletePlayerInfoVec(std::vector<PlayerInfo*> playerInfoVec);

    //! update players except 0, teh autoplayer
    void updatePlayerInfoVec(std::vector<PlayerInfo*>& playerInfoVec);

    //! set goal poses for 2 multi seeker
    void setMultiGoals2PlayerInfoVec(std::vector<PlayerInfo*>& playerInfoVec);



    SeekerHSParams* _params;
    GMap* _map;
    AutoPlayer* _autoPlayer;

    /*pomcp::HSObservation* genObs(GMap* map, const Pos& seekerPos, const Pos& hiderObsPos, int nOther);

    pomcp::HSState* genRandState(GMap* map);*/
    Pos randMove(GMap* map, const Pos& pos);

};



#endif
