#ifndef AUTOHIDER_H
#define AUTOHIDER_H

#include "Base/autoplayer.h"
#include "HSGame/gplayer.h"
#include <string>

/*!
 * \brief The AutoHider class abstract class for auto hiders
 */
class AutoHider : public AutoPlayer
{
public:
    AutoHider(SeekerHSParams* params);

    virtual ~AutoHider();

    virtual double getBelief(int r, int c);

    virtual bool tracksBelief() const;

    /*!
     * \brief getHiderType return the hider type (HSGlobalData::OPPONENT_TYPE_*)
     * \return
     */
    virtual int getHiderType() const=0;

    virtual bool isSeeker() const;
};

#endif // AUTOHIDER_H
