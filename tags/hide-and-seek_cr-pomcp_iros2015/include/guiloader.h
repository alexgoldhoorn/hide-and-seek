#ifndef GUILOADER_H
#define GUILOADER_H

#include <QObject>
#include <QMainWindow>
#include <QEvent>

class GuiLoader : public QObject
{
    Q_OBJECT
public:
    explicit GuiLoader(QMainWindow* window, QObject *parent = 0);

    virtual bool event( QEvent *ev );

signals:
    
public slots:
    
private:
    QMainWindow* _window;
};

#endif // GUILOADER_H
