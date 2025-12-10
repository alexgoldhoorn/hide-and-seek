

QMAKE_CC = gcc-4.8
QMAKE_CXX = g++-4.8

QMAKE_CXXFLAGS += -O3 \
#    -msse2 \
#    -mfpmath=sse\
#    -fpermissive \
    -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable -Wno-switch

QMAKE_CFLAGS += -O3 #\
#    -msse2 \
#    -fpermissive \
#    -mfpmath=sse


#QMAKE_CFLAGS_WARN_OFF

HEADERS += \
    include/HSGame/gmap.h \
    include/HSGame/gplayer.h \
    include/GUI/gmapwidget.h \
    include/GUI/popup.h \
#    include/PathPlan/map_grid/ppm_image_map.h \
#    include/PathPlan/map_grid/map_grid.h \
#    include/PathPlan/astar/astar.h \
    include/PathPlan/pathplanner.h \
    include/Utils/hslog.h \
    include/Utils/timer.h \
    include/Utils/readwriteimage.h \
    include/hsconfig.h \
    include/Utils/generic.h \
    include/hsglobaldata.h \
    include/gameconnectorclient.h  \
    include/game.h \
    include/HSGame/pos.h \
    include/seekerhsparams.h\
    include/autoplayer.h \
    include/PathPlan/propdistplanner.h \
    include/PathPlan/astarplanner.h \
    include/HSGame/idpos.h \
    include/abstractautoplayer.h \
    include/GUI/setparamsdialog.h\
    include/AutoHider/autohider.h \
    include/autowalker.h \
    include/AutoHider/fromlisthider.h \
    include/GUI/hsclientmainwindow.h \
    include/Server/hsserverclientcommunication.h

SOURCES += \
    src/HSGame/gmap.cpp \
    src/HSGame/gplayer.cpp \
    src/GUI/main_gui.cpp \
    src/GUI/gmapwidget.cpp \
    src/GUI/popup.cpp \
 #   src/PathPlan/map_grid/map_grid.cpp \
 #   src/PathPlan/map_grid/ppm_image_map.cpp \
 #   src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/Utils/readwriteimage.cpp \
    src/Utils/generic.cpp \
    src/hsglobaldata.cpp \
    src/gameconnectorclient.cpp \
    src/HSGame/pos.cpp \
    src/seekerhsparams.cpp \
    src/autoplayer.cpp \
    src/PathPlan/propdistplanner.cpp \
    src/PathPlan/astarplanner.cpp \
    src/HSGame/idpos.cpp \
    src/abstractautoplayer.cpp \
    src/GUI/setparamsdialog.cpp \
    src/AutoHider/autohider.cpp \
    src/autowalker.cpp \
    src/AutoHider/fromlisthider.cpp \
    src/GUI/hsclientmainwindow.cpp


FORMS += src/GUI/setparamsdialog.ui \
    src/GUI/hsclientmainwindow.ui

QT += network
QT += widgets

DEFINES += USE_QT
DEFINES += DEBUG_CLIENT_ON
DEFINES += DEBUG_CLIENT_VERB_ON
DEFINES += DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR
DEFINES += DO_NOT_USE_MOMDP
DEFINES += DO_NOT_WRITE_BELIEF_IMG
DEFINES += GMAP_CAN_DELETE
DEFINES += DEBUG_SHS_ON

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
