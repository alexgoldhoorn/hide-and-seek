#include "GUI/hsclientmainwindow.h"
#include "ui_hsclientmainwindow.h"

#include <QMessageBox>
#include <iostream>

HSClientMainWindow::HSClientMainWindow(GMapWidget* child, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::HSClientMainWindow), _gmapWidget(child)

{
    ui->setupUi(this);
    //TestWidget* tw=new TestWidget(this);
    setCentralWidget(child);
    child->setMainWindow(this);
    child->resetWindowToMapSize();
    //TODO: set window in widget such that widget can set 'status' and title
}

HSClientMainWindow::~HSClientMainWindow()
{
    delete ui;
}

void HSClientMainWindow::on_action_Quit_triggered()
{
    //ui->statusbar->showMessage("TEST");
    if (QMessageBox::question(this,"Quit","Do you really want to quit?")==QMessageBox::Yes) {
        QCoreApplication::exit(0); //TODO: better way
    }
}

void HSClientMainWindow::setTitle(QString title) {
    setWindowTitle(title);
}

void HSClientMainWindow::setStatus(QString status) {
    ui->statusbar->showMessage(status);
}

/*TestWidget::TestWidget(QWidget *parent) : QWidget(parent){
    QPalette Pal(palette());
    // set black background
    Pal.setColor(QPalette::Background, Qt::blue);
    setAutoFillBackground(true);
    setPalette(Pal);
}
*/

void HSClientMainWindow::keyPressEvent(QKeyEvent *event) {
    //pass to widget
    _gmapWidget->keyPressEvent(event);
}


void HSClientMainWindow::resizeFactor(double f) {
    int barsHeight = ui->menubar->height()*2 + ui->statusbar->height(); //menubar 2: assumed to be height of title

    int w = (int)round(width()*f);
    int h = (int)round((height()-barsHeight)*f) + barsHeight;
    resize(w,h);
}

void HSClientMainWindow::resizeInternalWidget(int w, int h) {
    int barsHeight = ui->menubar->height()*2 + ui->statusbar->height(); //menubar 2: assumed to be height of title
    resize(w,h+barsHeight);
}



void HSClientMainWindow::on_actionSend_Stop_to_Server_triggered()
{
    _gmapWidget->sendStopToServer();
}
