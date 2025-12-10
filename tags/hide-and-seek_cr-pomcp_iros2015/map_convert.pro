# Hide&Seek Qt project file
# uses APPL 0.95
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
# -w -O3 $(INCDIR) -msse2 -mfpmath=sse

QMAKE_CC = gcc-4.8
QMAKE_CXX = g++-4.8

QMAKE_CXXFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse \
    -std=c++11
QMAKE_CFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse

# debug of H&S


#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/convpgmmap
} else {
    DESTDIR = build/release/convpgmmap/
}
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui



# application
TEMPLATE = app

#Qt packages
#QT += network
QT -= gui

# --files--
HEADERS += include/HSGame/gmap.h \
        include/HSGame/pos.h \
    include/hsglobaldata.h \
    include/HSGame/gplayer.h \
    include/PathPlan/exceptions.h \
    include/PathPlan/map_grid/ppm_image_map.h \
    include/PathPlan/map_grid/map_grid.h \
    include/PathPlan/astar/astar.h \
    include/PathPlan/pathplanner.h \
    include/Utils/generic.h \
        include/Utils/hslog.h \
        include/Utils/timer.h \
        include/Utils/readwriteimage.h

SOURCES +=     src/main_map_convert.cpp \
    src/HSGame/gmap.cpp \
	src/HSGame/pos.cpp \
    src/hsglobaldata.cpp \
    src/HSGame/gplayer.cpp \
    src/PathPlan/exceptions.cpp \
    src/PathPlan/map_grid/map_grid.cpp \
    src/PathPlan/map_grid/ppm_image_map.cpp \
    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Utils/generic.cpp \
        src/Utils/hslog.cpp \
        src/Utils/timer.cpp \
        src/Utils/readwriteimage.cpp

INCLUDEPATH += /usr/local/include/opencv \
    /usr/local/include \
    include/exceptions \
    include \
    /usr/include \
    /usr/include/opencv \
    /usr/local/include/iridrivers






