#include "XMLConfig/xmlconfignode.h"

XMLConfigNode::XMLConfigNode(const QString& qName, const QXmlAttributes& atts, XMLConfigNode* parent) {
    _name = qName;    
    _parent = parent;

    //list attributes
    for(int i=0;i<atts.length();i++) {
        //_attributes.insert(atts.qName(i),atts.value(i));
        _attributes.append(atts.qName(i),atts.uri(i),atts.localName(i),atts.value(i));
    }
}

//! Add child node.
void XMLConfigNode::addChild(XMLConfigNode* node) {
    //_children.push_back(node);

    _children.insertMulti(node->getName(),node);
}

//! set value
void XMLConfigNode::setValue(const QString& value) {
    _value = value;
}

//! get name
QString XMLConfigNode::getName() {
    return _name;
}

//! get attributes
/*const QXmlAttributes* XMLConfigNode::getAttributes() {
    return _attributes;
}*/

QString XMLConfigNode::getAttribute(QString name) {
    return _attributes.value(name);
}

//! get value (if a value)
QString XMLConfigNode::getValue() {
    return _value;
}

//! get parrent
XMLConfigNode* XMLConfigNode::getParent() {
    return _parent;
}

bool XMLConfigNode::isEndNode() {
    return _children.size()==0;
}

//vector<XMLConfigNode*>* XMLConfigNode::getChildren() {
QList<XMLConfigNode*> XMLConfigNode::getChildren() {
    return _children.values();
}

//! accept visitor depth first
void XMLConfigNode::acceptDepthFirst(XMLConfigNodeVisitor& visitor) {
    visitor.visit(this);
    //vector<XMLConfigNode*>::iterator vecIt;
    QList<XMLConfigNode*> children = _children.values();
    QList<XMLConfigNode*>::iterator vecIt;

    //for (vecIt=_children.begin(); vecIt!=_children.end(); vecIt++) {
    for (vecIt=children.begin(); vecIt!=children.end(); vecIt++) {
        (*vecIt)->acceptDepthFirst(visitor);
    }
}

//! accept visitor breadth first
void XMLConfigNode::acceptBreadthFirst(XMLConfigNodeVisitor& visitor) {
    //vector<XMLConfigNode*>::iterator vecIt;
    QList<XMLConfigNode*> children = _children.values();
    QList<XMLConfigNode*>::iterator vecIt;

    //for (vecIt=_children.begin(); vecIt!=_children.end(); vecIt++) {
    for (vecIt=children.begin(); vecIt!=children.end(); vecIt++) {
        (*vecIt)->acceptBreadthFirst(visitor);
    }
    visitor.visit(this);
}

//! get child with name,  NULL if not found
QList<XMLConfigNode*> XMLConfigNode::getChildren(QString name) {
    return _children.values(name);
}

//! get child with name,  NULL if not found
XMLConfigNode* XMLConfigNode::getFirstChild(QString name) {
    if (_children.find(name)==_children.end()) {
        return NULL; //not found
    } else {
        return _children.value(name);
    }
}

QString XMLConfigNode::getFirstChildValue(QString name) {
    XMLConfigNode* node = getFirstChild(name);
    QString val;
    if (node!=NULL) {
        val = node->getValue();
    }
    return val;
}

