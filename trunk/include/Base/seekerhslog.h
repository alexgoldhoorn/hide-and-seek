#ifndef SEEKERHSLOG
#define SEEKERHSLOG

#include "Utils/hslog.h"

#include "Base/posxy.h"
#include "Base/playerinfo.h"
#include "HSGame/gmap.h"

/*#include "HSGame/gmap.h"
#include "Base/seekerhsparams.h"*/
//#ifndef SEEKERHS_H
class SeekerHS;
//#endif
//#include "Base/seekerhs.h"

/*!
 * \brief The SeekerHSLog class takes care of logging to a CSV file during the real games.
 * If a file already exists the new data is appended to the end. New games can be found because of the game state value of -1.
 * For each log a header is generated to indicate the meaning of the columns.
 * For the different type of games slightly different log files are generated (e.g. if more robots are used).
 *
 * WARNING: when appending a log file the header is not repeated and no test is done if the same type of log file will be generated.
 */
class SeekerHSLog {

public:
    SeekerHSLog(SeekerHS* seekerHS);

    virtual ~SeekerHSLog();

    /*!
     * \brief open the game log file and start header if not yet exists
     * \param gamelogFile
     */
    void open/*GameLogFile*/(std::string gamelogFile);

    //! close the file
    void close();

    //! init log line (no new seeker pos)
    /*void logLineInit(const std::vector<double> &seekerPosVector, const std::vector<double> &hiderPosVector, const Pos &seekerPos,
                     const IDPos &hiderPos, bool hiderVisible);*/

    void init(GMap* gmap, const SeekerHSParams* params, const PlayerInfo* thisPlayer, const PlayerInfo* otherPlayer,
                     const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const PosXY* otherSeekerPosXY, const PosXY* otherHiderObsPosXY);

    //! log line
    /*void logLine(std::vector<double> &seekerPosVector, std::vector<double> &hiderPosVector, Pos &seekerPos,
                 IDPos &hiderPos, bool& hiderVisible, std::vector<double> &newSeekerPosVector, Pos &newSeekerPos, int winState);*/


    void logLine(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, const PosXY *otherSeekerPosXY, const PosXY *otherHiderObsPosXY,
                 int winState);


    void logLine2(const PosXY* chosenGoalPosXY);

    /*
    //! init log line (no new seeker pos) for 2 seekers
    void logLineInit2(const std::vector<double>& seekerPosVectorMine,  const std::vector<double>& hiderPosVectorMine,
                     const std::vector<double>& seekerPosVectorOther, const std::vector<double>& hiderPosVectorOther,
                     const Pos& seekerPosMine,  const IDPos &hiderPosMine, bool hiderVisible,
                     const Pos& seekerPosOther, const Pos& hiderPosOther);

    //! log line for 2 seekers
    void logLine2(const std::vector<double>& seekerPosVectorMine,  const std::vector<double>& hiderPosVectorMine,
                 const std::vector<double> seekerPosVectorOther, const std::vector<double> hiderPosVectorOther,
                 const Pos& seekerPosMine,  const IDPos &hiderPosMine, bool hiderVisible,
                 const Pos* seekerPosOther, const Pos* hiderPosOther,
                 const std::vector<double>& newSeekerPosVectorMine, const std::vector<double>& newSeekerPosVectorOther,
                 const Pos& newSeekerPosMine, const Pos& newSeekerPosOther,
                 double newSeekerPosMineBelief, double newSeekerPosOtherBelief, int winState);

    //! log line 2 seekers, when selecting action based on both seeker's decisions
    void logLine2Select(const std::vector<double>& newSeekerPosVectorMineFromOther, const std::vector<double>& newSeekerPosVectorOtherFromOther,
                        const Pos& newSeekerPosMineFromOther, const Pos& newSeekerPosOtherFromOther,
                        double newSeekerPosMineBeliefFromOther, double newSeekerPosOtherBeliefFromOther,
                        const std::vector<double>& selectedGoalPosVectorMine, const Pos& selectedGoalPosMine);
*/

    void stopGame(int winState);

private:

    void logLine1Init();

    void logLine1AfterPos(int winState);

    //! params
    const SeekerHSParams* _params;

    //! map
    GMap* _map;

    //! the object which contains params and map
    SeekerHS* _seekerHS;

    //! game log
    HSLog* _gamelog;

    //! this seeker
    const PlayerInfo* _thisSeekerPlayerInfo;
    //! other seeker
    const PlayerInfo* _otherSeekerPlayerInfo;

    //! line no
    unsigned long _lineNum;

};

#endif // SEEKERHSLOG

