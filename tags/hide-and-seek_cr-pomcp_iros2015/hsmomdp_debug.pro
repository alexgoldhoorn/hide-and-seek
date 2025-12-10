# Hide&Seek Qt project file
# uses APPL 0.95
# (info on qmake pro files: http://qt-project.org/doc/qt-4.8/qmake-variable-reference.html)
# extra C++ compiler flags (copied from appl)
# -w -O3 $(INCDIR) -msse2 -mfpmath=sse

# AG121224: change compiler to 4.6
#           since on 4.7 it does complain about APPL and Boost files
# AG131230: changed to newest compiler again (4.8) since we don't compile APPL here, we use the library
#           and we need 4.8 for C++ features: random distributions
QMAKE_CC = gcc-4.8
QMAKE_CXX = g++-4.8

#AG150123: removed -fpermissive
QMAKE_CXXFLAGS += -O3 \
    #-msse2 \ # enabled by default
    #-mfpmath=sse \
    #-fpermissive \
    -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable
    #    -pg
# -pg = GProfiler

QMAKE_CFLAGS += -O3 #\
#-msse2 \
#-mfpmath=sse \
#-fpermissive \
#    -pg

#QMAKE_LFLAGS += -pg

#NDEBUG is to ignore asserts !
#DEFINES += NDEBUG


DEFINES += USE_QT

# debug of H&S
#DEFINES += DEBUG_HS_INIT_ON
#DEFINES += DEBUG_HS_ON
#DEFINES += DEBUG_HS1_ON
#DEFINES += DEBUG_PB_ON
#DEFINES += DEBUG_MAP_ON
#DEFINES += DEBUG_SEGMENT_ON
#DEFINES += DEBUG_DELETE
# DEFINES += DEBUG_AG_POL2    # for debug of policy

DEFINES += DEBUG_SHS_ON

DEFINES += DEBUG_POMCP_ON
#DEFINES += DEBUG_POMCP_SIM_ON #show all simulations
#DEFINES += DEBUG_POMCP_DETAIL_ON
#DEFINES += DEBUG_POMCP_LAST_VALUE

DEFINES += DEBUG_PEOPLEPRED_ON

DEFINES += SEEKERHS_CHECKS #to check seekerhs in GameConnector

DEFINES += GUI_DEBUG
DEFINES += DEBUG_CLIENT_ON

DEFINES += USE_POMCP_CONST_CHECK


#DEFINES += HSSOLVER_ACTSTATE_ON
    #ag130320: enable keeping track of actual state (y), but not always known neither used, only for evaluation of system -> disabled


# DEFINES += DEBUG_SHOW_TRANS_ON

# DEFINES += DEBUG_COMPARE_TOP_BOTTOM
#DEFINES += DEBUG_CHECK_TOP

#- show transition probs
# DEBUG_DELETE - delete


#AG140409: use of APPL / MOMDP
#CONFIG+=USE_APPL
USE_APPL {
#nothing
} else {
DEFINES+=DO_NOT_USE_APPL
#AG150123: disabled MOMDP solver (mainly because of include of appl, which gives a great amount of compile errors)
#          also disabled include of APPL headers and static lib
DEFINES += DO_NOT_USE_MOMDP
}

#build dir
CONFIG(debug, debug|release) {
    DESTDIR = build/debug/momdp_debug/
} else {
    DESTDIR = build/release/momdp_debug/
}
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui



# application
TEMPLATE = app

#Qt packages
QT += network
QT += gui
QT += widgets

# --files--
HEADERS += include/HSGame/gmap.h \
    include/HSGame/gplayer.h \
#    include/PathPlan/map_grid/ppm_image_map.h \
#    include/PathPlan/map_grid/map_grid.h \
#    include/PathPlan/astar/astar.h \
    include/PathPlan/pathplanner.h \
    include/Utils/hslog.h \
    include/Utils/timer.h \
    include/hsconfig.h \
    include/gameconnectorclient.h \
    include/Utils/generic.h \
    include/hsglobaldata.h \
    include/autoplayer.h \
    include/seekerhs.h \
##---MCVI---
#    include/mcvi/ParticlesBelief.h \
#    include/mcvi/RandSource.h \
#    include/mcvi/PolicyGraph.h \
#    include/mcvi/MState.h \
#    include/mcvi/MAction.h \
#    include/mcvi/MBeliefNode.h \
#    include/mcvi/MUtils.h \
#    include/mcvi/ValueIteration.h \
#    include/mcvi/MBeliefForest.h \
#    include/mcvi/MObs.h \
#    include/mcvi/MBelief.h \
#    include/mcvi/History.h \
#    include/mcvi/Simulator.h \
#    include/mcvi/MBeliefSet.h \
#    include/mcvi/MActNode.h \
#    include/mcvi/Solver.h \
#    include/mcvi/MBounds.h \
#    include/mcvi/MInclude.h \
#    include/mcvi/hideseekDiscrete/HideSeekModel.h \
#    include/mcvi/Particle.h \
#    include/mcvi/Model.h \
#    include/mcvi/ParticlesBeliefSet.h \
#    include/mcvi/MObsEdge.h \
#    include/mcvi/MBeliefTree.h \
#    include/mcvi/hideseekDiscrete/hideseekmcvi.h \
    #include/mcvi/hideseekDiscrete/hideseekmcvi.h \
##---
    include/Utils/hsexception.h \
    include/Smart/smartseeker.h \
    include/POMCP/node.h \
    include/POMCP/hssimulator.h \
    include/POMCP/simulator.h \
    include/POMCP/state.h \
    include/POMCP/hsstate.h \
    include/POMCP/belief.h \
    include/POMCP/mcts.h \
    include/POMCP/history.h \
    include/POMCP/hspomcp.h \
    include/Utils/readwriteimage.h \
    include/AutoHider/randomhider.h \
    include/AutoHider/randomlisthider.h \
    include/AutoHider/smarthider.h \
    include/AutoHider/verysmarthider.h \
    include/AutoHider/autohider.h \
    include/AutoHider/verysmarthider2.h \
#    include/AutoHider/sfmwalkers.h \
    include/gamenoserver.h \
    include/game.h \
    include/HSGame/pos.h \
    include/POMCP/hssimulatorcont.h \
    include/seekerhsparams.h \
    include/autoplayer.h \
    include/Smart/combinedseeker.h \
    include/Smart/follower.h \
    include/PathPlan/propdistplanner.h \
    include/autowalker.h \
    include/PathPlan/astarplanner.h \
    include/HSGame/idpos.h\
    include/Smart/highestbelieffollower.h \
    include/AutoHider/fromlisthider.h \
    include/POMCP/hssimulatorpred.h \
    include/PeoplePrediction/personpathpredconsumer.h \
    include/Utils/normalbivardist.h \
    include/PeoplePrediction/peoplepredictionwrapper.h \
    include/Server/hsserverclientcommunication.h \
    include/Smart/multiseekerhbexplorer.h \
    include/POMCP/treewindow.h \
    include/rungamethread.h \
    include/Utils/storeqttree.h \
    include/GUI/setparamsdialog.h



SOURCES += src/main_hideseek.cpp \
    src/HSGame/gmap.cpp \
    src/HSGame/gplayer.cpp \
#    src/PathPlan/map_grid/map_grid.cpp \
#    src/PathPlan/map_grid/ppm_image_map.cpp \
#    src/PathPlan/astar/astar.cpp \
    src/PathPlan/pathplanner.cpp \
    src/Segment/segment.cpp \
    src/Utils/hslog.cpp \
    src/Utils/timer.cpp \
    src/gameconnectorclient.cpp \
    src/Utils/generic.cpp \
    src/hsglobaldata.cpp \
    src/seekerhs.cpp \
##---MCVI---
#    src/mcvi/MBeliefTree.cc \
#    src/mcvi/ParticlesBelief.cc \
#    src/mcvi/MObsEdge.cc \
#    src/mcvi/MObs.cc \
#    src/mcvi/Solver.cc \
#    src/mcvi/ParticlesBeliefSet.cc \
#    src/mcvi/PolicyGraph.cc \
#    src/mcvi/ValueIteration.cc \
#    src/mcvi/MBeliefForest.cc \
#    src/mcvi/MActNode.cc \
#    src/mcvi/Simulator.cc \
#    src/mcvi/MBounds.cc \
##    src/mcvi/hideseekDiscrete/HideSeekSolver.cc \
#    src/mcvi/hideseekDiscrete/HideSeekSimulator.cc \
#    src/mcvi/hideseekDiscrete/HideSeekModel.cc \
#    src/mcvi/MAction.cc \
#    src/mcvi/hideseekDiscrete/hideseekmcvi.cpp \
#--
    src/Smart/smartseeker.cpp \
    src/autoplayer.cpp \
    src/POMCP/node.cpp \
    src/POMCP/hssimulator.cpp \
    src/POMCP/state.cpp \
    src/POMCP/hsstate.cpp \
    src/POMCP/belief.cpp \
    src/POMCP/mcts.cpp \
    src/POMCP/history.cpp \
    src/POMCP/hspomcp.cpp \
    src/Utils/readwriteimage.cpp \
    src/AutoHider/randomhider.cpp \
    src/AutoHider/randomlisthider.cpp \
    src/AutoHider/smarthider.cpp \
    src/AutoHider/verysmarthider.cpp \
    src/AutoHider/autohider.cpp \
    src/AutoHider/verysmarthider2.cpp \
#    src/AutoHider/sfmwalkers.cpp \
    src/gamenoserver.cpp \
    src/HSGame/pos.cpp \
    src/POMCP/hssimulatorcont.cpp \
    src/seekerhsparams.cpp \
    src/Smart/combinedseeker.cpp \
    src/Smart/follower.cpp \
    src/PathPlan/propdistplanner.cpp \
    src/autowalker.cpp \
    src/PathPlan/astarplanner.cpp \
    src/HSGame/idpos.cpp \
    src/Smart/highestbelieffollower.cpp  \
    src/AutoHider/fromlisthider.cpp \
    src/POMCP/hssimulatorpred.cpp \
    src/PeoplePrediction/personpathpredconsumer.cpp \
    src/Utils/normalbivardist.cpp \
    src/POMCP/simulator.cpp \
    src/PeoplePrediction/peoplepredictionwrapper.cpp \
    src/Smart/multiseekerhbexplorer.cpp \
    src/POMCP/treewindow.cpp \
    src/rungamethread.cpp \
    src/Utils/storeqttree.cpp \
    src/GUI/setparamsdialog.cpp

USE_APPL {
HEADERS+= include/Solver/hsmomdp.h \
    include/Solver/hsmomdp_layered.h \
    include/Solver/hsmomdp_runpolicy.h \
    include/Solver/hssolver.h \
    include/Segment/segment.h \
    include/Segment/kmeanssegmenter.h \
    include/HSMOMDPLoader/hsmomdploader.h \
    include/Segment/robotcenteredsegmentation.h \
    include/Segment/basesegmenter.h \
    include/Segment/testsegmenter.h \
    include/Segment/combinecenteredsegmentation.h

SOURCES+=src/Solver/hsmomdp.cpp \
    src/Solver/hsmomdp_layered.cpp \
    src/Solver/hsmomdp_runpolicy.cpp \
    src/Solver/hssolver.cpp \
    src/Segment/kmeanssegmenter.cpp \
    src/Segment/robotcenteredsegmentation.cpp \
    src/Segment/basesegmenter.cpp \
    src/HSMOMDPLoader/hsmomdploader.cpp \
    src/Segment/testsegmenter.cpp \
    src/Segment/combinecenteredsegmentation.cpp

    # appl directory
    APPL_DIR = ext_include/appl-0.95
    #${HOME}/Projects/appl-0.95/src


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
    #APPL
    LIBS += -Llib -L../lib -lappl
}


#IRI: libutils + people prediction
LIBS += -L/usr/local/lib/iridrivers -liriutils #-lpeople_prediction
#OpenCV
LIBS += -L/usr/local/lib -L/usr/lib -lopencv_core -lopencv_highgui

#all includes
INCLUDEPATH += /usr/local/include/opencv \
      /usr/local/include \
      src \
      include \
      /usr/include \
      /usr/include/opencv \
      /usr/local/include/iridrivers \
      /usr/include/openc \
      /usr/include/opencv2 \
      ../ext_include/opencv \
      ext_include/opencv \
      ext_include \
      ../ext_include \
      /usr/include/eigen3 \
      /usr/local/include/iridrivers/people_prediction


FORMS += \
    src/POMCP/treewindow.ui \
    src/GUI/setparamsdialog.ui




