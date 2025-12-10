#ifndef HSMOMDP_LAYERED_H
#define HSMOMDP_LAYERED_H


#include "hsmomdp.h"
#include "../Segment/segment.h"

#include "AlphaVectorPolicy.h"

#include <vector>


/*! Layered HSMOMDP

  Note: _momdp is used as Bottom MOMDP!
  */
class LayeredHSMOMDP : public HSMOMDP {
public:
    //! SEGMENT_VALUE refers to the value type used for segmentation
    static const char SEGMENT_VALUE_BELIEF_ONLY = 0;
    //! reward uses the direct reward,
    static const char SEGMENT_VALUE_REWARD_ONLY = 1;
    static const char SEGMENT_VALUE_BELIEF_REWARD = 2;

    //! top reward agregation
    static const char TOP_REWARD_SUM = 0;
    static const char TOP_REWARD_AVG = 1;
    static const char TOP_REWARD_MIN = 2;
    static const char TOP_REWARD_MAX = 3;


    LayeredHSMOMDP();
    ~LayeredHSMOMDP();

    //! Init with the POMDP file and log files
    bool init(const char* pomdpFile, const char* logOutFile, const char* timeLog = NULL,
              bool useFIBUBInit=true, char segmentValue=SEGMENT_VALUE_BELIEF_ONLY, const char* policyFile=NULL,
              char topRewardAggregation=TOP_REWARD_SUM, bool segmentSeekerStates=false, bool segmentObservations=false,
              bool setFinalStateOnTop=false, bool setFinalTopRewards=false);

    /*! Init belief with the GMap
    it assumes that the gmap alread contains: initial positions of seeker and hider, and the visibility map of the seeker
    */
    virtual bool initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool hiderVisible);

    virtual bool updateBelief(int observ, int x_state);


    void setSegmenter(Segmenter* segmenter) {
        _segmenter = segmenter;
    }

    Segmenter* getSegmenter() {
        return _segmenter;
    }

    void setSegmenterX(Segmenter* segmenter) {
        _segmenterX = segmenter;
    }

    Segmenter* getSegmenterX() {
        return _segmenterX;
    }

    // ! Update belief
    //bool updateBelief(int observ, int y_state, int x_state);

    //! Generate the Top MOMDP
    bool generateTopMOMDP();

    //generate bottom MOMDP
    //bool generateBottomMOMDP(GMap* gmap);

    //! Compress bottom belief to top
    bool compressBelief();

    //! Solve the MOMDP, Get best action
    virtual int getBestAction();

    /*! Get next best action to do, based on the Y state (hider pos), X state (seeker pos) and visibility of the hider
    note: for hierarchical can be 1-5 */
    int getNextAction(Pos seekerPos, Pos hiderPos, bool visible);

    virtual vector<int> getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int n);

    //ag120420
    //! use FIB Upper bound init (if no use (Q)MDP; see APPL)
    //! used for solving the top MOMDP
    void setUseFIBUBInit(bool useFIBUBInit=true);

    //ag120420
    //! get FIB Upper bound init (if no use (Q)MDP; see APPL)
    bool getUseFIBUBInit();

    //ag120423
    //! set target precision and initial precision factor
    void setTargetPrecision(double targetPrecision,double targetInitPrecFactor);

protected:
    //! get next best action to do, based on the Y state (hider pos), X state (seeker pos) and visibility of the hider
    virtual int getNextAction(int observ, int x_state);

private:

    //! Generates top states
    vector<string> generateTopStates(int numActions, int numStates, Segmenter* segmenter, vector<int>* &listBottomTopStates,
                                     vector< vector<int> >& listTopBottomStates);

    //AG121011:
    //! Get rank of action in the vector
    int getRankOfAction(int a, vector<double>* valPerA);

#ifdef DEBUG_COMPARE_TOP_BOTTOM
    //AG121015: compare top+bottom
    bool compareTopAndBottomMOMDP();
#endif

//#ifdef DEBUG_CHECK_TOP
    bool checkTopMOMDP();
//#endif

    bool isFinalState(unsigned int x, unsigned int y, vector<int>& xBs, vector<int>& yBs, bool& seekerWins);


    //! the (APPL) top MOMDP
    SharedPointer<MOMDP> _momdpTop;

    //! Solver for top MOMDP
    HSSolver* _topSolver;

    //! segmenter used
    Segmenter *_segmenter;

    //AG121221
    //! segmenter used for the X states
    Segmenter *_segmenterX;

    //! define on which values to segment, seeSEGMENT_VALUE_*
    char _segmentValueType;

    /*! stores to which top state the bottom state belongs
    vector<int> -> [i] = j -> bottom state i is in top state j */
    vector<int> *_listBottomTopStates;

    //! list of vectors [j] for top states j, having the bottom states in a vector
    vector< vector<int> > _listTopBottomStates;

    /*! stores to which top state the bottom state belongs
    vector<int> -> [i] = j -> bottom state i is in top state j */
    vector<int> *_listBottomTopStatesX;

    //! list of vectors [j] for top states j, having the bottom states in a vector
    vector< vector<int> > _listTopBottomStatesX;

    //! compare offline and layered
    bool _compareWithOffline;

    //! top reward aggregation
    char _topRewardAggregation;

    //! segment seeker states (like hider)
    bool _segmentSeekerStates;

    //! segment observations (like hider)
    bool _segmentObservations;

    //ag130503
    //! set final state on top
    bool _setFinalStateOnTop;
    //! set final state top rewards
    bool _setFinalTopRewards;

    //! base state
    int _baseState;
};


#endif // HSMOMDP_LAYERED_H

