
#include <QtWidgets>
#include <QStringList>

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>

#include <iostream>
#include <cassert>

#include "GUI/popup.h"
#include "GUI/gmapwidget.h"
#include "Base/hsglobaldata.h"

using namespace std;

//TODO: cleanup


const QStringList Dialog::OPPONENT_LIST = QStringList()
                                            << "Human" << "Random Hider" << "Smart Hider" << "Hider Action List" << "Seeker"
                                            << "Very Smart Hider" << "All Knowing Smart Hider"
                                            << "All Knowing Very Smart Hider" << "Very Smart Hider v2" << "All Knowing Very Smart Hider v2"
                                            << "Random Walker";

const QStringList Dialog::GAME_TYPE_LIST = QStringList() << "Hide-and-Seek" << "Find-and-Follow" << "Find-and-Follow 2 seekers";

const QStringList Dialog::AUTOWALKER_LIST = QStringList() << "Random" << "Random Goal";



Dialog::Dialog(QWidget *parent) : Dialog("localhost", "test", 1120, 0, 0, true, parent) {
}

Dialog::Dialog(SeekerHSParams *params, QWidget *parent) :
    Dialog(QString::fromStdString(params->serverIP), QString::fromStdString(params->userName), params->serverPort,
           params->mapID, params->opponentType, params->isSeeker, parent) {

    assert(params!=NULL);
    _params = params;

    comboGameType->setCurrentIndex((int)_params->gameType);
    spinBoxNumDynobst->setValue(_params->autoWalkerN);
    mapFileName->setText(QString::fromStdString(_params->mapFile));
    checkCont->setChecked(_params->useContinuousPos);

    switch(_params->autoWalkerType ) {
        case HSGlobalData::AUTOWALKER_RANDOM:
            comboWalkerType->setCurrentIndex(0);
            break;
        case HSGlobalData::AUTOWALKER_RANDOM_GOAL:
            comboWalkerType->setCurrentIndex(1);
            break;
        default:
            comboWalkerType->setCurrentIndex(1);
            break;
    }

}


Dialog::Dialog(QString ipStr, QString usernameStr, int port, int mapID, int oppType,  bool isSeeker, QWidget *parent): QDialog(parent) {
    _params = NULL;

    int frameStyle = QFrame::Sunken | QFrame::Panel;

    QLabel *insertip = new QLabel(tr("Server IP:"));
    insertip->setFrameStyle(frameStyle);
    ip_ = new QLineEdit();
    ip_->insert(ipStr);

    QLabel *insertport = new QLabel(tr("Server Port:"));
    insertport->setFrameStyle(frameStyle);
    port_ = new QLineEdit();
    port_->insert(QString::number(port));

    QLabel *username = new QLabel(tr("Player Name:"));
    username->setFrameStyle(frameStyle);
    usrnm = new QLineEdit();
    usrnm->insert(usernameStr);

    QLabel *mapFileLabel = new QLabel(tr("Map File:"));
    mapFileLabel->setFrameStyle(frameStyle);
    mapFileName = new QLineEdit();

    createComboBox();
    if (mapID>=0 && mapID<HSGlobalData::MAPS.size()) {
        combomap->setCurrentIndex(mapID);
    } else {
        combomap->setCurrentIndex(0);
    }

    comboopp = new QComboBox();
    comboopp->addItems(OPPONENT_LIST);
    if (oppType>=0 && oppType<OPPONENT_LIST.size()) {
        comboopp->setCurrentIndex(oppType);
    } else {
        comboopp->setCurrentIndex(0);
    }

    comboGameType = new QComboBox();
    comboGameType->addItems(GAME_TYPE_LIST);
    comboGameType->setCurrentIndex(1);

    comboWalkerType = new QComboBox();
    comboWalkerType->addItems(AUTOWALKER_LIST);
    comboWalkerType->setCurrentIndex(1);

    role = new QCheckBox(this);
    role->setText("Play as a Seeker");
    role->setChecked(isSeeker);

    checkCont = new QCheckBox(this);
    checkCont->setText("Continuous");
    checkCont->setChecked(true);

    Play = new QPushButton;
    Play->setText("PLAY");
    connect(Play,SIGNAL(clicked()),this, SLOT(play()));

    Leave = new QPushButton;
    Leave->setText("Quit");
    connect(Leave,SIGNAL(clicked()),this, SLOT(closeAll()));

    openMapButton = new QPushButton;
    openMapButton->setText("Open map file...");
    connect(openMapButton,SIGNAL(clicked()),this, SLOT(loadMap()));

    spinBoxNumDynobst = new QDoubleSpinBox();
    spinBoxNumDynobst->setRange(0,100);
    spinBoxNumDynobst->setDecimals(0);
    spinBoxNumDynobst->setSingleStep(1);
    if (_params==NULL)
        spinBoxNumDynobst->setValue(0);
    else
        spinBoxNumDynobst->setValue(_params->autoWalkerN);

    spinBoxNumReqPlayers = new QDoubleSpinBox();
    spinBoxNumReqPlayers->setRange(2,100);
    spinBoxNumReqPlayers->setDecimals(0);
    spinBoxNumReqPlayers->setSingleStep(1);
    if (_params==NULL)
        spinBoxNumReqPlayers->setValue(2);
    else
        spinBoxNumReqPlayers->setValue(_params->numPlayersReq);

    spinBoxObsProb = new QDoubleSpinBox();
    spinBoxObsProb->setRange(0,1);
    spinBoxObsProb->setDecimals(1);
    spinBoxObsProb->setSingleStep(0.1);
    if (_params==NULL || _params->multiSeekerOwnObsChooseProb==0)
        spinBoxObsProb->setValue(1);
    else
        spinBoxObsProb->setValue(_params->multiSeekerOwnObsChooseProb);

    QLabel* mapLabel = new QLabel(tr("Map:"));
    mapLabel->setFrameStyle(frameStyle);
    QLabel* oppLabel = new QLabel(tr("Opponent:"));
    oppLabel->setFrameStyle(frameStyle);
    QLabel* gameTypeLabel = new QLabel(tr("Game Type:"));
    gameTypeLabel->setFrameStyle(frameStyle);
    QLabel* dynObstLabel = new QLabel(tr("Number of dyn. obstacles:"));
    dynObstLabel->setFrameStyle(frameStyle);
    QLabel* autoWalkTypeLabel = new QLabel(tr("Dyn. obstacle type:"));
    autoWalkTypeLabel->setFrameStyle(frameStyle);
    QLabel* numReqPlLabel = new QLabel(tr("Number req. players:"));
    numReqPlLabel->setFrameStyle(frameStyle);
    QLabel* obsProbLabel = new QLabel(tr("Observ. probability:"));
    obsProbLabel->setFrameStyle(frameStyle);

    QGridLayout *layout = new QGridLayout;
    int i = 0;
    layout->setColumnStretch(1, 1);
    layout->setColumnMinimumWidth(1, 150);
    layout->addWidget(insertip, i, 0);
    layout->addWidget(ip_, i, 1);
    layout->addWidget(insertport, ++i, 0);
    layout->addWidget(port_, i, 1);
    layout->addWidget(username, ++i, 0);
    layout->addWidget(usrnm, i, 1);
    layout->addWidget(mapLabel, ++i, 0);
    layout->addWidget(combomap, i, 1);
    layout->addWidget(oppLabel, ++i, 0);
    layout->addWidget(comboopp, i, 1);
    layout->addWidget(gameTypeLabel, ++i, 0);
    layout->addWidget(comboGameType, i, 1);
    layout->addWidget(numReqPlLabel, ++i, 0);
    layout->addWidget(spinBoxNumReqPlayers, i, 1);
    layout->addWidget(obsProbLabel, ++i, 0);
    layout->addWidget(spinBoxObsProb, i, 1);
    layout->addWidget(autoWalkTypeLabel, ++i, 0);
    layout->addWidget(comboWalkerType, i, 1);
    layout->addWidget(dynObstLabel, ++i, 0);
    layout->addWidget(spinBoxNumDynobst, i, 1);
    layout->addWidget(mapFileLabel, ++i, 0);
    layout->addWidget(mapFileName, i, 1);
    layout->addWidget(openMapButton, ++i, 1);
    layout->addWidget(checkCont, ++i, 0);
    layout->addWidget(role, i++, 1);
    layout->addWidget(Play,++i, 0);
    layout->addWidget(Leave,i, 1);

    setLayout(layout);

    //setWindowTitle("_title");

}




void Dialog::play() {
    usern = usrnm->text();
    ip = ip_->text();
    por = port_->text();
    port = port_->text().toInt();
    this->accept();

}

void Dialog::closeAll() {
    exit(0);
}

void Dialog::loadMap() {
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Map"), "", tr("Map Text Files (*.txt *.map);;Image Map File (*.pgm);;All files (*.*)"));

    if (!fileName.isEmpty()) {
        mapFileName->setText(fileName);
    }
}

void Dialog::createComboBox() {
    //ag131125: create map list
    combomap = new QComboBox();

    for (QStringList::ConstIterator it = HSGlobalData::MAPS.constBegin(); it != HSGlobalData::MAPS.constEnd(); it++) {
        QString itmStr = (*it);
        if (itmStr.length()>4) {
            itmStr = itmStr.left(itmStr.length()-4);
        }

        itmStr.replace("_"," ");

        combomap->addItem(itmStr);
    }
}


char Dialog::getGameType() {
    return (char)comboGameType->currentIndex();
}

int Dialog::getNumDynObst() {
    return (int)spinBoxNumDynobst->value();
}

SeekerHSParams* Dialog::getParams() {
    if(_params==NULL) _params = new SeekerHSParams;

    _params->serverIP = getIP().toStdString();
    _params->serverPort = getPort();
    _params->mapID = getMapIndex();
    _params->isSeeker = isSeeker();
    _params->opponentType = getOpponentType();
    _params->gameType = getGameType();
    _params->autoWalkerN = getNumDynObst();    
    _params->mapFile = mapFileName->text().toStdString();
    _params->useContinuousPos = checkCont->isChecked();

    switch(comboWalkerType->currentIndex()) {
        case 0:
            _params->autoWalkerType =  HSGlobalData::AUTOWALKER_RANDOM;
            break;
        case 1:
            _params->autoWalkerType =  HSGlobalData::AUTOWALKER_RANDOM_GOAL;
            break;
        default:
            cout << "WARNING: unknown walker type comob index: "<<comboWalkerType->currentIndex()<<endl;
            break;
    }

    _params->numPlayersReq = (unsigned int)spinBoxNumReqPlayers->value();
    _params->multiSeekerOwnObsChooseProb = spinBoxObsProb->value();

    return _params;
}

