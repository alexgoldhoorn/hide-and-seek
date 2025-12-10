#ifndef PEOPLEPREDICTIONWRAPPER_H
#define PEOPLEPREDICTIONWRAPPER_H

#include "seekerhsparams.h"

#ifdef DO_USE_PEOPLEPREDICTION //AG120121: disabled because not finished and not used
#include "prediction_behavior.h"
#include "scene_sim.h"
#endif

#include <vector>

class PeoplePredictionWrapper {
public:
    PeoplePredictionWrapper(SeekerHSParams* params);

    ~PeoplePredictionWrapper();

#ifdef DO_USE_PEOPLEPREDICTION //AG120121: disabled because not finished and not used
    /*!
     * \brief update updates state of predictor with location (x,y) and speed (vx,vy)
     * \param x
     * \param y
     * \param vx
     * \param vy
     */
    void update(double x, double y, double vx, double vy);

    /*!
     * \brief predict returns the predicted steps
     * \param meanVec
     * \param covVec
     * \param timeHorizon (=0) NOT USED, use SeekerHSParams::ppHorizonSteps
     */
    bool predict(std::vector<std::vector<double>>& meanVec, std::vector<std::vector<double>>& covVec, unsigned int timeHorizon = 0);

    /*!
     * \brief getTime get current time
     * \return
     */
    double getTime();

private:
    //! params
    SeekerHSParams* _params;

    //! scene of the world
    Cscene_sim _scene;

    //! planner, used for prediction
    Cprediction_behavior _planner;

    //! flag to know if update has been done
    bool _updated;

    //! current 'time'
    double _now;

    //! observation vector
    std::vector<SdetectionObservation> _obsScene;

    //! observation vector for planner
    std::vector<SdetectionObservation> _obsScenePlanner;

    //! robot obs.
    Spose _robotObs;

    //! person to follow (scene)
    Cperson_abstract* _personScene;
    //! person to follow (planner)
    Cperson_abstract* _personPlanner;
#endif
};


#endif // PEOPLEPREDICTIONWRAPPER_H
