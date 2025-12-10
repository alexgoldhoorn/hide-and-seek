
#include "AutoHider/fromlisthider.h"

#include <iostream>
#include <fstream>
#include <cassert>

#include <QFile>
#include <QTextStream>
#include <QStringList>

#include "exceptions.h"

using namespace std;

FromListHider::FromListHider(SeekerHSParams* params,  std::string listFile) : AutoHider(params), AutoWalker(1) {
    QString name = "File_" + QString::fromStdString(listFile);
    _name = name.toStdString();
    cout << "Name:"<<_name<<endl;
    openListFile(QString::fromStdString(listFile));
}

void FromListHider::openListFile(QString file) {
    DEBUG_CLIENT(cout <<"FromListHider::openListFile: opening file '"<<file.toStdString()<<"': "<<flush;);
    _in = NULL;
    _listFile = new QFile(file);
    if (!_listFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString msg = "cannot find the position list file: '" + file + "'";
        throw CException(_HERE_, msg.toStdString());
    }
    DEBUG_CLIENT(cout<<"ok"<<endl;);

    _in = new QTextStream(_listFile);
    QString line = _in->readLine();
    if (line.isNull()) {
        throw CException(_HERE_, "unexpected data format of the file, expected map name");
    }

    if (_map != NULL) {
        if (line.compare(QString::fromStdString(_map->getName()))!=0) {
            cout << "WARNING: map file name in the file is: "<<line.toStdString()<<" while the used map is: "<<_map->getName()<<endl;
        }
    } else {
        DEBUG_CLIENT(cout<< "Map: "<<line.toStdString()<<endl;);
    }

    //num hiders
    bool ok = false;
    line = _in->readLine();
    if (line.isNull()) {
        throw CException(_HERE_, "unexpected data format of the file, expected number of hiders");
    }
    _numHiders = line.toUInt(&ok);
    if (!ok) {
        throw CException(_HERE_, "unexpected data format of the file, expected number of hiders");
    }

    //num steps
    ok = false;
    line = _in->readLine();
    if (line.isNull()) {
        throw CException(_HERE_, "unexpected data format of the file, expected number of steps");
    }
    _numSteps = line.toUInt(&ok);
    if (!ok) {
        throw CException(_HERE_, "unexpected data format of the file, expected number of steps");
    }

    DEBUG_CLIENT(cout << "Number of hiders: "<<_numHiders<<endl<<"Number of steps: "<<_numSteps<<endl<<endl<<"Now ready to read file"<<endl;);

    //init step numbers
    _stepNum = 0;
    //set size auto hiders
    _autoWalkerVec.resize(_numHiders);
}

FromListHider::~FromListHider() {
    if (_in!=NULL) {
        delete _in;
    }
    if (_listFile->isOpen()) {
        _listFile->close();
    }
    delete _listFile;
}

int FromListHider::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    throw CException(_HERE_, "FromListHider::getNextAction: not implemented, use getNextPos");
}

Pos FromListHider::getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    if (!readNextHiderPos()) {
        cout<<"FromListHider::getNextPosRun: next position could not be read"<<endl;
    }
    assert(_autoWalkerVec.size()>0);
    DEBUG_AUTOHIDER(cout<<"FromListHider::getNextPosRun: returning "<<_autoWalkerVec[0].toString()<<endl;);

    return _autoWalkerVec[0];
}

std::vector<IDPos> FromListHider::getAllNextPos(Pos seekerPos, Pos hiderPos) {
    if (!readNextHiderPos()) {
        cout<<"FromListHider::getAllNextPos: next position could not be read"<<endl;
    }
    DEBUG_AUTOHIDER(cout<<"FromListHider::getAllNextPos: returning all poses"<<endl;);

    return _autoWalkerVec;
}

Pos FromListHider::getInitPos() {
    if (!readNextHiderPos()) {
        cout<<"FromListHider::getInitPos: next position could not be read"<<endl;
    }
    assert(_autoWalkerVec.size()>0);
    DEBUG_AUTOHIDER(cout<<"FromListHider::getInitPos: returning "<<_autoWalkerVec[0].toString()<<endl;);

    return _autoWalkerVec[0];
}

bool FromListHider::readNextHiderPos() {
    assert(_map!=NULL);
    DEBUG_AUTOHIDER(cout<<"FromListHider::readNextHiderPos: Reading next positions (#"<<_stepNum<<"/"<<_numSteps<<"):"<<endl;);

    if (!_listFile->isOpen()) {
        cout <<"FromListHider::readNextHiderPos: ERROR: the file is not open!"<<endl;
        return false;
    }
    //read next line
    QString line = _in->readLine();
    if (line.isNull()) {
        cout <<"FromListHider::readNextHiderPos: ERROR: no more lines can be read!"<<endl;
        return false;
    }

    QStringList posStrList = line.split(";");
    if (posStrList.size()!=(int)_numHiders) {
        throw CException(_HERE_, "Number of hider positions in header is inconsistent with amount in the file");
    }
    //now get all the positions
    for(uint i=0; i<_numHiders; i++) {
        //now get pos
        QString posStr = posStrList[i];
        QStringList posItemStrList = posStr.split(",");
        if (posItemStrList.size()!=2) {
            throw CException(_HERE_, "Number of items in position is not 2");
        }
        //parse pos
        bool ok = false;
        //AG140605: always read as double, safer
        //if (_params->useContinuousPos) {
            double row,col;
            row = posItemStrList[0].toDouble(&ok);
            if (ok) col = posItemStrList[1].toDouble(&ok);
            if (!ok) throw CException(_HERE_, "Could not read the hider position");

            _autoWalkerVec[i].set(row,col,i+1); //AG150224: ID=(i+1) to avoid the first being detected as hider (having id 0)
        /*} else {
            int row,col;
            row = posItemStrList[0].toInt(&ok);
            if (ok) col = posItemStrList[1].toInt(&ok);
            if (!ok) throw CException(_HERE_, "Could not read the hider position");

            _autoWalkerVec[i].set(row,col,i);
        }*/

        DEBUG_AUTOHIDER(cout<<" "<<i<<") "<<_autoWalkerVec[i].toString()<<endl;);
    }

    _stepNum++;

    return true;
}

bool FromListHider::initBelief(GMap* gmap, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    bool initB = AutoHider::initBelief(gmap,seekerInitPos,hiderInitPos,opponentVisible);

    assert(_map!=NULL);

    _stepNum = 0;
    //warning: assumed to be called after opening
    if (!_listFile->isOpen()) {
        throw CException(_HERE_, "The file is NOT open");
    }

    return initB;
}


std::string FromListHider::getName() const {
    return _name;
}

int FromListHider::getHiderType() const {
    return HSGlobalData::OPPONENT_TYPE_HIDER_FILE;
}

void FromListHider::setMap(GMap* map) {
    AutoHider::setMap(map);
}

GMap* FromListHider::getMap() const {
    return AutoHider::getMap();
}

SeekerHSParams* FromListHider::getParams() const {
    return _params;
}


void FromListHider::writePosFile(AutoWalker* autoWalker, GMap* map, string outFile, uint numHiders, uint numSteps, bool useContinuous) {
    DEBUG_CLIENT(cout<<"Writing positions file "<<outFile<<":"<<endl;);

    //ifstream reader;
    ofstream writer;
    writer.open(outFile);

    writer << map->getName() <<endl << numHiders <<endl <<numSteps<<endl;

    Pos seekerPos = map->genRandomPos();
    IDPos hiderPos(map->genRandomPos(),0);
    //init poses
    DEBUG_CLIENT(cout<<"Init with random seeker pos: "<<seekerPos.toString()<<", and hider pos: "<<hiderPos.toString()<<endl;);

    //init
    autoWalker->initBelief(map, seekerPos, hiderPos, false);

    //run
    for(uint i=0; i < numSteps; i++) {
        seekerPos = map->genRandomPos();
        hiderPos.set( map->genRandomPos() );
        //next poses of autowalkers
        vector<IDPos> posVec = autoWalker->getAllNextPos(seekerPos,hiderPos);
        bool isFirst = true;
        //write to file
        for(const IDPos& pos : posVec) {
            if (isFirst) {
                isFirst = false;
            } else {
                writer <<";";
            }
            if (useContinuous) {
                writer << pos.rowDouble() <<","<<pos.colDouble();
            } else {
                writer << pos.row() <<","<<pos.col();
            }
        }
        writer << endl;
    }

    writer.close();
}


uint FromListHider::getNumberOfHiders() const {
    return _numHiders;
}

uint FromListHider::getNumberOfSteps() const {
    return _numSteps;
}

