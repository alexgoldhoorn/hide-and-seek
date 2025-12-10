#include "POMCP/treewindow.h"
#include "ui_treewindow.h"

#include "POMCP/node.h"
#include "Base/hsglobaldata.h"

#include <iostream>
#include <cassert>

#include <QFile>
#include <QXmlStreamWriter>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

#include <sstream>

#include "Base/gameconnectorclient.h"
#include "GUI/setparamsdialog.h"


using namespace pomcp;
using namespace std;

TreeWindow* TreeWindow::_instance = NULL;


TreeWindow::TreeWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TreeWindow)
{
    ui->setupUi(this);

    //set header width
    ui->treeWidget->header()->resizeSection(0,250);
    ui->treeWidget->header()->resizeSection(1,60);
    ui->treeWidget->header()->resizeSection(2,70);
    ui->treeWidget->header()->resizeSection(3,70);
    ui->treeWidget->header()->resizeSection(4,300);

    //_paramsValuesStr = "?";
    _params = NULL;
    _chosenActStr = "?";
    _rootItm = NULL;

}

TreeWindow::~TreeWindow()
{
    //cout << "destructor of treewindow"<<endl;
    delete ui; //cout << " END DESTRUCTOR treewindow"<<endl;
}

void TreeWindow::paintEvent(QPaintEvent *event) {
    ui->verticalWidget->resize(ui->centralwidget->size());
}



void TreeWindow::showTree(pomcp::Node *root, int a) {
    cout << "TreeWindow::showTree: "<<flush;

    //remove all children
    /*if (_rootItm!=NULL) {
        cout << "deleting previous (gui)tree: "<<flush;
        delete _rootItm;
        cout << "gui tree deleted"<<flush;
    }*/
    if (_rootItm!=NULL) {
        _rootItmHistVector.push_back(_rootItm);
    }

    cout <<". creating gui tree..."<<endl;    

    _rootItm = new QTreeWidgetItem();


    //set text for new root
    QString text = getBaseNodeString(root);
    QString shortDescr = QString::fromStdString(root->toString());
    QString longDescr = shortDescr + "\n"
            + QString::fromStdString(root->toString()) + "\n"
            //+ QString::fromStdString(root->getBelief()->getMapHistString(_gameConnector->getGMap())) + "\n"
            + QString::fromStdString(root->getBelief()->toString());

    _maxTreeDepth = 0;
    _numNodesTree = _numNodesGui = 0;

    treeItemSetText(_rootItm, text, 0, root, shortDescr, longDescr, 0);

    //generate tree
    addToTree(root,_rootItm, 0, 0);

    ui->treeWidget->addTopLevelItem(_rootItm);

    //remove subtree
    if (ui->treeWidget->topLevelItemCount()>1) {
        ui->treeWidget->takeTopLevelItem(0);
    }

    //set additional text info
    _chosenActStr = "Chosen action: "+HSGlobalData::ACTIONS_SHORT[a]+ " (" +QString::number(a)+")";
    ui->statusbar->showMessage(_chosenActStr);

    _chosenActStr  +=  "\nMaximum tree depth: " + QString::number(_maxTreeDepth) +"\n"
            + "Number of nodes (in tree): " + QString::number(_numNodesTree) +"\n"
            + "Number of nodes (in GUI): " + QString::number(_numNodesGui) +" (contain non-existent action nodes)\n"
            + "Last node ID: " + QString::number(BaseNode::generatedNodeCount()-1)+"\n"
            + "Root: " + QString::fromStdString(root->toString());

    treeExpandCollapse(_rootItm,1,true,false);
    ui->textEdit->setText(_chosenActStr);

    cout << "gui tree created"<<endl;
    //AG140307: add to list of nodes
    //_rootNodeVector.push_back(root);
}

void TreeWindow::treeItemSetText(QTreeWidgetItem *itm, QString name, int depth, BaseNode* node, QString descrShort, QString descrLong, int treeDepth) {
    //we've used one node more

    itm->setText(COL_NAME,name);
    itm->setText(COL_DEPTH,QString::number(depth));
    itm->setText(COL_DESCR_SHORT,descrShort);

    QString extraDescr = "Depth = "+QString::number(depth)+"; Tree depth = "+QString::number(treeDepth)+"\n"+"----------\n";
    QString valStr, countStr;

    if (node != NULL) {
        extraDescr = "ID="+QString::number(node->getNodeID())+"\n"+
                "Value = "+QString::number(node->getValue())+" (#"+QString::number(node->getCount())+")\n"+extraDescr;
        valStr = QString::number(node->getValue());
        countStr = QString::number(node->getCount());

        _numNodesTree++;
        _numNodesGui++;
    } else {
        extraDescr = "";
        valStr = "-";
        countStr = "0";

        _numNodesGui++;
    }

    descrLong = extraDescr + descrLong;
    itm->setText(COL_DESCR_LONG,descrLong);
    itm->setText(COL_VALUE, valStr);
    itm->setText(COL_COUNT, countStr);
    itm->setText(COL_TREEDEPTH, QString::number(treeDepth));

    if (node == NULL || node->getCount()==0 || node->getValue()==0) {
        //set colors
        QBrush b;
        if (node==NULL || node->getCount()==0) {
            b.setColor(Qt::lightGray);
        } else {
            b.setColor(Qt::darkGray);
        }

        setQTreeWidgetItemColor(itm,b);

    }
    itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled );
}

void TreeWindow::setQTreeWidgetItemColor(QTreeWidgetItem *itm, QBrush &brush)  {
    for(int i=0;i<NUM_COLS;i++) {
        itm->setForeground(i,brush);
        //itm->setBackground(i,brush);
    }
}

QTreeWidgetItem* TreeWindow::addItemToTree(QString name, QString descrShort, int depth,
                                            BaseNode* node, QString descrLong, int treeDepth, QTreeWidgetItem *parent) {

    QTreeWidgetItem *itm = new QTreeWidgetItem();
    treeItemSetText(itm,name,depth,node,descrShort,descrLong,treeDepth);
    itm->setExpanded(true);
    parent->addChild(itm);
    parent->setExpanded(true);
    return itm;
}

QString TreeWindow::getBaseNodeString(BaseNode *node) {
    QString str;
    if (node!=NULL)
        str = "[" + QString::number(node->getNodeID()) + "] v=" + QString::number(node->getValue(),'f',3)
            + " (#" +  QString::number(node->getCount())
#ifdef DEBUG_POMCP_LAST_VALUE
                + ", lastV=" + QString::number(node->getLastValue())
#endif
                + ")";
    else
        str = "[-] v=? (#0)";
    return str;
}

void TreeWindow::addToTree(pomcp::Node *node, QTreeWidgetItem *parent, int depth, int treeDepth) {
    vector<QTreeWidgetItem*> itmMaxVec;
    double maxVal = HSGlobalData::INFTY_NEG_DBL;

    //tree depth
    if (treeDepth>_maxTreeDepth) {
        _maxTreeDepth = treeDepth;
    }

    //add children to tree
    for(size_t a=0;a<node->childCount(); a++) {
        NodeA *nodeA = node->getChild(a);

        QString name;
        QString shortDescr;
        if (nodeA!=NULL) {
            name = getBaseNodeString(nodeA);
            shortDescr = QString::fromStdString(nodeA->toString());
        } else {
            name = "[-]";
            shortDescr = "Action not possible";
        }
        name += " action " + HSGlobalData::ACTIONS_SHORT[a] + " ("+QString::number(a)+")";
        name = "A" + HSGlobalData::ACTIONS_SHORT[a] + name;        
        QString descr = name + "\n" + shortDescr;


        QTreeWidgetItem *itm = addItemToTree(name,shortDescr,depth+1,nodeA,descr,treeDepth+1,parent);

        if (nodeA!=NULL) {
            //add as child
            if (nodeA->childCount()>0) {
                addToTreeA(nodeA, itm,depth+1,treeDepth+1);
            }

            if (nodeA->getCount()>0) {
                //check max
                double v = nodeA->getValue();
                if (v>=maxVal) {
                    if (v>maxVal) {
                        itmMaxVec.clear();
                    }
                    itmMaxVec.push_back(itm);
                    maxVal = v;
                }
            }
        }
    } // for

    //set color
    for(vector<int>::size_type i=0;i<itmMaxVec.size();i++) {
        QTreeWidgetItem* itm = itmMaxVec[i];
        QBrush b(Qt::darkGreen);
        setQTreeWidgetItemColor(itm,b);
    }
}

void TreeWindow::addToTreeA(NodeA *nodeA, QTreeWidgetItem *parent, int depth, int treeDepth) {
    for(size_t o=0;o<nodeA->childCount(); o++) {
        ObsNodePair* obsNodePair = nodeA->getChildItem(o);
        pomcp::Node *node = obsNodePair->node;
        assert(node!=NULL); //because we only iterate through obs that are set

        QString name = "O"+getBaseNodeString(node);
        name += " obs " + QString::fromStdString(obsNodePair->observation->toString());
        QString shortDescr = QString::fromStdString(node->toString());
        QString descr = name + "\n"
                        + shortDescr + "\n"
                        //+ QString::fromStdString(node->getBelief()->getMapHistString(_gameConnector->getGMap())) + "\n"
                        + QString::fromStdString(node->getBelief()->toString());

        QTreeWidgetItem *itm = addItemToTree(name,shortDescr,depth,node,descr,treeDepth+1,parent);

        if (node->childCount()>0) {
            addToTree(node, itm,depth,treeDepth+1);
        }
    }
}



void TreeWindow::setParams(SeekerHSParams *params, GameConnectorClient *gameConnector) {
    assert(params!=NULL);
    assert(gameConnector!=NULL);
    _params = params;
    _gameConnector = gameConnector;
}


void TreeWindow::storeTree(QTreeWidgetItem *item, QString xmlFile) {
    assert(item!=NULL);

    QFile file(xmlFile);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(0, "Read only", "The file is in read only mode");
        return;
    }

    QXmlStreamWriter writer(&file);
    writer.writeStartDocument();
    writer.writeStartElement("QTreeWidgetItems");

    storeSubTree(writer, item);

    writer.writeEndElement();
    writer.writeEndDocument();
    file.close();
}

void TreeWindow::storeSubTree(QXmlStreamWriter &writer, QTreeWidgetItem *itm) {
    writer.writeStartElement("node");
    //columns
    writer.writeStartElement("columns");
    for(int i=0; i<itm->columnCount(); i++) {
        writer.writeStartElement("column");
        writer.writeCharacters(itm->data(i,0).toString());
        writer.writeEndElement();
    }
    writer.writeEndElement();

    //children
    writer.writeStartElement("children");
    for(int i=0;i<itm->childCount();i++) {
        storeSubTree(writer,itm->child(i));
    }
    writer.writeEndElement();

    writer.writeEndElement();
}

void TreeWindow::loadTree(QString xmlFile)  {
    QFile file(xmlFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(0, "Read only", "The file is in read only mode");
        return;
    }

    QXmlStreamReader reader(&file);

    //QXmlStreamReader::TokenType token = reader.readNext();
    QTreeWidgetItem* newRoot = NULL;

    //load tree from XML
    while(!reader.atEnd() && !reader.hasError()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = reader.readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {

            if(reader.name() == "QTreeWidgetItems") {
                continue;
            }

            if(reader.name() == "node") {
                assert(newRoot==NULL);
                newRoot = loadSubTree(reader);
            }
        }
    }
    /* Error handling. */
    if(reader.hasError()) {
        QMessageBox::critical(this,
                              "QXSRExample::parseXML",
                              reader.errorString(),
                              QMessageBox::Ok);
    }

    /* Removes any device() or data from the reader
     * and resets its internal state to the initial state. */
    reader.clear();

    //load tree


    file.close();

    //now set new root, and remove old one
    ui->treeWidget->addTopLevelItem(newRoot);

    if (ui->treeWidget->topLevelItemCount()>1) {
        ui->treeWidget->takeTopLevelItem(0);
    }

    _rootItm = newRoot;
}


QTreeWidgetItem* TreeWindow::loadSubTree(QXmlStreamReader &reader) {
    //generate item
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled );
    item->setExpanded(true);

    QXmlStreamReader::TokenType ntoken = QXmlStreamReader::NoToken;

    while(!reader.atEnd() && !reader.hasError() && ntoken!=QXmlStreamReader::EndElement) {
        /* Read next element.*/
        ntoken = reader.readNext();

        if(ntoken == QXmlStreamReader::StartElement) {

            //read all columns
            if(reader.name() == "columns") {
                int col = 0;
                QXmlStreamReader::TokenType n2token = QXmlStreamReader::NoToken;

                while(!reader.atEnd() && !reader.hasError() && n2token!=QXmlStreamReader::EndElement) {
                    n2token = reader.readNext();
                    if(ntoken == QXmlStreamReader::StartElement && reader.name() == "column") {
                        //set column in this node
                        item->setText(col, reader.readElementText());
                        col++;
                    }
                } //while for each col

                //check value to set color
                QString cntStr = item->text(COL_COUNT);
                if (cntStr=="-" || cntStr=="0") {
                    QBrush b(Qt::lightGray);
                    setQTreeWidgetItemColor(item,b);
                }

                cout <<endl;

            } else if(reader.name() == "children") {
                QXmlStreamReader::TokenType n2token = QXmlStreamReader::NoToken;

                //read all children
                while(!reader.atEnd() && !reader.hasError() && n2token!=QXmlStreamReader::EndElement) {
                    n2token = reader.readNext();
                    if(ntoken == QXmlStreamReader::StartElement && reader.name() == "node") {
                        //get child
                        QTreeWidgetItem* child = loadSubTree(reader);
                        //add to this node
                        item->addChild(child);
                    }
                } //while for each child
            }
        } // if is startElement
    }//while - reading reader

    return item;
}


TreeWindow* TreeWindow::instance() {
    if (_instance == NULL) {
        _instance = new TreeWindow();
    }
    return _instance;
}

QTreeWidgetItem* TreeWindow::getSelectedOrRoot() {
    QList<QTreeWidgetItem*> selItems = ui->treeWidget->selectedItems();

    if (selItems.count()>0) {
        return selItems[0];
    } else {
        return _rootItm;
    }
}

void TreeWindow::treeExpandCollapse(QTreeWidgetItem *itm, int treeDepth, bool beforeExpand, bool afterExpand) {
    int thisTreeDepth = itm->text(COL_TREEDEPTH).toInt();
    if (thisTreeDepth < treeDepth) {
        itm->setExpanded(beforeExpand);
    } else {
        itm->setExpanded(afterExpand);
    }

    for(int i=0; i<itm->childCount(); i++) {
        treeExpandCollapse(itm->child(i), treeDepth, beforeExpand, afterExpand);
    }
}

void TreeWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
/*    QVariant val = item->data(COL_DESCR_LONG,0);

    ui->textEdit->setPlainText(val.toString());*/
}


void TreeWindow::on_actionExpand_Until_This_triggered()
{
    QTreeWidgetItem *itm = getSelectedOrRoot();
    int treeDepth = itm->text(COL_TREEDEPTH).toInt();

    //cout << "tdepth: "<<treeDepth<<endl;
    treeExpandCollapse(_rootItm, treeDepth, true, false);
}


void TreeWindow::on_actionChosen_Action_triggered()
{
    ui->textEdit->setText(_chosenActStr);
}


void TreeWindow::on_actionShow_Parameters_triggered()
{
    QString txtStr = "?";

    if (_params != NULL) {
        stringstream str;
        _params->printVariables(str,true);
        txtStr = QString::fromStdString(str.str());
    }

    ui->textEdit->setText(txtStr); //_paramsValuesStr);
}

void TreeWindow::on_actionExpand_All_triggered()
{
    treeExpandCollapse(_rootItm, 0, true, true);
}

void TreeWindow::on_actionCollapse_All_triggered()
{
    treeExpandCollapse(_rootItm, 0, false, false);
}

void TreeWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    QVariant val = current->data(COL_DESCR_LONG,0);

    ui->textEdit->setPlainText(val.toString());
}

void TreeWindow::on_actionSave_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Tree File"), ".xml", tr("XML files (*.xml);;All files (*.*)"));
    if (!fileName.isEmpty()) {
        storeTree(_rootItm,fileName);
    }
}

void TreeWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tree File"), "", tr("XML files (*.xml);;All files (*.*)"));
    if (!fileName.isEmpty()) {
        loadTree(fileName);
    }
}

void TreeWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void TreeWindow::on_actionChange_Parameters_triggered()
{
    SetParamsDialog setParamDiag(_params, this);
    setParamDiag.exec();
}

void TreeWindow::on_actionClear_Memory_triggered()
{
    cout << "Deleting old trees: "<<flush;
    for (QTreeWidgetItem* item : _rootItmHistVector) {
        delete item;
        cout << "*"<<flush;
    }
    cout <<endl;
}
