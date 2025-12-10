#ifndef HSSTATE_H
#define HSSTATE_H

#include "HSGame/gmap.h"

#include "POMCP/state.h"

#include "Base/playerinfo.h"

namespace pomcp {

/*!
 * \brief The HSState class position of hider and seeker
 */
struct HSState : public State {
public:
    HSState();
    /*!
     * \brief HSState
     * \param seekerPos
     * \param hiderPos
     * \param makeContinuous if true, the pos will be made continuous if it is int (i.e. add 0.5, to put it in the centre)
     */
    HSState(Pos seekerPos, Pos hiderPos);
    HSState(int seekerRow, int seekerCol, int hiderRow, int hiderCol);

    virtual ~HSState();

    /*!
     * \brief castFromState casts a State* to HSState*, if NULL it returns NULL
     * \param state
     * \return
     */
    static const HSState* castFromState(const State* state);

    static HSState* castFromState(State* state);


    //AG140108: change it to make it compatible with next
    Pos hiderPos;
    Pos seekerPos;

    bool hiderVisible() const;

    virtual State* copy() const;

    virtual std::string toString() const;

    virtual bool operator <(const State& other) const ;

    virtual bool operator >(const State& other) const ;

    virtual bool operator ==(const State& other) const;

    virtual bool operator !=(const State& other) const;

    //virtual void operator =(const State& other) const;

    virtual std::string getHash() const;

    virtual StateType getStateType() const;

    /*!
     * \brief convPosToInt convert position to int
     */
    virtual void convPosToInt();

    /*!
     * \brief convPosToCont convert pos to continuous
     */
    virtual void convPosToCont();

    virtual void convertToObservation();

    /*!
     * \brief readFromPlayer reads the seeker pos and hider pos from the playerInfo
     * \param playerInfo
     * \returns if no
     */
    virtual bool readFromPlayer(const PlayerInfo& playerInfo);
};

}

#endif // HSSTATE_H
