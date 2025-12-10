#ifndef FROMLISTHIDER_H
#define FROMLISTHIDER_H

#include <vector>

#include "AutoHider/autohider.h"
#include "Base/hsglobaldata.h"

#include "autowalker.h"

#ifdef USE_QT
#include <QFile>
#include <QTextStream>
#else
class QFile;
class QTextStream;
#endif

/*!
 * \brief The FromListHider class uses a pregenerated list of positions. Use the static function writePosFile to generate a file.
 * The class can be used as hider or list of dynamic obstalces/auto walker. If the getNextAction (AutoHider) is used, only the first
 * column of the file is used. In the case of the usage as AutoWalker, all the columns are used, assuming it to have numHiders (as defined in the
 * file's header).
 */
class FromListHider : /*public AutoHider,*/ public AutoWalker
{
public:
    /*!
     * \brief FromListHider
     * \param params
     * \param n number of auto walkers, default 0, and only 1 'hider'
     */
    FromListHider(SeekerHSParams* params,  std::string listFile);

    virtual ~FromListHider();

    //virtual int getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone=-1);

    virtual std::string getName() const;

    virtual int getHiderType() const;

    virtual std::vector<IDPos> getAllNextPos(Pos seekerPos, Pos hiderPos);

    /*virtual void setMap(GMap* map);

    virtual GMap* getMap() const;*/

    //AG150525: override from AutoPlayer, because they are overridden from AutoWalker
    //virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible);    
    //virtual bool initBelief(GMap* gmap, /*PlayerInfo* seekerPlayer,*/ PlayerInfo* otherPlayer);
    //virtual bool initBeliefMulti(GMap* gmap, std::vector<PlayerInfo*> playerVec, int thisPlayerID, int hiderPlayerID);


    //virtual SeekerHSParams* getParams() const;


    virtual Pos getInitPos();

    /*!
     * \brief writePosFile Write the positions for the map, done by the passed auto hider. Each line contains 'numHiders' hiders, and in total it will
     * contain numSteps steps (lines).
     * File format:
     * -------------------
     * map-name
     * number of hiders
     * number of steps
     * step1_hider1_row,step1_hider1_col;step1_hider2_row,step1_hiderr2_col;.....
     * ...
     *
     * \param autoHider
     * \param map
     * \param outFile
     * \param numHiders
     * \param numSteps
     */
    static void writePosFile(AutoWalker* autoWalker, GMap* map, std::string outFile, uint numHiders, uint numSteps, bool useContinuous);

    /*!
     * \brief getNumberOfHiders return the number of hiders read
     * \return
     */
    unsigned int getNumberOfHiders() const;

    /*!
     * \brief getNumberOfSteps get number of steps
     * \return
     */
    unsigned int getNumberOfSteps() const;


    //virtual PlayerInfo* getPlayerInfo();

protected:

    //virtual Pos getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone=-1, int n=1);
    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL);

    /*!
     * \brief openListFile opens the file and reads the header
     * \param file
     */
    void openListFile(QString file);

    /*!
     * \brief readNextHiderPos reads the next hider poses from file into the vector _autoWalkerVec. Returns true if reading succeeded.
     * \return
     */
    bool readNextHiderPos();


    virtual bool initBeliefRun();


    //! name of player, contains file name
    std::string _name;

    //! step
    uint _stepNum;

    //! num steps
    uint _numSteps;

    //! num hiders
    uint _numHiders;

    //! file ref
    QFile* _listFile;

    //! stream
    QTextStream* _in;

};

#endif // FROMLISTHIDER_H
