#include "XMLConfig/xmlconfignodeprintvisitor.h"
#include <iostream>

using namespace std;

/*XMLConfigNodePrintVisitor::XMLConfigNodePrintVisitor()
{
}*/

void XMLConfigNodePrintVisitor::visit(XMLConfigNode *node) {
    cout << node->getName().toStdString();
    if (node->isEndNode()) cout << " [end]";
    if (!node->getValue().isEmpty()) cout << " '"<<node->getValue().toStdString()<<"'";
    cout<<endl;
}
