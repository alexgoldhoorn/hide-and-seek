#include "simmaingui.h"
#include "ui_simmaingui.h"





SimMainGUI::SimMainGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SimMainGUI)
{
    ui->setupUi(this);
}

SimMainGUI::~SimMainGUI()
{
    delete ui;
}
