#-------------------------------------------------
#
# Project created by QtCreator 2012-05-03T12:58:06
#
#-------------------------------------------------

TEMPLATE = app

QT       += core
QT       += xml
QT       -= gui

TARGET = XMLConfig
CONFIG   += console
CONFIG   -= app_bundle


#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/xmlconfig
} else {
    DESTDIR = build/release/xmlconfig
}

DESTDIRF = $${DESTDIR}/app
OBJECTS_DIR = $${DESTDIRF}/.obj
MOC_DIR = $${DESTDIRF}/.moc
RCC_DIR = $${DESTDIRF}/.rcc
UI_DIR = $${DESTDIRF}/.ui


SOURCES += src/main.cpp \
    src/xmlconfigparser.cpp \
    src/xmlconfignode.cpp \
    src/xmlconfignodeprintvisitor.cpp

HEADERS += \
    include/xmlconfigparser.h \
    include/xmlconfignode.h \
    include/xmlconfignodeprintvisitor.h

INCLUDEPATH += include/
