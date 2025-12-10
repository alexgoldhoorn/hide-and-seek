#include "PeoplePrediction/peoplepredictionwrapper.h"
#include "Base/hsconfig.h"
#include "exceptions.h"

#include <iostream>

//define position of x and y in return vector; since we are working with row,col, the order of x,y is reversed
#define X_POS_MEANVEC_INDEX 1
#define Y_POS_MEANVEC_INDEX 0
#define XX_POS_COVVEC_INDEX 3
#define XY_POS_COVVEC_INDEX 2
#define YX_POS_COVVEC_INDEX 1
#define YY_POS_COVVEC_INDEX 0

using namespace std;


PeoplePredictionWrapper::~PeoplePredictionWrapper(){
}

#ifdef DO_USE_PEOPLEPREDICTION //AG120121: disabled because not finished and not used

PeoplePredictionWrapper::PeoplePredictionWrapper(SeekerHSParams* params) :
    _params(params),
    _planner(_params->ppHorizonSteps, false /*true*/, Cperson_abstract::Low_pass_linear_regression_filtering),
    _updated(false), _obsScene(1), _obsScenePlanner(1), _robotObs(0,0,0) {

    _planner.set_dt(1);
    _scene.set_dt(1);
    _scene.set_number_virtual_people( 1 ); //num of people
    _scene.set_remove_targets(false); //remove

    //set person id to be 1 (the first and only)
    _obsScene[0].id = 1;
    _obsScenePlanner[0].id = 1;

    //init to NULL
    _personPlanner = _personScene = NULL;

    DEBUG_PEOPLEPRED(cout<<"PeoplePredictionWrapper: file="<<_params->ppDestinationFile<<"; horizon steps="<<_planner.get_horizon_time()
                     <<"; horizon steps index="<<_planner.get_horizon_time_index() <<"; dt="<<_planner.get_dt() <<endl;);

    if (params->ppDestinationFile.size()==0) {
        throw CException(_HERE_,"People prediction destination file is required");
    }

    if ( !_scene.read_destination_map( params->ppDestinationFile.c_str() ) ) {
        std::cout << "Could not read map destinations file !!!" << std::endl;
    } else {
        DEBUG_PEOPLEPRED(cout << "Scene read destinations map file "<<_params->ppDestinationFile<<" successfully"<< std::endl;);
    }

    if ( !_planner.read_destination_map(_params->ppDestinationFile.c_str() ) ) {
        std::cout << "Could not read map destinations file !!!" << std::endl;
    } else {
        DEBUG_PEOPLEPRED(cout << "Planner read destinations map file "<<_params->ppDestinationFile<<" successfully"<< std::endl;);
    }
}


void PeoplePredictionWrapper::update(double x, double y, double vx, double vy) {
    //based on prediction_example of Gonzalo
    DEBUG_PEOPLEPRED(cout<<"PeoplePredictionWrapper::update: update person: pos=("<<x<<","<<y<<") with velocity=("<<vx<<","<<vy<<")"<<endl;);

    //void observation, just for the timestamp
    _obsScene[0].time_stamp = _now;
    //update scene
    _scene.update_scene(_obsScene);

    // get person
    _scene.find_person(1, &_personScene);
    assert(_personScene!=NULL);

    //observation as received
    _obsScenePlanner[0].x = x;
    _obsScenePlanner[0].y = y;
    _obsScenePlanner[0].vx = vx;
    _obsScenePlanner[0].vy = vy;
    _obsScenePlanner[0].time_stamp = _now;
    //update planner
    _planner.update_scene(_obsScenePlanner);

    // get person
    _planner.find_person(1, &_personPlanner);
    assert(_personPlanner!=NULL);

    //robot state updated (only time)
    _robotObs.time_stamp = _now;
    _planner.update_robot(_robotObs);

    //increase time
    _now += 1;
    _updated = true;

    DEBUG_PEOPLEPRED(cout<<"PeoplePredictionWrapper::update done"<<endl;);
}


bool PeoplePredictionWrapper::predict(std::vector<std::vector<double> > &meanVec, std::vector<std::vector<double> > &covVec,
                                      unsigned int timeHorizon) {

    bool ok = true;

    DEBUG_PEOPLEPRED(cout<<"PeoplePredictionWrapper::predict"<<endl;);

    assert(timeHorizon==0); //TODO: could be passed later on

    if (!_updated) {
        throw CException(_HERE_, "you should call update() before calling predict()");
        //no return
    }

    //do prediction
    _planner.scene_prediction();

    assert(_personPlanner!=NULL);

    //get prediction
    const std::vector<SpointV_cov>* predVec = _personPlanner->get_prediction_trajectory();

    if (predVec==NULL) {
        cout << "WARNING PeoplePredictionWrapper::predict: get_prediction_trajectory returned a NULL vector, returning nothing"<<endl;
        ok = false;
    } else if (predVec->empty()) {
        cout << "WARNING PeoplePredictionWrapper::predict: get_prediction_trajectory returned an empty vector, returning nothing"<<endl;
        ok = false;
    } else {
        //prepare vectors
        meanVec.resize(predVec->size());
        covVec.resize(predVec->size());
//cout<<"Set pred. vect:"<<endl;
        //fill vectors
        //for(const SpointV_cov& spoint : predVec) {
        for (size_t i = 0; i<predVec->size(); i++) {
            //cout<<" "<<i<<")"<<flush;
            //get point
            auto p = (*predVec)[i];
            //cout<<" p=NULL:"<<(p==NULL)<<endl;
            //p.print();

            //set mean
            vector<double>& mean = meanVec[i];
            mean.resize(2);
            mean[X_POS_MEANVEC_INDEX] = p.x;
            mean[Y_POS_MEANVEC_INDEX] = p.y;
            //set cov
            vector<double>& cov = covVec[i];
            cov.resize(4);
            cov[XX_POS_COVVEC_INDEX] = p.cov_xx();
            cov[XY_POS_COVVEC_INDEX] = cov[YX_POS_COVVEC_INDEX] = p.cov_xy();
            cov[YY_POS_COVVEC_INDEX] = p.cov_yy();
        }
    }

    _updated = false;

    return ok;
}

double PeoplePredictionWrapper::getTime() {
    return _now;
}

#else
PeoplePredictionWrapper::PeoplePredictionWrapper(SeekerHSParams* params) {
}

#endif
