# Hide&Seek Qt project file - SERVER
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
QMAKE_CC = gcc-4.8
QMAKE_CXX = g++-4.8

QMAKE_CXXFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse\
    -fpermissive \
    -std=c++11

QMAKE_CFLAGS += -O3 \
    -msse2 \
    -fpermissive \
    -mfpmath=sse

# no assert
DEFINES += NDEBUG

# debug of H&S
DEFINES += USE_QT
#DEFINES += DEBUG_HS_INIT_ON
#DEFINES += DEBUG_MAP_ON
DEFINES += AUTOHIDER_OLD # use old, external auto hider

# DEBUG_DELETE - delete

#DEFINES += DEBUG_SERVER_ON
#DEFINES += DEBUG_SERVER_VERB_ON
#DEFINES += DEBUG_AUTOHIDER_ON

DEFINES += DO_NOT_USE_MOMDP

#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/server/
    LIBS += -Lbuild/debug/xmlconfig/ -lXMLConfig
} else {
    DESTDIR = build/release/server/
    LIBS += -Lbuild/release/xmlconfig/ -lXMLConfig
}
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui

#QMAKE_MAKEFILE = Makefile_server

# application
TEMPLATE = app

#Qt packages
QT += network
QT += xml
QT += sql
QT -= gui
#QT += widgets

# --files--
HEADERS += include/HSGame/gmap.h \
    include/Server/hsserver.h \
    include/Server/hstcpserver.h \
#    include/PathPlan/map_grid/ppm_image_map.h \
#    include/PathPlan/map_grid/map_grid.h \
#    include/PathPlan/astar/astar.h \
    include/PathPlan/pathplanner.h \
    include/PathPlan/propdistplanner.h \
    include/Segment/segment.h \
    include/Utils/hslog.h \
    include/Utils/timer.h \
    include/hsconfig.h \
    include/Utils/generic.h \
    include/Server/hsserverconfig.h \
    include/hsglobaldata.h \
    include/Server/hsgamelog.h \
    include/Server/hsgamelogdb.h \
    include/Utils/readwriteimage.h \
    include/HSGame/pos.h \
    include/HSGame/gplayer.h \
    include/seekerhsparams.h \
    include/autoplayer.h \
#    include/AutoHider/gameconnectorhider.h \
    include/AutoHider/randomhider.h \
    include/AutoHider/randomlisthider.h \
    include/AutoHider/smarthider.h \
    include/AutoHider/verysmarthider.h \
    include/AutoHider/autohider.h \
    include/AutoHider/verysmarthider2.h \
    include/AutoHider/sfmwalkers.h \
    include/autowalker.h \
    include/HSGame/idpos.h \
    include/AutoHider/randomwalker.h \
    include/AutoHider/fromlisthider.h \
    include/AutoHider/randomfixedhider.h

SOURCES += src/Server/main_server.cpp \
    src/HSGame/gmap.cpp \
    src/Server/hsserver.cpp \
    src/Server/hstcpserver.cpp \
#    src/PathPlan/map_grid/map_grid.cpp \
#    src/PathPlan/map_grid/ppm_image_map.cpp \
#    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/PathPlan/propdistplanner.cpp \
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/Utils/generic.cpp \
    src/Server/hsserverconfig.cpp \
    src/hsglobaldata.cpp \
    src/Server/hsgamelogdb.cpp \
    src/HSGame/pos.cpp \
    src/HSGame/gplayer.cpp \
    src/Server/hsgamelog.cpp \
    src/seekerhsparams.cpp \
    src/AutoHider/randomhider.cpp \
    src/AutoHider/randomlisthider.cpp \
    src/AutoHider/smarthider.cpp \
    src/AutoHider/verysmarthider.cpp \
    src/AutoHider/autohider.cpp \
    src/Utils/readwriteimage.cpp \
    src/AutoHider/verysmarthider2.cpp \
    src/autoplayer.cpp \
    src/AutoHider/sfmwalkers.cpp \
    src/autowalker.cpp \
    src/HSGame/idpos.cpp \
    src/AutoHider/randomwalker.cpp \
    src/AutoHider/fromlisthider.cpp \
    src/AutoHider/randomfixedhider.cpp


LIBS += -L/usr/local/lib -L/usr/lib -L/usr/local/lib/iridrivers -liriutils -lopencv_core -lopencv_highgui
LIBS += -Llib -L../lib -lappl


INCLUDEPATH +=  include \
    /usr/local/include/iridrivers \
	ext_include \
        ../ext_include \
    /usr/local/include \
    include/
