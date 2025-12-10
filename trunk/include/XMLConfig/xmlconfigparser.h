#ifndef XMLCONFIGPARSER_H
#define XMLCONFIGPARSER_H

#include <QXmlDefaultHandler>
#include <vector>
#include "xmlconfignode.h"



/*
TODO:   put in a library


  */


/*! Parser of a XML config file.
  It actually parses any

  */
class XMLConfigParser : public QXmlDefaultHandler
{
public:
    //! Parses an XML config file and returns the root.
    static XMLConfigNode* parseXMLConfigFile(QFile* file);

    XMLConfigParser();


    bool startDocument();
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);
    bool startCDATA();
    bool endCDATA();
    bool characters(const QString &ch);

    bool fatalError ( const QXmlParseException & exception );
    bool warning(const QXmlParseException &exception);
    bool error(const QXmlParseException &exception);

    //! get root node
    XMLConfigNode* getRootNode();

private:
    // config path
    //vector<QString> cfgPath;
    //! current node
    XMLConfigNode* _currentNode;
    XMLConfigNode* _rootNode;

};

#endif // XMLCONFIGPARSER_H
