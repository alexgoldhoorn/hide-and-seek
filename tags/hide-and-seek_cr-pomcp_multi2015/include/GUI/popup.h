#ifndef POPUP_H
#define POPUP_H



#include <QDialog>
#include <QComboBox>
#include<QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include "Base/seekerhsparams.h"

//class QCheckBox;
class QLabel;
class QErrorMessage;

class Dialog : public QDialog {
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);
    Dialog(SeekerHSParams* params, QWidget *parent = 0);
    Dialog(QString ipStr, QString usernameStr, int port, int mapID, int oppType,  bool isSeeker, QWidget *parent = 0);

    QString getIP() {
        return ip;
    }
    int getPort() {
        return port;
    }
    QString getUsername() {
        return usern;
    }

    int getMapIndex() {
        return combomap->currentIndex();
    }

    int getOpponentType() {
        return comboopp->currentIndex();
    }

    void setPort(int p) {
        por = QString::number(p);
    }

    void setIP(QString i) {
        ip=i;
    }

    void setusername(QString u) {
        usern = u;
    }

    bool isSeeker() {
        if (!role->isChecked())
            return 0;
        else
            return 1;
    }

    char getGameType();

    int getNumDynObst();
    /*void setTitle(QString t){
        _title =t;
    }*/

    //! set all params of the dialog in the params struct
    SeekerHSParams* getParams();

protected:
    static const QStringList OPPONENT_LIST;

    static const QStringList GAME_TYPE_LIST;

    static const QStringList AUTOWALKER_LIST;


    //create combobox with maps (using hsglobaldata::MAPS)
    void createComboBox();

private slots:
    void play();

    void closeAll();

    void loadMap();

private:

    QCheckBox *native;

    QLabel *itemLabel;


    QLineEdit *usrnm;

    QPushButton *Play;
    QPushButton *Leave;
    QPushButton *openMapButton;

    QLineEdit *ip_;
    QLineEdit *port_;
    QLineEdit *mapFileName;

    QString usern;
    QString ip;
    QString por;
    int port;
    QComboBox *combomap;
    QComboBox *comboopp;
    QComboBox *comboGameType;
    QComboBox *comboWalkerType;
    QCheckBox *role;
    QCheckBox *checkCont;


    QDoubleSpinBox *spinBoxNumDynobst;

    QDoubleSpinBox *spinBoxNumReqPlayers;
    QDoubleSpinBox *spinBoxObsProb;


    SeekerHSParams* _params;
};










#endif // POPUP_H
