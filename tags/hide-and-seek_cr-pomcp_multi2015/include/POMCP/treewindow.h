#ifndef TREEWINDOW_H
#define TREEWINDOW_H

#include <QtWidgets/QMainWindow>
#include <vector>

class QTreeWidgetItem;
class QXmlStreamWriter;
class QXmlStreamReader;

class GameConnectorClient;

namespace pomcp {
class BaseNode;
class Node;
class NodeA;
}

namespace Ui {
class TreeWindow;
}



#include "Base/seekerhsparams.h"

/*!
 * \brief The TreeWindow class A debug window, showing debug output and the tree.
 */
class TreeWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    /*!
     * \brief instance a singleton, only generate one instance
     * \return
     */
    static TreeWindow* instance();

    explicit TreeWindow(QWidget *parent = 0);
    ~TreeWindow();
    
    /*!
     * \brief setParamValues set parameter values text
     * \param paramValuesStr
     */
    //void setParamValues(QString paramValuesStr);
    void setParams(SeekerHSParams* params, GameConnectorClient* gameConnector);

    /*!
     * \brief storeTreeAsXML
     * \param item
     * \param xmlFile
     */
    void storeTree(QTreeWidgetItem* item, QString xmlFile);

    /*!
     * \brief loadTree
     * \param xmlFile
     */
    void loadTree(QString xmlFile);


protected:
    void paintEvent(QPaintEvent *event);

public slots:
    /*!
     * \brief showTree show the POMCP tree
     * TODO: should be done online, sending an
     * \param root
     */
    void showTree(pomcp::Node* root, int a);


private slots:
    //void on_actionAdd_triggered();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_actionExpand_Until_This_triggered();

    //void on_actionCollapse_Until_This_triggered();

    void on_actionChosen_Action_triggered();

    void on_actionShow_Parameters_triggered();

    void on_actionExpand_All_triggered();

    void on_actionCollapse_All_triggered();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_actionSave_As_triggered();

    void on_actionOpen_triggered();

    void on_actionExit_triggered();

    void on_actionChange_Parameters_triggered();

    void on_actionClear_Memory_triggered();

private:
    //column indices in tree table
    static const int COL_NAME = 0;
    static const int COL_DEPTH = 1;
    static const int COL_VALUE = 2;
    static const int COL_COUNT = 3;
    static const int COL_DESCR_SHORT = 4;
    static const int COL_DESCR_LONG = 5;
    static const int COL_TREEDEPTH = 6;
    static const int NUM_COLS = COL_TREEDEPTH+1;

    //for stats
    int _maxTreeDepth;
    int _numNodesTree;
    int _numNodesGui;

    //! set value of a tree item
    void treeItemSetText(QTreeWidgetItem *itm, QString name, int depth, pomcp::BaseNode* node, QString shortDescr, QString longDescr, int treeDepth);

    //! generate tree
    void addToTree(pomcp::Node* node, QTreeWidgetItem* parent, int depth, int treeDepth);
    void addToTreeA(pomcp::NodeA* nodeA, QTreeWidgetItem* parent, int depth, int treeDepth);

    //! base description for a node
    QString getBaseNodeString(pomcp::BaseNode* node);

    //! creates item widget with name and descr, adds to parent and returns it
    QTreeWidgetItem* addItemToTree(QString name, QString descrShort, int depth, pomcp::BaseNode* node, QString descrLong, int treeDepth, QTreeWidgetItem* parent);

    //! return the (first) selected tree item, or the root if none is selected
    QTreeWidgetItem* getSelectedOrRoot();

    /*!
     * \brief treeExpandCollapse collapse/expands the tree based on the treedepth
     * \param itm
     * \param treeDepth tree depth 0=root
     * \param beforeExpand expand(true)/collaps(false) for all nodes with lower treedepth
     * \param afterExpand expand(true)/collaps(false) for all nodes with higher treedepth
     */
    void treeExpandCollapse(QTreeWidgetItem* itm, int treeDepth, bool beforeExpand, bool afterExpand);

    void storeSubTree(QXmlStreamWriter& writer, QTreeWidgetItem* itm);

    QTreeWidgetItem* loadSubTree(QXmlStreamReader& reader);

    void setQTreeWidgetItemColor(QTreeWidgetItem* item, QBrush& brush);


    Ui::TreeWindow *ui;

    QTreeWidgetItem *_rootItm;

    QString _chosenActStr;

    //QString _paramsValuesStr;

    SeekerHSParams* _params;

    GameConnectorClient* _gameConnector;

    static TreeWindow* _instance;

    //! list of all passed root nodes
    //std::vector<pomcp::Node*> _rootNodeVector;

    //! history of root items
    std::vector<QTreeWidgetItem*> _rootItmHistVector;
};

#endif // TREEWINDOW_H
