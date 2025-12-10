#include "Base/seekerhsparams.h"
#include "Base/playerinfo.h"
#include <limits>

/*!
 * \brief The PlayerInfoCompWrapper struct can be used to compare to playerInfo structs by either: id or
 * sum of highest belief.
 * Note 1: the sum of belief is calculated at creation (if this ordening is used) by summing all beliefs
 *  of the multiHBPosVec, after this it can be calculated with recalc().
 * Note 2: on equality the ordering (> or <) by belief is done by ID
 */
struct PlayerInfoWrapper {
    PlayerInfoWrapper(PlayerInfo* playerInfo, SeekerHSParams* params);

    bool operator <(const PlayerInfoWrapper& other) const;

    bool operator >(const PlayerInfoWrapper& other) const;

    bool operator ==(const PlayerInfoWrapper& other) const;

    bool operator !=(const PlayerInfoWrapper& other) const;

    //! recalculate the belief
    void recalc();

    //! recalculate the belief if required
    void recalcIfReq();

    //! get sum of belief (if set)
    double getBeliefSum();

    PlayerInfo* playerInfo;

    static double constexpr EPS = 2.0*std::numeric_limits<double>::epsilon();

private:
    SeekerHSParams* _params;

    double _beliefSum;

};
