#ifndef HSCLIENTMAINWINDOW_H
#define HSCLIENTMAINWINDOW_H

#include <QMainWindow>
#include "GUI/gmapwidget.h"

namespace Ui {
class HSClientMainWindow;
}

class HSClientMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HSClientMainWindow(GMapWidget* child, QWidget *parent = 0);
    ~HSClientMainWindow();

    void setTitle(QString title);
    void setStatus(QString status);

    void keyPressEvent ( QKeyEvent * event );

    //! resize window with a factor (1) being same size
    void resizeFactor(double f);

    //! resize intenal widget, and thereby whole window
    void resizeInternalWidget(int w, int h);

private slots:
    void on_action_Quit_triggered();

    void on_actionSend_Stop_to_Server_triggered();

private:
    Ui::HSClientMainWindow *ui;

    GMapWidget* _gmapWidget;


};

/*class TestWidget : public QWidget {
public:
    TestWidget(QWidget *parent = 0);
};*/

#endif // HSCLIENTMAINWINDOW_H
