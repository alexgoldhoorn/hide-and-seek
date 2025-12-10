#ifndef HSGAMELOGDB_H
#define HSGAMELOGDB_H

#include "Server/hsserverconfig.h"
#include "Server/hsgamelog.h"

#include <QSqlDatabase>


class HSGameLogDB : public HSGameLog
{
public:    

    HSGameLogDB(HSServerConfig* config);
    ~HSGameLogDB();


    virtual int startGame(QString mapName, GMap* gmap, SeekerHSParams* params, std::vector<PlayerInfoServer*> playerInfoVec);


    virtual long addGameStep(int actionNum, QDateTime sentTime, int status,
                                std::vector<PlayerInfoServer*> playerInfoVec,
                                PlayerInfoServer* hiderPlayerInfo);

    /*virtual int addGameStep(int actionNum, QDateTime sentTime,
                            QDateTime hiderTime, QDateTime seekerTime, QDateTime seeker2Time,
                            int hiderAction, int seekerAction, int seeker2Action,
                            const Pos& hider, const Pos& seeker, const Pos* seeker2,
                            int status, double dsh, double dsb, double dhb,
                            double dshEuc, double ds2h, double ds2hEuc, double dss2, double dss2Euc,
                            bool vis_sh, bool vis_s2h, bool vis_ss2,
                            const Pos* hiderPosWNoiseSent, const Pos* hiderPosWNoiseSent2,
                            double seekerBeliefScore, double seeker2BeliefScore, double seekerReward, double seeker2Reward,
                            const std::vector<Pos> goalVectorFromS1, const std::vector<double> goalBeliefVectorFromS1, const Pos& multiChosenPosS1,
                            const std::vector<Pos> goalVectorFromS2, const std::vector<double> goalBeliefVectorFromS2, const Pos& multiChosenPosS2);*/



    virtual int stopGame();

    virtual int close();

    virtual int getID();


protected:
    /*!
     * \brief queryAddBindValue add a string to the query and NULL if it is empty
     * \param query
     * \param str
     */
    void queryAddBindValueStr(QSqlQuery& query, std::string str);

    /*!
     * \brief queryAddBindValueStr queryAddBindValue add a string to the query and NULL if it is empty
     * \param query
     * \param str
     */
    void queryAddBindValueStr(QSqlQuery& query, QString str);



    static const QString DATE_TIME_MYSQL_FORMAT;

private:
    //! config file
    HSServerConfig* _config;

    //! database connection
    QSqlDatabase _db;

    //! game id of current game
    int _gameID;

    //! session id of opened session to db
    int _sessionID;

    //! map to do dist calculations
    GMap* _map;
};

#endif // HSGAMELOGDB_H
