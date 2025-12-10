
#include <QtGui>


#include "popup.h"
#include "gmapwidget.h"
#include <iostream>
#include <QStringList>

#include "hsglobaldata.h"

using namespace std;


#define MESSAGE \
    Dialog::tr("<p>Message boxes have a caption, a text, " \
               "and any number of buttons, each with standard or custom texts." \
               "<p>Click a button to close the message box. Pressing the Esc button " \
               "will activate the detected escape button (if any).")

/*
QStringList GWorld::MAPS = QStringList() <<
                           "map1_6x5_0o.txt"<<"map2_6x5_1o.txt"<<"map3_6x5_2o.txt"
                           <<"map4_6x5_2o.txt"<<"map5_6x5_4o.txt"<< "map1_40x40_1o.txt"
                           << "map2_40x40_2o.txt"<< "map3_40x40_3o.txt"<< "map4_40x40_4o.txt"
                           << "map5_40x40_5o.txt"<< "testmapbig1.txt" << "testmapbig2.txt"
                           << "map1_10x10_1o.txt" << "map2_10x10_2o.txt" << "map3_10x10_3o.txt"
                           << "map4_10x10_4o.txt" << "map5_10x10.txt" << "map6_10x10.txt"
                           << "map7_10x10.txt"  << "mapbcn1.txt" << "mapbcn2.txt"
                           << "mapbcn3.txt" << "mapbcn1a.txt" << "mapbcn1b.txt"
                           << "mapbcn1c.txt" << "map1_20x20_0o.txt" << "map2_20x20_1o.txt"
                           << "map3_20x20_2o.txt" << "map4_20x20_3o.txt" << "map5_20x20_4o.txt";
*/ //TODO: UPDATE each time one added
/*QStringList MAPS_LIST  = QStringList() << "(6x5) no obstacle" << "(6x5) 1 obstacle" << "(6x5) 2 obstacles"
                     << "(6x5) 2 obstacles" << "(6x5) 4 obstacles"<< "(40x40) 1 obstacle"
                     << "(40x40) 2 obstacles"<< "(40x40) 3 obstacles"<< "(40x40) 4 obstacles"
                     << "(40x40) 5 obstacles"<< "big map 1 ()" << "big map 2 (19x13)"
                     << "(10x10) 1 obstacle" << "(10x10) 2 obstacles"<< "(10x10) 3 obstacles"
                     << "(10x10) 4 obstacles"<< "(10x10) easy for seeker 1" << "(10x10) easy for seeker 2"
                     << "(10x10) easy for hider" << "bcn 1" << "bcn 2"
                     << "bcn 3" << "bcn 1a" << "bcn 1b"
                     << "bcn 1c" << "(20x20) no obstacles" << "(20x20) 1 obstacle"
                     << "(20x20) 2 obstacles" << "(20x20) 3 obstacles" << "(20x20) 4 obstacles";*/

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
 {
    //_title="Hide&Seek";
     int frameStyle = QFrame::Sunken | QFrame::Panel;

     port=0;



     combomap = new QComboBox();
     /*QStringList maps;
     maps << MAPS_LIST;*/ //ag130508
     combomap->addItems(HSGlobalData::MAPS); //maps);
     combomap->setCurrentIndex(0);

     role = new QCheckBox(this);
     role->setText("check to be the SEEKER");
     role->setChecked(true);

     //bool ok;
    QLabel *username = new QLabel(tr("Player Name:"));
    username->setFrameStyle(frameStyle);
    usrnm = new QLineEdit();
    usrnm->insert(usern);
    //usrnm
    //usnam->getText(this, tr("QInputDialog::getText()"),tr("User name:"), QLineEdit::Normal, QDir::home().dirName(), &ok);
    QLabel *insertip = new QLabel(tr("Insert ip:"));
    insertip->setFrameStyle(frameStyle);
    ip_ = new QLineEdit();


    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
         if (ipAddressesList.at(i) != QHostAddress::LocalHost && ipAddressesList.at(i).toIPv4Address()) {
                 ipAddress = ipAddressesList.at(i).toString();
                 break;
                }
        }
        // if we did not find one, use IPv4 localhost
        if (ipAddress.isEmpty())
                ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    ipAddress = "localhost";
    por = "1120";
    ip_->insert(ipAddress);
    QLabel *insertport = new QLabel(tr("Port:"));
    insertport->setFrameStyle(frameStyle);
    port_ = new QLineEdit();
    port_->insert(por);
    usrnm->insert(usern);

    /*connect(itemButton, SIGNAL(clicked()), this, SLOT(setItem()));
    connect(textButton, SIGNAL(clicked()), this, SLOT(setText()));

    connect(criticalButton, SIGNAL(clicked()), this, SLOT(criticalMessage()));*/

            /*connect(usrnm, SIGNAL(textChanged(QString)), this, SLOT(textchanged()));
            connect(ip_, SIGNAL(textChanged(QString)), this, SLOT(textchanged()));

            connect(port_, SIGNAL(textChanged(QString)), this, SLOT(textchanged()));*/


    comboopp = new QComboBox();
    QStringList opponents;
    //opponents << "Human" << "Hider1" << "Hider2" << "SeekroBot";
    /*static const int OPPONENT_TYPE_HUMAN = 0;
    static const int OPPONENT_TYPE_HIDER_RANDOM = 1;
    static const int OPPONENT_TYPE_HIDER_SMART = 2;
    static const int OPPONENT_TYPE_HIDER_ACTION_LIST = 3;
    static const int OPPONENT_TYPE_SEEKER = 4;*/
    opponents << "Human" << "Random Hider" << "Smart Hider" << "Hider Action List" << "Seeker";
    comboopp->addItems(opponents);
    comboopp->setCurrentIndex(0);

    Play = new QPushButton;
    Play->setText("PLAY");
    connect(Play,SIGNAL(clicked()),this, SLOT(play()));
    Leave = new QPushButton;
    Leave->setText("Quit");
    connect(Leave,SIGNAL(clicked()),this, SLOT(close()));


    QGridLayout *layout = new QGridLayout;
    layout->setColumnStretch(1, 1);
    layout->setColumnMinimumWidth(1, 150);
    layout->addWidget(insertip, 0, 0);
    layout->addWidget(ip_, 0, 1);
    layout->addWidget(insertport, 1, 0);
    layout->addWidget(port_, 1, 1);
    layout->addWidget(username, 2, 0);
    layout->addWidget(usrnm, 2, 1);
    layout->addWidget(combomap, 3, 0);
    layout->addWidget(comboopp, 3, 1);
    layout->addWidget(role, 4, 1);
    layout->addWidget(Play, 6, 0);
    layout->addWidget(Leave, 6, 1);



    setLayout(layout);

    //setWindowTitle("poios?");
}




Dialog::Dialog(QString i, QString u, int p, int m, int o,  bool r, QWidget *parent): QDialog(parent) {



    int frameStyle = QFrame::Sunken | QFrame::Panel;



    QLabel *insertip = new QLabel(tr("Insert ip:"));
    insertip->setFrameStyle(frameStyle);
    ip_ = new QLineEdit();
    ip_->insert(i);

    QLabel *insertport = new QLabel(tr("Port:"));
    insertport->setFrameStyle(frameStyle);
    port_ = new QLineEdit();
    port_->insert(QString::number(p));

    QLabel *username = new QLabel(tr("Player Name:"));
    username->setFrameStyle(frameStyle);
    usrnm = new QLineEdit();
    usrnm->insert(u);

    combomap = new QComboBox();
    QStringList maps;
    maps << "Map1: (6x5)no obstacles" << "Map2: (6x5)1 obstacle" << "Map3: (6x5)2 obstacles" << "Map4: (6x5)2 obstacles" << "Map5: (6x5)4 obstacles"<< "Map6: (40x40) 1 obstacle"<< "Map7: (40x40)2 obstacles"<< "Map8: (40x40)3 obstacles"<< "Map9: (40x40)4 obstacles"<< "Map10: (40x40)5 obstacles";
    combomap->addItems(maps);
    combomap->setCurrentIndex(m);

   comboopp = new QComboBox();
   QStringList opponents;
   //opponents << "Human" << "Hider1" << "Hider2" << "SeekroBot";
   //AG120904: updated list
   /*static const int OPPONENT_TYPE_HUMAN = 0;
   static const int OPPONENT_TYPE_HIDER_RANDOM = 1;
   static const int OPPONENT_TYPE_HIDER_SMART = 2;
   static const int OPPONENT_TYPE_HIDER_ACTION_LIST = 3;
   static const int OPPONENT_TYPE_SEEKER = 4;*/
   opponents << "Human" << "Random Hider" << "Smart Hider" << "Hider Action List" << "Seeker";
   comboopp->addItems(opponents);
   comboopp->setCurrentIndex(o);

   role = new QCheckBox(this);
   role->setText("check to be the SEEKER");
   role->setChecked(r);

   Play = new QPushButton;
   Play->setText("REPLAY");
   connect(Play,SIGNAL(clicked()),this, SLOT(play()));

   Leave = new QPushButton;
   Leave->setText("Quit");
   connect(Leave,SIGNAL(clicked()),this, SLOT(close()));


   QGridLayout *layout = new QGridLayout;
   layout->setColumnStretch(1, 1);
   layout->setColumnMinimumWidth(1, 150);
   layout->addWidget(insertip, 0, 0);
   layout->addWidget(ip_, 0, 1);
   layout->addWidget(insertport, 1, 0);
   layout->addWidget(port_, 1, 1);
   layout->addWidget(username, 2, 0);
   layout->addWidget(usrnm, 2, 1);
   layout->addWidget(combomap, 3, 0);
   layout->addWidget(comboopp, 3, 1);
   layout->addWidget(role, 4, 1);
   layout->addWidget(Play, 6, 0);
   layout->addWidget(Leave, 6, 1);



   setLayout(layout);

   //setWindowTitle("_title");

    }




void Dialog::play()
{
    usern = usrnm->text();
    ip = ip_->text();
    por = port_->text();
    port = port_->text().toInt();
    //cout<<ip.toStdString()<<" : "<<port<<" : "<<usern.toStdString()<<endl;
    this->accept();

}



