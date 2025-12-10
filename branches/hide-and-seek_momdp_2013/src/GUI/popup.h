#ifndef POPUP_H
#define POPUP_H



#include <QDialog>
#include <QComboBox>
#include<QCheckBox>
#include <QPushButton>

//class QCheckBox;
class QLabel;
class QErrorMessage;

class Dialog : public QDialog
 {
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);
    Dialog( QString i, QString u,int p, int m, int o, bool r, QWidget *parent = 0);

    QString getip(){
        return ip;
    }
    int getport(){
        return port;
    }
    QString getusername(){
        return usern;
    }

    int getmap(){
       return combomap->currentIndex();
    }

    int getopp(){
       return comboopp->currentIndex();
    }

    void setport(QString p){
        por = p;
    }

    void setip(QString i){
        ip=i;
    }

    void setusername(QString u){
        usern = u;
    }

    bool getType(){
       if (!role->isChecked())
        return 0;
       else
           return 1;
    }
    /*void setTitle(QString t){
        _title =t;
    }*/


private slots:
       void play();

private:

    QCheckBox *native;

    QLabel *itemLabel;


    QLineEdit *usrnm;

    QPushButton *Play;
    QPushButton *Leave;

    QLineEdit *ip_;
    QLineEdit *port_;

    QString usern;
    QString ip;
    QString por;
    int port;
    QComboBox *combomap;
    QComboBox *comboopp;
    QCheckBox *role;


};










#endif // POPUP_H
