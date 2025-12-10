# Hide&Seek Qt project file - SERVER
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)


QMAKE_CXXFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse
QMAKE_CFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse

# debug of H&S
DEFINES += USE_QT
DEFINES += DEBUG_HS_INIT_ON
DEFINES += DEBUG_HS_ON
DEFINES += DEBUG_HS1_ON
#DEFINES += DEBUG_MAP_ON
DEFINES += DEBUG_SEGMENT_ON
DEFINES += DEBUG_DELETE_ON
DEFINES += DEBUG_SHOW_AUTOHIDER_ON
DEFINES += AUTOHIDER_OLD # use old, external auto hider
    #AUTOHIDER_NEW -> new
# DEBUG_HS_INIT_ON   - only init
# DEBUG_HS_ON - detailed
# DEBUG_HS1_ON - detailed debug
# DEBUG_SHOW_TRANS_ON - show transition probs
# DEBUG_DELETE - delete

#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/server/
} else {
    DESTDIR = build/release/server/
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

# --files--
HEADERS += src/HSGame/gmap.h \    
    src/Server/hsserver.h \
    src/Server/hstcpserver.h \
    src/PathPlan/exceptions.h \
    src/PathPlan/map_grid/ppm_image_map.h \
    src/PathPlan/map_grid/map_grid.h \
    src/PathPlan/astar/astar.h \
    src/PathPlan/pathplanner.h \
    src/Segment/segment.h \        
    src/Utils/hslog.h \
    src/Utils/timer.h \    
    src/hsconfig.h \
    src/Utils/generic.h \    
    src/Server/hsserverconfig.h \
    src/hsglobaldata.h \
    src/Server/hsgamelog.h \
    src/Server/hsgamelogdb.h \
    src/Utils/readwriteimage.h
    #src/AutoHider/autohider.h \
    #src/AutoHider/randomhider.h

SOURCES += src/main_server.cpp \    
    src/HSGame/gmap.cpp \
    src/Server/hsserver.cpp \
    src/Server/hstcpserver.cpp \
    src/PathPlan/exceptions.cpp \
    src/PathPlan/map_grid/map_grid.cpp \
    src/PathPlan/map_grid/ppm_image_map.cpp \
    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/Utils/generic.cpp \
    src/Server/hsserverconfig.cpp \
    src/hsglobaldata.cpp \
    src/Server/hsgamelog.cpp \
    src/Server/hsgamelogdb.cpp \
    src/Utils/readwriteimage.cpp
    #src/AutoHider/autohider.cpp \
    #src/AutoHider/randomhider.cpp


OTHER_FILES += \
    hs_todo.txt


XMLCONFIG_DIR = /home/agoldhoorn/MyProjects/XMLConfig
#APPL lib
LIBS += -L$${XMLCONFIG_DIR}/build/release/ -lXMLConfig

# included files: xml config headers
INCLUDEPATH += $${XMLCONFIG_DIR}/include


