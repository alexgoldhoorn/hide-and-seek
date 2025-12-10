# Hide&Seek Qt project file
# uses APPL 0.95
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
# -w -O3 $(INCDIR) -msse2 -mfpmath=sse

# AG121224: change compiler to 4.6
#           since on 4.7 it does complain about APPL and Boost files
QMAKE_CC = gcc-4.6
QMAKE_CXX = g++-4.6

QMAKE_CXXFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse \
    -fpermissive
QMAKE_CFLAGS += -O3 \
    -msse2 \
    -mfpmath=sse \
    -fpermissive


DEFINES += USE_QT

# debug of H&S
#DEFINES += DEBUG_HS_INIT_ON
DEFINES += DEBUG_HS_ON
#DEFINES += DEBUG_HS1_ON
#DEFINES += DEBUG_PB_ON
#DEFINES += DEBUG_MAP_ON
#DEFINES += DEBUG_SEGMENT_ON
#DEFINES += DEBUG_DELETE
# DEFINES += DEBUG_AG_POL2    # for debug of policy

#DEFINES += DEBUG_SHS_ON
DEFINES += DEBUG_POMCP_ON
DEFINES += DEBUG_POMCP_SIM_ON #show all simulations

DEFINES += SEEKERHS_CHECKS #to check seekerhs in GameConnector


#DEFINES += HSSOLVER_ACTSTATE_ON
    #ag130320: enable keeping track of actual state (y), but not always known neither used, only for evaluation of system -> disabled


# DEFINES += DEBUG_SHOW_TRANS_ON

# DEFINES += DEBUG_COMPARE_TOP_BOTTOM
#DEFINES += DEBUG_CHECK_TOP

#- show transition probs
# DEBUG_DELETE - delete

# appl directory
APPL_DIR = /home/agoldhoorn/Projects/appl-0.95/src
#${HOME}/Projects/appl-0.95/src


#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/momdp/
} else {
    DESTDIR = build/release/momdp/
}
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui



# application
TEMPLATE = app

#Qt packages
QT += network
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
    src/gameconnector.h \
    src/Solver/hsmomdp.h \
    src/Solver/hsmomdp_layered.h \
    src/Solver/hsmomdp_runpolicy.h \
    src/Solver/hssolver.h \
    src/Utils/generic.h \
    src/Segment/kmeanssegmenter.h \
    src/HSMOMDPLoader/hsmomdploader.h \
    src/Segment/robotcenteredsegmentation.h \
    src/Segment/basesegmenter.h \
    src/hsglobaldata.h \
    src/Segment/testsegmenter.h \
    src/Segment/combinecenteredsegmentation.h \
    src/autoplayer.h \
    src/seekerhs.h \
        ./src/mcvi/ParticlesBelief.h \
        ./src/mcvi/RandSource.h \
        ./src/mcvi/PolicyGraph.h \
        ./src/mcvi/MState.h \
        ./src/mcvi/MAction.h \
        ./src/mcvi/MBeliefNode.h \
        ./src/mcvi/MUtils.h \
        ./src/mcvi/ValueIteration.h \
        ./src/mcvi/MBeliefForest.h \
        ./src/mcvi/MObs.h \
        ./src/mcvi/MBelief.h \
        ./src/mcvi/History.h \
        ./src/mcvi/Simulator.h \
        ./src/mcvi/MBeliefSet.h \
        ./src/mcvi/MActNode.h \
        ./src/mcvi/Solver.h \
        ./src/mcvi/MBounds.h \
        ./src/mcvi/MInclude.h \
        ./src/mcvi/hideseekDiscrete/HideSeekModel.h \
        ./src/mcvi/Particle.h \
        ./src/mcvi/Model.h \
        ./src/mcvi/ParticlesBeliefSet.h \
        ./src/mcvi/MObsEdge.h \
        ./src/mcvi/MBeliefTree.h \
    src/Utils/hsexception.h \
    src/mcvi/hideseekDiscrete/hideseekmcvi.h \
    src/Smart/smartseeker.h \
    src/POMCP/node.h \
    src/POMCP/simulator.h \
    src/POMCP/hssimulator.h \
    src/POMCP/state.h \
    src/POMCP/hsstate.h \
    src/POMCP/belief.h \
    src/POMCP/mcts.h \
    src/POMCP/history.h \
    src/POMCP/hspomcp.h \
    src/Utils/readwriteimage.h



SOURCES += src/main_hideseek.cpp \
    src/HSGame/gmap.cpp \
    src/HSGame/gplayer.cpp \
    src/PathPlan/exceptions.cpp \
    src/PathPlan/map_grid/map_grid.cpp \
    src/PathPlan/map_grid/ppm_image_map.cpp \
    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Segment/segment.cpp \    
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/gameconnector.cpp \
    src/Solver/hsmomdp.cpp \
    src/Solver/hsmomdp_layered.cpp \
    src/Solver/hsmomdp_runpolicy.cpp \
    src/Solver/hssolver.cpp \
    src/Utils/generic.cpp \
    src/Segment/kmeanssegmenter.cpp \
    src/HSMOMDPLoader/hsmomdploader.cpp \
    src/Segment/robotcenteredsegmentation.cpp \
    src/Segment/basesegmenter.cpp \
    src/hsglobaldata.cpp \
    src/Segment/testsegmenter.cpp \
    src/Segment/combinecenteredsegmentation.cpp \
    src/seekerhs.cpp \
        ./src/mcvi/MBeliefTree.cc \
        ./src/mcvi/ParticlesBelief.cc \
        ./src/mcvi/MObsEdge.cc \
        ./src/mcvi/MObs.cc \
        ./src/mcvi/Solver.cc \
        ./src/mcvi/ParticlesBeliefSet.cc \
        ./src/mcvi/PolicyGraph.cc \
        ./src/mcvi/ValueIteration.cc \
        ./src/mcvi/MBeliefForest.cc \
        ./src/mcvi/mActNode.cc \
        ./src/mcvi/Simulator.cc \
        ./src/mcvi/MBounds.cc \
#        ./src/mcvi/hideseekDiscrete/HideSeekSolver.cc \
        ./src/mcvi/hideseekDiscrete/HideSeekSimulator.cc \
        ./src/mcvi/hideseekDiscrete/HideSeekModel.cc \
        ./src/mcvi/MAction.cc \
    src/mcvi/hideseekDiscrete/hideseekmcvi.cpp \
    src/Smart/smartseeker.cpp \
    src/POMCP/node.cpp \
    src/POMCP/simulator.cpp \
    src/POMCP/hssimulator.cpp \
    src/POMCP/state.cpp \
    src/POMCP/hsstate.cpp \
    src/POMCP/belief.cpp \
    src/POMCP/mcts.cpp \
    src/POMCP/history.cpp \
    src/POMCP/hspomcp.cpp \
    src/Utils/readwriteimage.cpp



#APPL lib
LIBS += -L$${APPL_DIR} -lappl

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
LIBS += -L/usr/local/lib -lopencv_core  -Llib -liriutils # -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann
INCLUDEPATH += /usr/local/include/opencv \
    /usr/local/include \
    include/exceptions \
    src \
    src/mcvi


#iri utils
# LIBS += -L/usr/local/lib/iridrivers -liriutils



OTHER_FILES += \
    hs_todo.txt




