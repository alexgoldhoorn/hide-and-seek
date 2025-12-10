#ifndef SIMMAINGUI_H
#define SIMMAINGUI_H

#include <QMainWindow>

namespace Ui {
    class SimMainGUI;
}

class SimMainGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimMainGUI(QWidget *parent = 0);
    ~SimMainGUI();

private:
    Ui::SimMainGUI *ui;
};

#endif // SIMMAINGUI_H
