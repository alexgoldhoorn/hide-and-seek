# Hide&Seek Qt project file - SERVER
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
QMAKE_CC = gcc #4.8
QMAKE_CXX = g++ #4.8

#AG151118: define usage C++11
CONFIG += c++11
#CONFIG+=USE_OPENCV3

QMAKE_CXXFLAGS += -O3 \
#    -msse2 \
#    -mfpmath=sse\
#    -fpermissive \
#    -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable

QMAKE_CFLAGS += -O3 #\
#    -msse2 \
#    -fpermissive \
#    -mfpmath=sse

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

#DEFINES += SERVERTIMER_ENABLED

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
HEADERS += 	include/AutoHider/autohider.h \
        include/AutoHider/autowalker.h \
        include/AutoHider/fromlisthider.h \
        include/AutoHider/randomfixedhider.h \
        include/AutoHider/randomhider.h \
        include/AutoHider/randomlisthider.h \
        include/AutoHider/randomwalker.h \
        include/AutoHider/sfmwalkers.h \
        include/AutoHider/smarthider.h \
        include/AutoHider/verysmarthider.h \
        include/AutoHider/verysmarthider2.h \
        include/HSGame/gmap.h \
        include/HSGame/gplayer.h \
        include/HSGame/idpos.h \
        include/HSGame/pos.h \
        include/HSGame/bpos.h \
        include/PathPlan/astarplanner.h \
        include/PathPlan/pathplanner.h \
        include/PathPlan/propdistplanner.h \
        include/Segment/segment.h \
        include/Server/clientsocket.h \
        include/Server/hsgamelog.h \
        include/Server/hsgamelogdb.h \
        include/Server/hsserver.h \
        include/Server/hsserverclientcommunication.h \
        include/Server/hsserverconfig.h \
        include/Server/hstcpserver.h \
        include/Utils/generic.h \
        include/Utils/hslog.h \
        include/Utils/readwriteimage.h \
        include/Utils/timer.h \
        include/Base/autoplayer.h \
        include/Base/hsconfig.h \
        include/Base/hsglobaldata.h \
        include/Base/playerinfo.h \
        include/Base/playerinfoserver.h \
        include/Base/seekerhsparams.h
#    include/PathPlan/map_grid/ppm_image_map.h \
#    include/PathPlan/map_grid/map_grid.h \
#    include/PathPlan/astar/astar.h \
#    include/AutoHider/gameconnectorhider.h \


SOURCES +=	src/AutoHider/autohider.cpp \
        src/AutoHider/autowalker.cpp \
        src/AutoHider/fromlisthider.cpp \
        src/AutoHider/randomfixedhider.cpp \
        src/AutoHider/randomhider.cpp \
        src/AutoHider/randomlisthider.cpp \
        src/AutoHider/randomwalker.cpp \
        src/AutoHider/sfmwalkers.cpp \
        src/AutoHider/smarthider.cpp \
        src/AutoHider/verysmarthider.cpp \
        src/AutoHider/verysmarthider2.cpp \
        src/HSGame/gmap.cpp \
        src/HSGame/gplayer.cpp \
        src/HSGame/idpos.cpp \
        src/HSGame/pos.cpp \
        src/HSGame/bpos.cpp \
        src/PathPlan/astarplanner.cpp \
        src/PathPlan/pathplanner.cpp \
        src/PathPlan/propdistplanner.cpp \
        src/Server/clientsocket.cpp \
        src/Server/hsgamelog.cpp \
        src/Server/hsgamelogdb.cpp \
        src/Server/hsserver.cpp \
        src/Server/hsserverconfig.cpp \
        src/Server/hstcpserver.cpp \
        src/Server/main_server.cpp \
        src/Utils/generic.cpp \
        src/Utils/hslog.cpp \
        src/Utils/readwriteimage.cpp \
        src/Utils/timer.cpp \
        src/Base/autoplayer.cpp \
        src/Base/hsglobaldata.cpp \
        src/Base/playerinfo.cpp \
        src/Base/playerinfoserver.cpp \
        src/Base/seekerhsparams.cpp
#    src/PathPlan/map_grid/map_grid.cpp \
#    src/PathPlan/map_grid/ppm_image_map.cpp \
#    src/PathPlan/astar/astar.cpp \



OTHER_FILES +=     todo.txt \
    hsserver-client-communication-def.txt

#IRI: libutils + people prediction
LIBS += -L/usr/local/lib/iridrivers -liriutils #-lpeople_prediction
#OpenCV
#LIBS += -lopencv_core -lopencv_imgproc #-lopencv_highgui
#LIBS += -L/usr/local/lib -L/usr/lib -L/usr/lib/x86_64-linux-gnu -lopencv_core -lopencv_imgproc #-lopencv_highgui
LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui
USE_OPENCV3 {
LIBS += -lopencv_imgproc -lopencv_imgcodecs  # FOR OpenCV 3 !!
}
#LIBS += -L/usr/local/lib/iridrivers -liriutils -lopencv_core -lopencv_imgproc #-lopencv_highgui
#LIBS += -Llib -L../lib # -lappl


INCLUDEPATH +=  /usr/local/include \
    include \
    /usr/local/include/iridrivers \
    ext_include \
    ../ext_include
