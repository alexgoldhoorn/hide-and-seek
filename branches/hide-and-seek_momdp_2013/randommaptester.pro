# Hide&Seek Qt project file
# uses APPL 0.95
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
# -w -O3 $(INCDIR) -msse2 -mfpmath=sse


QMAKE_CXXFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse
QMAKE_CFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse

# debug of H&S
DEFINES += DEBUG_HS_INIT_ON
DEFINES += DEBUG_HS_ON
DEFINES += DEBUG_HS1_ON
#DEFINES += DEBUG_MAP_ON
DEFINES += DEBUG_SEGMENT_ON
DEFINES += DEBUG_DELETE
# DEBUG_HS_INIT_ON   - only init
# DEBUG_HS_ON - detailed
# DEBUG_HS1_ON - detailed debug


#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/randomtester/
} else {
    DESTDIR = build/release/randomtester/
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
HEADERS += src/HSGame/gmap.h \
    src/hsglobaldata.h \
    src/RandomTester/randomtester.h \
    src/HSGame/gplayer.h \
    src/PathPlan/exceptions.h \
    src/PathPlan/map_grid/ppm_image_map.h \
    src/PathPlan/map_grid/map_grid.h \
    src/PathPlan/astar/astar.h \
    src/PathPlan/pathplanner.h \
    src/Utils/generic.h

SOURCES +=     src/RandomTester/main_randomtester.cpp \
    src/HSGame/gmap.cpp \
    src/hsglobaldata.cpp \
    src/HSGame/gplayer.cpp \
    src/RandomTester/randomtester.cpp \
    src/PathPlan/exceptions.cpp \
    src/PathPlan/map_grid/map_grid.cpp \
    src/PathPlan/map_grid/ppm_image_map.cpp \
    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Utils/generic.cpp






