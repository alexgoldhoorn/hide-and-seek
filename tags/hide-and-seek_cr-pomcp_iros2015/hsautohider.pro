# Hide&Seek Qt project file - SERVER
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
QMAKE_CC = gcc-4.8
QMAKE_CXX = g++-4.8

QMAKE_CXXFLAGS += -O3 \
#    -msse2 \
#    -mfpmath=sse\
#    -fpermissive \
    -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable

QMAKE_CFLAGS += -O3 \
#    -msse2 \
#    -fpermissive \
#    -mfpmath=sse
    -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable

DEFINES += USE_QT

# debug of H&S
DEFINES += DEBUG_AUTOHIDER_ON
DEFINES += DEBUG_CLIENT_ON

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
    include/HSGame/gplayer.h \
#    include/PathPlan/map_grid/ppm_image_map.h \
#    include/PathPlan/map_grid/map_grid.h \
#    include/PathPlan/astar/astar.h \
    include/PathPlan/pathplanner.h \
    include/Utils/hslog.h \
    include/Utils/timer.h \
    include/hsconfig.h \
    include/Utils/generic.h \
    include/hsglobaldata.h \
    include/autoplayer.h \
#    include/AutoHider/gameconnectorhider.h \
    include/AutoHider/randomhider.h \
    include/AutoHider/randomlisthider.h \
    include/AutoHider/smarthider.h \
    include/AutoHider/verysmarthider.h \
    include/AutoHider/autohider.h \
    include/Utils/readwriteimage.h \
    include/AutoHider/verysmarthider2.h \
    include/gameconnectorclient.h  \
    include/game.h \
    include/HSGame/pos.h\
    include/seekerhsparams.h \
    include/AutoHider/sfmwalkers.h \
    include/PathPlan/propdistplanner.h \
    include/PathPlan/astarplanner.h \
    include/autowalker.h \
    include/HSGame/idpos.h \
    include/AutoHider/randomwalker.h \
    include/AutoHider/fromlisthider.h \
    include/AutoHider/randomfixedhider.h

SOURCES += src/AutoHider/main_autohider.cpp \
    src/HSGame/gmap.cpp \
    src/HSGame/gplayer.cpp \
#    src/PathPlan/map_grid/map_grid.cpp \
#    src/PathPlan/map_grid/ppm_image_map.cpp \
#    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/Utils/generic.cpp \
    src/hsglobaldata.cpp \    
#    src/AutoHider/gameconnectorhider.cpp \
    src/AutoHider/randomhider.cpp \
    src/AutoHider/randomlisthider.cpp \
    src/AutoHider/smarthider.cpp \
    src/AutoHider/verysmarthider.cpp \
    src/AutoHider/autohider.cpp \
    src/Utils/readwriteimage.cpp \
    src/AutoHider/verysmarthider2.cpp \
    src/gameconnectorclient.cpp \
    src/autoplayer.cpp \
    src/HSGame/pos.cpp\
    src/seekerhsparams.cpp \
    src/AutoHider/sfmwalkers.cpp \
    src/PathPlan/propdistplanner.cpp \
    src/PathPlan/astarplanner.cpp \
    src/autowalker.cpp \
    src/HSGame/idpos.cpp \
    src/AutoHider/randomwalker.cpp \
    src/AutoHider/fromlisthider.cpp \
    src/AutoHider/randomfixedhider.cpp


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


