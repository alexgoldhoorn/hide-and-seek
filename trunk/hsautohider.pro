# Hide&Seek Qt project file - SERVER
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
QMAKE_CC = gcc #4.8
QMAKE_CXX = g++ #4.8

#AG151118: define usage C++11
CONFIG += c++11

QMAKE_CXXFLAGS += -O3 \
#    -msse2 \
#    -mfpmath=sse\
#    -fpermissive \
#    -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable

QMAKE_CFLAGS += -O3 \
#    -msse2 \
#    -fpermissive \
#    -mfpmath=sse
#   -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable

DEFINES += USE_QT

# debug of H&S
#DEFINES += DEBUG_AUTOHIDER_ON
#DEFINES += DEBUG_CLIENT_ON

DEFINES += DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
DEFINES += DO_NOT_USE_MOMDP
DEFINES += DO_NOT_WRITE_BELIEF_IMG

DEFINES += USE_HIDER_AS_AUTOPLAYER

#DEFINES += DEBUG_MAP_ON
#DEFINES += DEBUG_DELETE
#DEFINES += DEBUG_HS_INIT_ON   #- only init
#DEFINES +=  DEBUG_HS_ON #- detailed

# DEBUG_HS1_ON - detailed debug
# DEBUG_SHOW_TRANS_ON - show transition probs
# DEBUG_DELETE - delete

#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/autohider/
} else {
    DESTDIR = build/release/autohider/
}
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui


# application
TEMPLATE = app

#Qt packages
QT += network
QT += xml
#QT += sql
QT -= gui

# --files--
HEADERS += include/HSGame/gmap.h \
        include/AutoHider/autohider.h \
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
        include/Utils/generic.h \
        include/Utils/hslog.h \
        include/Utils/readwriteimage.h \
        include/Utils/timer.h \
        include/Base/autoplayer.h \
        include/Base/game.h \
        include/Base/gameconnectorclient.h  \
        include/Base/hsconfig.h \
        include/Base/hsglobaldata.h \
        include/Base/playerinfo.h \
        include/Base/seekerhsparams.h
#    include/PathPlan/map_grid/ppm_image_map.h \
#    include/PathPlan/map_grid/map_grid.h \
#    include/PathPlan/astar/astar.h \
#    include/AutoHider/gameconnectorhider.h \

SOURCES += src/AutoHider/autohider.cpp \
        src/AutoHider/autowalker.cpp \
        src/AutoHider/fromlisthider.cpp \
        src/AutoHider/main_autohider.cpp \
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
        src/Utils/generic.cpp \
        src/Utils/hslog.cpp \
        src/Utils/readwriteimage.cpp \
        src/Utils/timer.cpp \
        src/Base/autoplayer.cpp \
        src/Base/gameconnectorclient.cpp \
        src/Base/hsglobaldata.cpp \
        src/Base/playerinfo.cpp \
        src/Base/seekerhsparams.cpp
#    src/PathPlan/map_grid/map_grid.cpp \
#    src/PathPlan/map_grid/ppm_image_map.cpp \
#    src/PathPlan/astar/astar.cpp \
#    src/AutoHider/gameconnectorhider.cpp \


#XMLCONFIG_DIR = /home/agoldhoorn/MyProjects/XMLConfig
#APPL lib
#LIBS += -L$${XMLCONFIG_DIR}/build/release/ -lXMLConfig

LIBS += -L/usr/local/lib -lopencv_core  -Llib -L/usr/local/lib/iridrivers -liriutils
INCLUDEPATH += /usr/local/include/opencv \
    /usr/local/include \
    include/ \
    /usr/local/include/iridrivers \
    ext_include \
    ../ext_include




# included files: xml config headers
#INCLUDEPATH += $${XMLCONFIG_DIR}/include


