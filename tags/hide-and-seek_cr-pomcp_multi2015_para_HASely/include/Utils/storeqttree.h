#ifndef STOREQTTREE_H
#define STOREQTTREE_H

#include <QObject>

class QTreeWidgetItem;

/*!
 * \brief The StoreQtTree class store qt tree items
 */
class StoreQtTree : public QObject
{
    Q_OBJECT
public:
    explicit StoreQtTree(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // STOREQTTREE_H
