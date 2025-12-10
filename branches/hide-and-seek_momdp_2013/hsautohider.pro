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
DEFINES += DEBUG_AUTOHIDER_ON
#DEFINES += DEBUG_MAP_ON
#DEFINES += DEBUG_DELETE
# DEBUG_HS_INIT_ON   - only init
# DEBUG_HS_ON - detailed
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
HEADERS += src/HSGame/gmap.h \    
    src/HSGame/gplayer.h \
    src/PathPlan/exceptions.h \
    src/PathPlan/map_grid/ppm_image_map.h \
    src/PathPlan/map_grid/map_grid.h \
    src/PathPlan/astar/astar.h \
    src/PathPlan/pathplanner.h \
    src/Utils/hslog.h \
    src/Utils/timer.h \    
    src/hsconfig.h \
    src/Utils/generic.h \    
    src/hsglobaldata.h \
    src/autoplayer.h \
    src/AutoHider/gameconnectorhider.h \
    src/AutoHider/randomhider.h \
    src/AutoHider/randomlisthider.h \
    src/AutoHider/smarthider.h \
    src/AutoHider/verysmarthider.h \
    src/AutoHider/autohider.h

SOURCES += src/AutoHider/main_autohider.cpp \
    src/HSGame/gmap.cpp \
    src/HSGame/gplayer.cpp \
    src/PathPlan/exceptions.cpp \
    src/PathPlan/map_grid/map_grid.cpp \
    src/PathPlan/map_grid/ppm_image_map.cpp \
    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/Utils/generic.cpp \
    src/hsglobaldata.cpp \    
    src/AutoHider/gameconnectorhider.cpp \
    src/AutoHider/randomhider.cpp \
    src/AutoHider/randomlisthider.cpp \
    src/AutoHider/smarthider.cpp \
    src/AutoHider/verysmarthider.cpp \
    src/AutoHider/autohider.cpp


OTHER_FILES += \
    hs_todo.txt


#XMLCONFIG_DIR = /home/agoldhoorn/MyProjects/XMLConfig
#APPL lib
#LIBS += -L$${XMLCONFIG_DIR}/build/release/ -lXMLConfig

LIBS += -L/usr/local/lib -lopencv_core  -Llib -liriutils 
INCLUDEPATH += /usr/local/include/opencv \
    /usr/local/include \
    include/exceptions \
    src 




# included files: xml config headers
#INCLUDEPATH += $${XMLCONFIG_DIR}/include


