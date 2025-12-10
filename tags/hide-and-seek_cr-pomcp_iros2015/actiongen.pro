# Hide&Seek Qt project file
# uses APPL 0.95
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
# -w -O3 $(INCDIR) -msse2 -mfpmath=sse
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

# debug of H&S
#DEFINES += DEBUG_HS_INIT_ON
DEFINES += DEBUG_HS_ON
#DEFINES += DEBUG_HS1_ON
#DEFINES += DEBUG_SEGMENT_ON
#DEFINES += DEBUG_DELETE


#DEFINES += DEBUG_MAP_ON
# DEBUG_HS_INIT_ON   - only init
# DEBUG_HS_ON - detailed
# DEBUG_HS1_ON - detailed debug


#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/actiongen/
} else {
    DESTDIR = build/release/actiongen/
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
    include/hsglobaldata.h \
    include/ActionListGenerator/actionlistgenerator.h \
    include/HSGame/gplayer.h \
    include/HSGame/pos.h \
#    include/PathPlan/map_grid/ppm_image_map.h \
#    include/PathPlan/map_grid/map_grid.h \
#    include/PathPlan/astar/astar.h \
    include/PathPlan/pathplanner.h \
    include/Utils/generic.h \
    include/hsconfig.h \
    include/Utils/timer.h \
    include/Utils/readwriteimage.h

SOURCES +=     src/ActionListGenerator/main_actionlistgen.cpp \
    src/HSGame/gmap.cpp \
    src/hsglobaldata.cpp \
    src/HSGame/gplayer.cpp \
    src/HSGame/pos.cpp \
    src/ActionListGenerator/actionlistgenerator.cpp \
#    src/PathPlan/map_grid/map_grid.cpp \
#    src/PathPlan/map_grid/ppm_image_map.cpp \
#    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Utils/generic.cpp \
    src/Utils/timer.cpp \
    src/Utils/readwriteimage.cpp


LIBS += -L/usr/local/lib -lopencv_core  -Llib -liriutils
INCLUDEPATH += /usr/local/include/opencv \
    /usr/local/include \
    include \
    /usr/local/include/iridrivers





