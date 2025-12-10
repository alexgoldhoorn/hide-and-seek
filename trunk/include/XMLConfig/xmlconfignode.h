#ifndef XMLCONFIGNODE_H
#define XMLCONFIGNODE_H

//#include <vector>
#include <QMap>
#include <QString>
#include <QXmlAttributes>

using namespace std;

class XMLConfigNodeVisitor;


/*! A node in the XML (config) file.
  */
class XMLConfigNode {
public:
    /*! Create a node of an XML element.
      */
    XMLConfigNode(const QString& qName, const QXmlAttributes& atts, XMLConfigNode* parent);
    //! Add child node.
    void addChild(XMLConfigNode* node);
    //! set value
    void setValue(const QString& value);

    //! get name
    QString getName();
    //! get attributes
    //const QXmlAttributes* getAttributes();
    //! get 1 attribute
    QString getAttribute(QString name);
    //! get value (if a value)
    QString getValue();
    //! get children
    QList<XMLConfigNode*> getChildren();
    //vector<XMLConfigNode*>* getChildren();
    //! get parrent
    XMLConfigNode* getParent();

    //! get children with name
    QList<XMLConfigNode*> getChildren(QString name);

    //! get first child with name,  NULL if not found
    XMLConfigNode* getFirstChild(QString name);

    //! get value of 1st child (if it has, if no empty string)
    QString getFirstChildValue(QString name);

    //! is end node
    bool isEndNode();

    //! accept visitor depth first
    void acceptDepthFirst(XMLConfigNodeVisitor& visitor);
    //! accept visitor breadth first
    void acceptBreadthFirst(XMLConfigNodeVisitor& visitor);

private:
    QString _name;
    QXmlAttributes _attributes;
    XMLConfigNode* _parent;
    QString _value;
    QMap<QString, XMLConfigNode*> _children;
};


/*!
  visitor for config node
  */
class XMLConfigNodeVisitor {
public:
    virtual void visit(XMLConfigNode* node)=0;
};


#endif // XMLCONFIGNODE_H
