#ifndef HSOBSERVATION_H
#define HSOBSERVATION_H

#include "POMCP/observation.h"
#include "POMCP/hsstate.h"
#include "HSGame/idpos.h"

namespace pomcp {

    struct HSObservation : public Observation {
    public:
        HSObservation();

        //HSObservation(const HSObservation* obs);

        /*!
         * \brief HSObservation creates an observation from this seeker's position and observation, and the positions and
         * observations of other players.
         * \param thisSeeker
         * \param allPlayersVec
         */
        HSObservation(const PlayerInfo* thisSeeker, const std::vector<PlayerInfo*>& allPlayersVec);

        virtual ~HSObservation();

        /*!
         * \brief castFromState casts from const Observation* to const HSObsevation*, if NULL it returns NULL
         * \param state
         * \return
         */
        static const HSObservation* castFromObservation(const Observation* obs);

        /*!
         * \brief castFromState casts from Observation* to HSObsevation*, if NULL it returns NULL
         * \param state
         * \return
         */
        static HSObservation* castFromObservation(Observation* obs);

        /*!
         * \brief readFromPlayerInfo read the observations from the player info. It sets the ownSeekerObs
         * based on thisSeekerPlayer, and the otherSeekersObsVec is set based on the allPlayersVec, but only
         * for the PlayerInfo's in that vector which are not thisSeeker, and which are of the player type P_Seeker.
         * \param thisSeeker
         * \param allPlayersVec
         * \return if read without problems
         */
        bool readFromPlayerInfo(const PlayerInfo* thisSeekerPlayer, const std::vector<PlayerInfo*>& allPlayersVec);


        virtual State* copy() const;

        virtual std::string toString() const;

        virtual bool operator <(const State& other) const ;

        virtual bool operator >(const State& other) const ;

        virtual bool operator ==(const State& other) const;

        virtual bool operator !=(const State& other) const;

        virtual std::string getHash() const;

        virtual StateType getStateType() const;

        virtual const State* getUpdateObservationState() const;

        /*!
         * \brief getRandomState get random state based on the prob of the HSState in otherSeekersObsVec or ownSeekerObs,
         * the normalized passed index is used to choose the HSState. ownSeekerObs is chosen when i<ownSeekerObs.prob,
         * otherwise otherSeekersObsVec[j].prob is used to choose otherSeekersObsVec.
         * \param i (random) value in [0,1]
         * \return a randomly chosen state (has to be deleted by caller)
         */
        HSState* getRandomState(double i) const;

        /*!
         * \brief size the number of states contained in this observation, should be at least 1 (own seeker's observation)
         * \return the number of observations
         */
        std::size_t size() const;

        /*!
         * \brief getObservationState get the observation, the first item is the observation of this seeker, the following are other seekers'
         * observations
         * \param i the index
         * \return the observed state
         */
        const HSState* getObservationState(std::size_t i) const;

        //has to be implemented, but won't be used here
        virtual void convertToObservation();

        /*!
         * \brief getSeekerPoses extracts the seeker poses from the observation (own pos, and other seekers)
         * \return
         */
        std::vector<Pos> getSeekerPoses() const;

        //AG160215
        /*!
         * \brief hasVisibleHider
         * \return
         */
        bool hasVisibleHider() const;


        //! the own seeker obs
        HSState ownSeekerObs;
        //! the other seeker obs
        std::vector<HSState> otherSeekersObsVec;

    };
}


#endif // HSOBSERVATION_H

