
#include <QtCore/QCoreApplication>

#include <QSettings>
#include <QDebug>

#include <QXmlStreamReader>
#include <QXmlSimpleReader>
#include <QXmlInputSource>

#include <QString>
#include <QStringList>

#include <QFile>


#include "xmlconfigparser.h"
#include "xmlconfignodeprintvisitor.h"

//Source: http://pastebin.com/M6Lqac91#

/*
bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map);
bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    const QSettings::Format XmlFormat =
            QSettings::registerFormat("xml", readXmlFile, writeXmlFile);

    QSettings settings(XmlFormat, QSettings::UserScope, "MySoft",
                       "Star Runner");




    return a.exec();
}*/



static bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map)
{
    qDebug()<< "read";
    QXmlStreamReader reader(&device);
    QString key;
    while(!reader.atEnd())
    {
        qDebug()<<">"<<reader.tokenString()<<endl;
        if( reader.isStartElement() && reader.tokenString() != "Settings")
        {
            if( reader.text().isNull() )
            {
                // key = Settings
                if(key.isEmpty())
                    key = reader.tokenString();
                // key = Settings/Intervall
                else key += "/" + reader.tokenString();

                qDebug()<<"key:"<<key<<endl;
            }
            else {
                map.insert(key, reader.text().data());
                qDebug()<<"insert key:"<<key<<endl;
            }
        } else {
            qDebug()<<"     skipping"<<endl;
            //ag120503: in order not te get stuck -> DOESN'T work
            reader.skipCurrentElement();
        }

        qDebug()<<"<"<<key<<endl;
    }

    return true;
}

static bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    qDebug()<< "write";
    QXmlStreamWriter writer(&device);
    writer.writeStartDocument("1.0");
    writer.writeStartElement("Settings");
    foreach(QString key, map.keys())
    {
        foreach(QString elementKey, key.split("/"))
        {
            writer.writeStartElement(elementKey);
        }
        writer.writeCDATA(map.value(key).toString());
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();

    return true;
}



int mainx(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //a.setApplicationName("SPC");
    //a.setOrganizationName("EUP");

    //set new config format
    const QSettings::Format XmlFormat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);
    QSettings::setPath(XmlFormat, QSettings::UserScope, a.applicationFilePath());
    QSettings::setDefaultFormat(XmlFormat);

    /*QSettings setting( XmlFormat, QSettings::UserScope, "EUP", "applicationtest");
    setting.setValue("xmlFormat", 2);*/

    qDebug() << "path: " <<  a.applicationFilePath()<<endl;

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, a.applicationFilePath());

    //ini using system locations:
    //QSettings cfg(QSettings::IniFormat, QSettings::UserScope, "organization");//, "application");
    //ini using specific file:
    //QSettings cfg("test.ini",QSettings::IniFormat);//, QSettings::UserScope, "organization");//, "application");
    QSettings cfg("test.xml",XmlFormat);

    qDebug() << "TEST= "<<cfg.value("TEST")<<endl;
    cfg.setValue("TEST",123);
    cfg.setValue("TEST3","x55x");
    cfg.setValue("floaty",1.255333f);
    //cfg.

    return 0;//a.exec();
}



int maino1(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //a.setApplicationName("SPC");
    //a.setOrganizationName("EUP");

    //set new config format
    const QSettings::Format XmlFormat = QSettings::registerFormat("xml", readXmlFile, writeXmlFile);
    QSettings::setPath(XmlFormat, QSettings::UserScope, a.applicationFilePath());
    QSettings::setDefaultFormat(XmlFormat);

    qDebug() << "path: " <<  a.applicationFilePath()<<endl;
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, a.applicationFilePath());

    QString file = QString::fromLatin1(argv[1]);
    qDebug() <<"file: "<<file<<endl;
    QSettings cfg(file,XmlFormat);

    qDebug() << cfg.value("mode")<<endl;
    qDebug() << cfg.value("record/in-port")<<endl;


    return 0;//a.exec();
}


void printNode(XMLConfigNode* node, QString ident="") {
    qDebug() << ident << node->getName()<<" ["<<node->getValue()<<"]"<<endl;
    QString newInd=ident+"\t";
    QList<XMLConfigNode*> children = node->getChildren();
    foreach(XMLConfigNode* child, children) {
        printNode(child,newInd);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //QString file = QString::fromAscii(argv[1]);
    QFile file(argv[1]);
    QXmlInputSource source(&file );

    XMLConfigParser xmlConfigParser;

    QXmlSimpleReader reader;
    reader.setContentHandler( &xmlConfigParser );

    reader.parse(& source );

    qDebug() << "--- done ----"<<endl<<"Printing:"<<endl;

    XMLConfigNode* root = xmlConfigParser.getRootNode();
    if (root==NULL) {
        qDebug()<<"NULL"<<endl;
    } else {
        XMLConfigNodePrintVisitor pvisit;
        root->acceptDepthFirst(pvisit);
    }

    qDebug() << "--- done ----"<<endl<<"Printing all:"<<endl;
    printNode(root,"> ");

    return 0;//a.exec();
}
