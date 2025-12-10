#include "XMLConfig/xmlconfigparser.h"

//#include <iostream>
#include <QDebug>

//using namespace std;



XMLConfigParser::XMLConfigParser() : QXmlDefaultHandler()
{
    _currentNode = _rootNode = NULL;
}


bool XMLConfigParser::startDocument()
{
    //inAdBook = false;
    return true;
}
bool XMLConfigParser::endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
{
    //qDebug() << "END-ELEMENT: nameSpace="<<namespaceURI<<"; localName="<<localName<<"; name="<<qName<<endl;

    //go back to previous node
    _currentNode = _currentNode->getParent();

    return true;
}

bool XMLConfigParser::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts)
{
    /*qDebug() << "START-ELEMENT: nameSpace="<<namespaceURI<<"; localName="<<localName<<"; name="<<qName<<";#attrs="<<atts.count()<<endl;
    for(unsigned i=0; i<atts.count(); i++) {
        qDebug() << "  - "<<atts.qName(i)<<" = "<<atts.value(i)<<endl;
    }*/

    //create node
    XMLConfigNode* node = new XMLConfigNode(qName,atts,_currentNode);

    //check if there exists a root
    if (_rootNode == NULL) {
        //set as root
        _currentNode = _rootNode = node;
    } else {
        //add as child
        _currentNode->addChild(node);
    }

    //set as new current
    _currentNode = node;

    return true;
}


bool XMLConfigParser::fatalError( const QXmlParseException & exception ) {
    qDebug() << "Fatal error: "<<exception.message()<<endl;
    return true;
}

bool XMLConfigParser::warning(const QXmlParseException &exception) {
    qDebug() << "Warning: "<<exception.message()<<endl;
    return true;
}

bool XMLConfigParser::error(const QXmlParseException &exception) {
    qDebug() << "Error: "<<exception.message()<<endl;
    return true;
}

bool XMLConfigParser::startCDATA() {
    //qDebug()<<"[cdata]"<<endl;
    return true;
}

bool XMLConfigParser::endCDATA() {
    //qDebug()<<"[/cdata]"<<endl;
    return true;
}

bool XMLConfigParser::characters(const QString &ch) {
    if (ch.trimmed().isEmpty()) return true;

    //qDebug() << " string: '"<<ch<<"'"<<endl;

    if (_currentNode!=NULL) _currentNode->setValue(ch);

    return true;
}

XMLConfigNode* XMLConfigParser::getRootNode() {
    return _rootNode;
}

XMLConfigNode* XMLConfigParser::parseXMLConfigFile(QFile* file) {
    QXmlInputSource source(file);

    XMLConfigParser xmlConfigParser;

    QXmlSimpleReader reader;
    reader.setContentHandler( &xmlConfigParser );

    reader.parse(& source );

    return xmlConfigParser.getRootNode();
}

