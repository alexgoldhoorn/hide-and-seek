

QMAKE_CC = gcc-5  #4.8
QMAKE_CXX = g++-5  #4.8

#AG151118: define usage C++11
CONFIG += c++11

QMAKE_CXXFLAGS += -O3 \
#    -msse2 \
#    -mfpmath=sse\
#    -fpermissive \
#    -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable -Wno-switch

QMAKE_CFLAGS += -O3 #\
#    -msse2 \
#    -fpermissive \
#    -mfpmath=sse


#QMAKE_CFLAGS_WARN_OFF

HEADERS += include/AutoHider/autohider.h \
        include/AutoHider/autowalker.h \
        include/AutoHider/fromlisthider.h \
        include/GUI/gmapwidget.h \
        include/GUI/hsclientmainwindow.h \
        include/GUI/popup.h \
        include/GUI/setparamsdialog.h \
        include/HSGame/gmap.h \
        include/HSGame/gplayer.h \
        include/HSGame/idpos.h \
        include/HSGame/pos.h \
        include/HSGame/bpos.h \
        include/PathPlan/astarplanner.h \
        include/PathPlan/pathplanner.h \
        include/PathPlan/propdistplanner.h \
        include/Server/hsserverclientcommunication.h \
        include/Utils/generic.h \
        include/Utils/hslog.h \
        include/Utils/readwriteimage.h \
        include/Utils/timer.h \
        include/Base/abstractautoplayer.h \
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

SOURCES += src/AutoHider/autohider.cpp \
        src/AutoHider/autowalker.cpp \
        src/AutoHider/fromlisthider.cpp \
        src/GUI/gmapwidget.cpp \
        src/GUI/hsclientmainwindow.cpp \
        src/GUI/main_gui.cpp \
        src/GUI/popup.cpp \
        src/GUI/setparamsdialog.cpp \
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
        src/Base/abstractautoplayer.cpp \
        src/Base/autoplayer.cpp \
        src/Base/gameconnectorclient.cpp \
        src/Base/hsglobaldata.cpp \
        src/Base/playerinfo.cpp \
        src/Base/seekerhsparams.cpp
 #   src/PathPlan/map_grid/map_grid.cpp \
 #   src/PathPlan/map_grid/ppm_image_map.cpp \
 #   src/PathPlan/astar/astar.cpp \


FORMS += src/GUI/setparamsdialog.ui \
    src/GUI/hsclientmainwindow.ui

QT += network
QT += widgets

DEFINES += USE_QT
#DEFINES += DEBUG_CLIENT_ON
#DEFINES += DEBUG_CLIENT_VERB_ON
DEFINES += DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
DEFINES += DO_NOT_USE_MOMDP
DEFINES += DO_NOT_WRITE_BELIEF_IMG
DEFINES += GMAP_CAN_DELETE
#DEFINES += DEBUG_SHS_ON

#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/gui/
} else {
    DESTDIR = build/release/gui/
}
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui



INCLUDEPATH += include \
    /usr/local/include/iridrivers


LIBS += -L/usr/local/lib -L/usr/lib -L/usr/local/lib/iridrivers -liriutils
