# Hide&Seek Qt project file
# uses APPL 0.95
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
# -w -O3 $(INCDIR) -msse2 -mfpmath=sse

# AG121224: change compiler to 4.6
#           since on 4.7 it does complain about APPL and Boost files
#           and we need 4.8 for C++ features: random distributions
QMAKE_CC = gcc-4.8
QMAKE_CXX = g++-4.8

QMAKE_CXXFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse \
    -fpermissive \
    -std=c++11
#    -pg
# -pg = GProfiler

QMAKE_CFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse \
    -fpermissive \
#    -pg

#QMAKE_LFLAGS += -pg

#NDEBUG is to ignore asserts !
#DEFINES += NDEBUG


# debug of H&S
DEFINES += DEBUG_HS_INIT_ON
DEFINES += DEBUG_HS_ON
DEFINES += DEBUG_HS1_ON
#DEFINES += DEBUG_MAP_ON
DEFINES += DEBUG_SEGMENT_ON
DEFINES += DEBUG_DELETE
# DEBUG_HS_INIT_ON  # - only init
# DEBUG_HS_ON - detailed
# DEBUG_HS1_ON - detailed debug
# DEFINES += DEBUG_AG_POL2    # for debug of policy

#DEFINES += HSSOLVER_ACTSTATE_ON
    #ag130320: enable keeping track of actual state (y), but not always known neither used, only for evaluation of system -> disabled


# DEFINES += DEBUG_SHOW_TRANS_ON

# DEFINES += DEBUG_COMPARE_TOP_BOTTOM
##DEFINES += DEBUG_CHECK_TOP

#- show transition probs
# DEBUG_DELETE - delete

# appl directory
APPL_DIR = /home/agoldhoorn/Projects/appl-0.95/src
#${HOME}/Projects/appl-0.95/src


#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/seekerhs/
} else {
    DESTDIR = build/release/seekerhs/
}
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui



# application
TEMPLATE = app #lib

#Qt packages
QT -= network
QT -= gui

# --files--
HEADERS += src/HSGame/gmap.h \
    src/HSGame/gplayer.h \
    src/PathPlan/exceptions.h \
    src/PathPlan/map_grid/ppm_image_map.h \
    src/PathPlan/map_grid/map_grid.h \
    src/PathPlan/astar/astar.h \
    src/PathPlan/pathplanner.h \
    src/Segment/segment.h \        
    src/Utils/hslog.h \
    src/Utils/timer.h \    
    src/hsconfig.h \    
    src/Solver/hsmomdp.h \
    src/Solver/hsmomdp_layered.h \
    src/Solver/hsmomdp_runpolicy.h \
    src/Solver/hssolver.h \
    src/Utils/generic.h \
    src/Segment/kmeanssegmenter.h \    
    src/Segment/robotcenteredsegmentation.h \
    src/Segment/basesegmenter.h \
    src/hsglobaldata.h \
    src/Segment/testsegmenter.h \
    src/Segment/combinecenteredsegmentation.h \
    src/autoplayer.h \
    src/seekerhs.h

SOURCES += src/HSGame/gmap.cpp \
    src/HSGame/gplayer.cpp \
    src/PathPlan/exceptions.cpp \
    src/PathPlan/map_grid/map_grid.cpp \
    src/PathPlan/map_grid/ppm_image_map.cpp \
    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Segment/segment.cpp \    
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/Solver/hsmomdp.cpp \
    src/Solver/hsmomdp_layered.cpp \
    src/Solver/hsmomdp_runpolicy.cpp \
    src/Solver/hssolver.cpp \
    src/Utils/generic.cpp \
    src/Segment/kmeanssegmenter.cpp \
    src/Segment/robotcenteredsegmentation.cpp \
    src/Segment/basesegmenter.cpp \
    src/hsglobaldata.cpp \
    src/Segment/testsegmenter.cpp \
    src/Segment/combinecenteredsegmentation.cpp \
    src/seekerhs.cpp




# included files: APPL 0.95 headers
INCLUDEPATH += $${APPL_DIR}/Models/MOMDP \
    $${APPL_DIR}/Bounds \
    $${APPL_DIR}/Core \
    $${APPL_DIR}/Utils \
    $${APPL_DIR}/Utils/boost/config \
    $${APPL_DIR}/Utils/boost/smart_ptr \
    $${APPL_DIR}/Utils/boost/exception \
    $${APPL_DIR}/Utils/boost/detail \
    $${APPL_DIR}/Utils/boost/smart_ptr/detail \
    $${APPL_DIR}/OfflineSolver \
    $${APPL_DIR}/Algorithms \
    $${APPL_DIR}/Core \
    $${APPL_DIR}/Evaluator \
    $${APPL_DIR}/MathLib \
    $${APPL_DIR}/Parser \
    $${APPL_DIR}/Parser/POMDPX \
    $${APPL_DIR}/Parser/Cassandra \
    $${APPL_DIR}/Algorithms \
    $${APPL_DIR}/Algorithms/SARSOP


#    $${APPL_DIR}/Utils/config \
#    $${APPL_DIR}/Utils/smart_ptr \
#    $${APPL_DIR}/Utils/exception \
#    $${APPL_DIR}/Utils/detail \
#    $${APPL_DIR}/Utils/smart_ptr/detail \



#OpenCV
LIBS += -L/usr/local/lib -L/usr/lib -lopencv_core  -L/usr/local/lib/iridrivers -liriutils
#APPL
LIBS += -Llib -L../lib -lappl
# include
INCLUDEPATH += /usr/local/include/opencv \
    /usr/local/include \    
#    src \
    include \
    /usr/include \
    /usr/include/opencv \
    /usr/local/include/iridrivers


OTHER_FILES += \
    hs_todo.txt




