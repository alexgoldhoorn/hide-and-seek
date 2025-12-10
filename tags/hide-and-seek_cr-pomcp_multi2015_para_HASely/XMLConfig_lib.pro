#-------------------------------------------------
#
# Project created by QtCreator 2012-05-03T12:58:06
#
#-------------------------------------------------

TEMPLATE=lib

QT       += core
QT       += xml
QT       -= gui

TARGET = XMLConfig

CONFIG   -= app_bundle

# If you want a create a shared library use CONFIG += dll, and if you want to create a static library use CONFIG += staticlib.
#CONFIG += dll
CONFIG += staticlib


#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/xmlconfig
} else {
    DESTDIR = build/release/xmlconfig
}
DESTDIRF = $${DESTDIR}/lib
OBJECTS_DIR = $${DESTDIRF}/.obj
MOC_DIR = $${DESTDIRF}/.moc
RCC_DIR = $${DESTDIRF}/.rcc
UI_DIR = $${DESTDIRF}/.ui

SOURCES += \
    src/XMLConfig/xmlconfigparser.cpp \
    src/XMLConfig/xmlconfignode.cpp \
    src/XMLConfig/xmlconfignodeprintvisitor.cpp

HEADERS += \
    include/XMLConfig/xmlconfigparser.h \
    include/XMLConfig/xmlconfignode.h \
    include/XMLConfig/xmlconfignodeprintvisitor.h

INCLUDEPATH += include/

