#ifndef XMLCONFIGNODEPRINTVISITOR_H
#define XMLCONFIGNODEPRINTVISITOR_H

#include "xmlconfignode.h"

class XMLConfigNodePrintVisitor : public XMLConfigNodeVisitor {
public:
    //XMLConfigNodePrintVisitor();
    void visit(XMLConfigNode *node);
};

#endif // XMLCONFIGNODEPRINTVISITOR_H
