

HEADERS += \
    src/HSGame/gmap.h \
    src/HSGame/gplayer.h \
    src/GUI/gmapwidget.h \
    src/GUI/popup.h \
    src/PathPlan/exceptions.h \
    src/PathPlan/map_grid/ppm_image_map.h \
    src/PathPlan/map_grid/map_grid.h \
    src/PathPlan/astar/astar.h \
    src/PathPlan/pathplanner.h \
    src/Utils/hslog.h \
    src/Utils/timer.h \
    src/hsconfig.h \
    src/Utils/generic.h \
    src/hsglobaldata.h

SOURCES += \
    src/HSGame/gmap.cpp \
    src/HSGame/gplayer.cpp \
    src/GUI/main_gui.cpp \
    src/GUI/gmapwidget.cpp \
    src/GUI/popup.cpp \
    src/PathPlan/exceptions.cpp \
    src/PathPlan/map_grid/map_grid.cpp \
    src/PathPlan/map_grid/ppm_image_map.cpp \
    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/Utils/generic.cpp \
    src/hsglobaldata.cpp

FORMS += src/GUI/simmaingui.ui

QT += network

DEFINES += USE_QT

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



INCLUDEPATH += src





