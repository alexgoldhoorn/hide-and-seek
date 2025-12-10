#include "AutoHider/sfmwalkers.h"

#include <cassert>
#include <iostream>

#include "Utils/generic.h"
#include "Base/hsglobaldata.h"

#include "exceptions.h"

using namespace std;

SFMWalkers::SFMWalkers(SeekerHSParams *params, size_t n, std::vector<Pos> goals) :
    /*AutoHider(params),*/ AutoWalker(params, n), _goals(goals), _nWalkers(n)
{
    //assert(nWalkers>0);
    //assert(goals.size()>0);
}

SFMWalkers::SFMWalkers(SeekerHSParams *params, size_t n) :
    /*AutoHider(params),*/ AutoWalker(params, n), _nWalkers(n)
{
    //assert(nWalkers>0);
}

SFMWalkers::~SFMWalkers() {
}

bool SFMWalkers::initBeliefRun() {  //(GMap *gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    initRandomizer();

    //set vector lengths
    DEBUG_AUTOHIDER(cout<<"SFMWalkers: init "<<_nWalkers<<" persons"<<endl;);
    //_curPos.resize(_nWalkers);
    _curVel.resize(_nWalkers);
    _nextPos.resize(_nWalkers);
    _nextVel.resize(_nWalkers);
    _curGoals.resize(_nWalkers);

    initPersons();

    return true;
}


void SFMWalkers::initPersons() {
    DEBUG_AUTOHIDER(cout<<"SFMWalkers::initPersons: "<<endl;);
    assert(_map!=NULL);

    for(int i=0; i<_nWalkers; i++) {
        //set init pos //todo: maybe require hidden pos
        //_curPos[i] =
        IDPos pos(_map->genRandomPos(),i);
        _autoWalkerVec[i] = pos;
        //also _nextpos??

        //set goal
        if (_goals.size()==0) {
            _curGoals[i] = _map->genRandomPos();
        } else {
            //choose goal
            int gi = random(_goals.size()-1);
            //set goal
            _curGoals[i] = _goals[gi];
        }

        DEBUG_AUTOHIDER(cout<<" * "<<i<<": starts at: "<<pos.toString() << ", goal: "<<_curGoals[i].toString()<<endl;);
    }
}


//int SFMWalkers::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    /*int a = -1;

    do {
        _hiderPlayer.setCurPos(hiderPos);
        a = random(HSGlobalData::NUM_ACTIONS-1); // rand() % HSGlobalData::NUM_ACTIONS;
    } while (!_hiderPlayer.move(a));

    return a;*/
    /*throw CException(_HERE_,"SFMWalkers::getNextAction: not implemented, use getAllNextPos");
    return -1;
}*/

Pos SFMWalkers::getNextPosRun(int actionDone, int *newAction) {
    /*int a = -1;

    do {
        _hiderPlayer.setCurPos(hiderPos);
        a = random(HSGlobalData::NUM_ACTIONS-1); // rand() % HSGlobalData::NUM_ACTIONS;
    } while (!_hiderPlayer.move(a));

    return a;*/
    throw CException(_HERE_,"SFMWalkers::getNextPosRun: not implemented, use getAllNextPos");

    return Pos();
}


void SFMWalkers::movePerson(int i) {
    //TODO
}


std::string SFMWalkers::getName() const {
    return "SFMWalkers";
}

int SFMWalkers::getHiderType() const {
    return HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM;
}


/*bool SFMWalkers::hasDynamicObstacles() {
    return true;
}

vector<Pos> SFMWalkers::getDynamicObstacles() {
    //vector<Pos> dynObst;
    return _curPos; //TODO: should not include the hider
}*/


std::vector<IDPos> SFMWalkers::getAllNextPos(Pos seekerPos, Pos hiderPos) {
    return std::vector<IDPos>();
}

/*void SFMWalkers::setMap(GMap* map) {
    AutoHider::setMap(map);
}

GMap* SFMWalkers::getMap() const {
    return AutoHider::getMap();
}

SeekerHSParams* SFMWalkers::getParams() const {
    return _params;
}*/
