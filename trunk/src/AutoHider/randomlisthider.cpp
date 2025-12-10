#include "AutoHider/randomlisthider.h"

#include "Utils/generic.h"
#include "Base/hsglobaldata.h"

#include <iostream>

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>


using namespace std;

RandomListHider::RandomListHider(SeekerHSParams* params, std::string actionFileName) : RandomHider(params)
{
    readActionFile(actionFileName);

    QString name = "ActionList_" + QString::fromStdString(_recActionListName);
    _name = name.toStdString();
}

RandomListHider::~RandomListHider() {
}

Pos RandomListHider::getNextPosRun(int actionDone, int *newAction) {
    //get next action
    int a = getNextActionFromList();

    if (a==-1) {
        //don't move
        cout << "RandomListHider::getNextPosRun: WARNING no action returned, not moving"<<endl;
        playerInfo.nextPos = playerInfo.currentPos;
    } else {
        //get next pos
        playerInfo.nextPos = _map->tryMove(a, playerInfo.currentPos);

        if (!playerInfo.nextPos.isSet()) {
            cout << "RandomListHider::getNextPosRun: WARNING incorrect action returned ("<<a<<"), not moving"<<endl;
            playerInfo.nextPos = playerInfo.currentPos;
        }
    }

    //set action
    if (newAction!=NULL) *newAction = a;

    return playerInfo.nextPos;
}

//AG120906: read action file
void RandomListHider::readActionFile(string actionFileName) {
    //load actions in list (first name is name, then each line an action id)
    QFile aFile(QString::fromStdString(actionFileName));

    if (aFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&aFile);
        if (in.atEnd()) {
            cout << "ERROR: Empty action file: "<<actionFileName<<endl;
            QCoreApplication::exit(-1);
        }
        //get name, first line
        _recActionListName = in.readLine().toStdString();

        if (in.atEnd()) {
            cout << "ERROR: Action file, 2nd line should be init pos (eg: 1,3): "<<actionFileName<<endl;
            QCoreApplication::exit(-1);
        }
        //now init pos
        QString initPosLine = in.readLine();
        QStringList posList = initPosLine.split(',');
        if (posList.size()!=2) {
            cout << "ERROR: Action file, 2nd line should be init pos (eg: 1,3): "<<actionFileName<<endl;
            QCoreApplication::exit(-1);
        }
        //convert init pos
        bool ok = false;
        int row = posList[0].toInt(&ok);
        int col = -1;
        if (ok) col = posList[1].toInt(&ok);
        if (!ok) {
            cout << "ERROR: Action file, 2nd line should be init pos, could not parse the numbers (eg: 1,3): "<<actionFileName<<endl;
            QCoreApplication::exit(-1);
        }
        _initActionListPos.set(row,col);

        DEBUG_HS(cout << "Initial position: "<<_initActionListPos.toString()<<endl;);
        DEBUG_HS(cout << "Actions from file: ";);

        if (in.atEnd()) {
            cout << "WARNING: Action file does not contain actions: "<<actionFileName<<endl;
        }
        while (!in.atEnd()) {
            bool ok = false;
            //get line with action
            QString s = in.readLine();
            if (s.isEmpty()) {
                //empty line
                cout << "Warning: empty line at line "<<_recActionList.size()<<endl;
                continue;
            }
            // get action
            int a = s.toInt(&ok);
            if (!ok) {
                cout << "Error: unknown action in file: "<<s.toStdString() <<" at line "<<_recActionList.size()<<endl;
                QCoreApplication::exit(-1);
            }
            if (a<0 || a>HSGlobalData::NUM_ACTIONS-1) {
                cout << "Error: unknown action in file: "<<s.toStdString() << " ("<<a<< ") at line "<<_recActionList.size()<<endl;
                QCoreApplication::exit(-1);
            }
            //put in list
            _recActionList.push_back(a);
            cout << a << ",";
        }
        cout << endl;
        aFile.close();
    }  else {
        cout << "ERROR: Action file could not be opened!"<<endl;
        QCoreApplication::exit(-1);
    }

    _recordActionI = 0;
}


//AG120903: get next action from list, -1 if no list or end
int RandomListHider::getNextActionFromList() {
    int a = -2;
    if (_recActionList.size()==0 || _recordActionI<0 || _recordActionI>=(int)_recActionList.size()) a=-1;

    a = _recActionList[_recordActionI];
    _recordActionI++;
    if (_recordActionI>=(int)_recActionList.size()) cout << "Warning: end of action list reached"<<endl;

    DEBUG_HS(cout << "** NEXT ACTION FROM LIST: "<< a <<endl;);

    return a;
}

string RandomListHider::getActionListName()  const {
    return _recActionListName;
}

//AG120911: get init actionlist pos
Pos RandomListHider::getInitPos() {
    return _initActionListPos;
}

std::string RandomListHider::getName() const {
    return _name;
}

int RandomListHider::getHiderType() const {
    return HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST;
}
