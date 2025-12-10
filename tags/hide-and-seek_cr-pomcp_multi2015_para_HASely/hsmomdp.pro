# Hide&Seek Qt project file


#AG150615: qttest
#CONFIG+=USE_TEST_HS

#AG150716: gui debug
#CONFIG+=USE_GUI_DEBUG


################## COMPILER FLAGS ##################

# AG121224: change compiler to 4.6
#           since on 4.7 it does complain about APPL and Boost files
# AG131230: changed to newest compiler again (4.8) since we don't compile APPL here, we use the library
#           and we need 4.8 for C++ features: random distributions
QMAKE_CC = gcc-5 #4.8
QMAKE_CXX = g++-5 #4.8

#AG151118: define usage C++11
CONFIG += c++11

#AG150123: removed -fpermissive
QMAKE_CXXFLAGS += -O3 \
    #-msse2 \ # enabled by default
    #-mfpmath=sse \
    #-fpermissive \
    -std=c++11 \
    -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable
    #-pg
 #-pg = GProfiler

QMAKE_CFLAGS += -O3 #\
#-msse2 \
#-mfpmath=sse \
#-fpermissive \
#    -pg

#QMAKE_LFLAGS += -pg


################## define flags ##################

#NDEBUG is to ignore asserts !
#DEFINES += NDEBUG


DEFINES += USE_QT

DEFINES += DEBUG_KF_ON
#DEFINES += DO_NOT_WRITE_BELIEF_IMG

# debug of H&S
#DEFINES += DEBUG_HS_INIT_ON

#DEFINES += DEBUG_HS_ON

#DEFINES += DEBUG_HS1_ON
#DEFINES += DEBUG_PB_ON
#DEFINES += DEBUG_MAP_ON
#DEFINES += DEBUG_SEGMENT_ON
#DEFINES += DEBUG_DELETE
# DEFINES += DEBUG_AG_POL2    # for debug of policy

#DEFINES += DEBUG_SHS_ON
#DEFINES += DEBUG_POMCP_ON
#DEFINES += DEBUG_CLIENT_ON


#DEFINES += DEBUG_POMCP_SIM_ON #show all simulations
#DEFINES += DEBUG_POMCP_DETAIL_ON

##DEFINES += SEEKERHS_CHECKS #to check seekerhs in GameConnector

DEFINES += DEBUG_CHECK_INF_LOOP_ON

DEFINES += DEBUG_PEOPLEPRED_ON

DEFINES += DEBUG_ASSUMEPERFECTCONNECTION #only for test, to be removed when no communication can occur

#DEFINES += HSSOLVER_ACTSTATE_ON
#ag130320: enable keeping track of actual state (y), but not always known neither used, only for evaluation of system -> disabled


# DEFINES += DEBUG_SHOW_TRANS_ON

# DEFINES += DEBUG_COMPARE_TOP_BOTTOM
#DEFINES += DEBUG_CHECK_TOP

#- show transition probs
# DEBUG_DELETE - delete


#AG150617: to debug the rest
#DEFINES += DO_NOT_USE_SEEKERSH_IN_GAMECONNECTOR


################## CONFIG ##################
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


#AG140409: use of APPL / MOMDP
#CONFIG+=USE_APPL
USE_APPL {
#nothing
} else {
#AG150123: disabled MOMDP solver (mainly because of include of appl, which gives a great amount of compile errors)
#          also disabled include of APPL headers and static lib
DEFINES += DO_NOT_USE_MOMDP
}

# application
TEMPLATE = app

#Qt packages
QT += network
USE_GUI_DEBUG {
    QT += gui
    QT += widgets
    DEFINES += GUI_DEBUG
} else {
    QT -= gui
}

#USE_TEST_HS {
    QT += testlib
#}


################## SOURCES ##################
# --files--
HEADERS += include/AutoHider/autohider.h \
        include/AutoHider/fromlisthider.h \
        include/AutoHider/randomhider.h \
        include/AutoHider/randomlisthider.h \
        include/AutoHider/smarthider.h \
        include/AutoHider/verysmarthider.h \
        include/AutoHider/verysmarthider2.h \
        include/AutoHider/autowalker.h \
#       include/AutoHider/sfmwalkers.h \
        include/Base/autoplayer.h \
        include/Base/gameconnectorclient.h \
        include/Base/game.h \
        include/Base/hsconfig.h \
        include/Base/hsglobaldata.h \
        include/Base/playerinfo.h \
        include/Base/playerinfowrapper.h \
        include/Base/playerinfoserver.h \
        include/Base/posxy.h \
        include/Base/seekerhs.h \
        include/Base/seekerhslog.h \
        include/Base/seekerhsparams.h \
        include/HSGame/gmap.h \
        include/HSGame/gplayer.h \
        include/HSGame/idpos.h \
        include/HSGame/pos.h \
        include/HSGame/bpos.h \
        include/POMCP/belief.h \
        include/POMCP/history.h \
        include/POMCP/hsobservation.h \
        include/POMCP/hspomcp.h \
        include/POMCP/hssimulator.h \
        include/POMCP/hssimulatorcont.h \
        include/POMCP/hssimulatorpred.h \
        include/POMCP/hsstate.h \
        include/POMCP/mcts.h \
        include/POMCP/node.h \
        include/POMCP/observation.h \
        include/POMCP/simulator.h \
        include/POMCP/state.h \
        include/PathPlan/astarplanner.h \
        include/PathPlan/pathplanner.h \
        include/PathPlan/propdistplanner.h \
#       include/PathPlan/map_grid/ppm_image_map.h \
#       include/PathPlan/map_grid/map_grid.h \
#       include/PathPlan/astar/astar.h \
        include/PeoplePrediction/peoplepredictionwrapper.h \
        include/PeoplePrediction/personpathpredconsumer.h \
        include/Server/hsserverclientcommunication.h \
        include/Smart/combinedseeker.h \
        include/Smart/follower.h \
        include/Smart/highestbelieffollower.h \
        include/Smart/multiseekerhbexplorer.h \
        include/Smart/smartseeker.h \
        include/Smart/twoseekerhbexplorer.h \
        include/Utils/generic.h \
        include/Utils/hsexception.h \
        include/Utils/hslog.h \
        include/Utils/normalbivardist.h \
        include/Utils/readwriteimage.h \
        include/Utils/timer.h \
        include/Utils/consoleutils.h \
        include/Filter/kalmanfilter.h \
        include/Filter/kalmanfilterplayer.h\
        include/Filter/particlefilter.h

##---MCVI---##
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

  SOURCES += src/AutoHider/autohider.cpp \
        src/AutoHider/fromlisthider.cpp \
        src/AutoHider/randomhider.cpp \
        src/AutoHider/randomlisthider.cpp \
        src/AutoHider/smarthider.cpp \
        src/AutoHider/verysmarthider.cpp \
        src/AutoHider/verysmarthider2.cpp \
        src/AutoHider/autowalker.cpp \
#       src/AutoHider/sfmwalkers.cpp \
        src/Base/autoplayer.cpp \
        src/Base/gameconnectorclient.cpp \
        src/Base/hsglobaldata.cpp \
        src/Base/playerinfo.cpp \
        src/Base/playerinfowrapper.cpp \
        src/Base/playerinfoserver.cpp \
        src/Base/posxy.cpp \
        src/Base/seekerhs.cpp \
        src/Base/seekerhsparams.cpp \
        src/Base/seekerhslog.cpp \
        src/HSGame/gmap.cpp \
        src/HSGame/gplayer.cpp \
        src/HSGame/idpos.cpp \
        src/HSGame/pos.cpp \
        src/HSGame/bpos.cpp \
        src/POMCP/belief.cpp \
        src/POMCP/history.cpp \
        src/POMCP/hsobservation.cpp \
        src/POMCP/hspomcp.cpp \
        src/POMCP/hssimulator.cpp \
        src/POMCP/hssimulatorcont.cpp \
        src/POMCP/hssimulatorpred.cpp \
        src/POMCP/hsstate.cpp \
        src/POMCP/mcts.cpp \
        src/POMCP/node.cpp \
        src/POMCP/observation.cpp \
        src/POMCP/simulator.cpp \
        src/POMCP/state.cpp \
        src/PathPlan/astarplanner.cpp \
        src/PathPlan/pathplanner.cpp \
        src/PathPlan/propdistplanner.cpp \
#    src/PathPlan/map_grid/map_grid.cpp \
#    src/PathPlan/map_grid/ppm_image_map.cpp \
#    src/PathPlan/astar/astar.cpp \
        src/PeoplePrediction/peoplepredictionwrapper.cpp \
        src/PeoplePrediction/personpathpredconsumer.cpp \
        src/Segment/segment.cpp \
        src/Smart/combinedseeker.cpp \
        src/Smart/follower.cpp \
        src/Smart/highestbelieffollower.cpp  \
        src/Smart/multiseekerhbexplorer.cpp \
        src/Smart/smartseeker.cpp \
        src/Smart/twoseekerhbexplorer.cpp \
        src/Utils/generic.cpp \
        src/Utils/hslog.cpp \
        src/Utils/normalbivardist.cpp \
        src/Utils/readwriteimage.cpp \
        src/Utils/timer.cpp \
        src/Filter/kalmanfilter.cpp \
        src/Filter/kalmanfilterplayer.cpp \
        src/Filter/particlefilter.cpp

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
#    src/mcvi/hideseekDiscrete/HideSeekSolver.cc \
#    src/mcvi/hideseekDiscrete/HideSeekSimulator.cc \
#    src/mcvi/hideseekDiscrete/HideSeekModel.cc \
#    src/mcvi/MAction.cc \
#    src/mcvi/hideseekDiscrete/hideseekmcvi.cpp \

OTHER_FILES += CMakeLists.txt \
    src/CMakeLists.txt \
    todo.txt \
    hsserver-client-communication-def.txt \
    README.txt \
    robot_params.txt \
    versions.txt

USE_APPL {
HEADERS+=        include/HSMOMDPLoader/hsmomdploader.h \
        include/Segment/basesegmenter.h \
        include/Segment/combinecenteredsegmentation.h \
        include/Segment/kmeanssegmenter.h \
        include/Segment/robotcenteredsegmentation.h \
        include/Segment/segment.h \
        include/Segment/testsegmenter.h \
        include/Solver/hsmomdp_layered.h \
        include/Solver/hsmomdp_runpolicy.h \
        include/Solver/hssolver.h
        include/Solver/hsmomdp.h
SOURCES+=        src/HSMOMDPLoader/hsmomdploader.cpp \
        src/Segment/basesegmenter.cpp \
        src/Segment/combinecenteredsegmentation.cpp \
        src/Segment/kmeanssegmenter.cpp \
        src/Segment/robotcenteredsegmentation.cpp \
        src/Segment/testsegmenter.cpp \
        src/Solver/hsmomdp_layered.cpp \
        src/Solver/hsmomdp_runpolicy.cpp \
        src/Solver/hssolver.cpp
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

#AG150703: test sources, since we might want to test from the default program
SOURCES += test/src/HSGame/pos-test.cpp \
    test/src/HSGame/gmap-test.cpp \
    test/src/POMCP/mcts-test.cpp \
    test/src/POMCP/hsstate-test.cpp \
    test/src/POMCP/hsobservation-test.cpp \
    test/src/POMCP/node-test.cpp \
    test/src/POMCP/simulator-test.cpp \
    test/src/Base/autoplayer-test.cpp \
    test/src/Server/clientsocket-test.cpp \
    src/Server/clientsocket.cpp \
    test/src/Smart/multiseekerhbexplorer-test.cpp \
    src/Base/abstractautoplayer.cpp \
    test/src/Base/playerinfowrapper-test.cpp

HEADERS += test/include/HSGame/pos-test.h \
    test/include/HSGame/gmap-test.h \
    test/include/POMCP/hsstate-test.h \
    test/include/POMCP/hsobservation-test.h \
    test/include/POMCP/node-test.h \
    test/include/POMCP/simulator-test.h \
    test/include/POMCP/mcts-test.h \
    test/include/Base/autoplayer-test.h \
    test/include/Server/clientsocket-test.h \
    include/Server/clientsocket.h \
    test/include/Smart/multiseekerhbexplorer-test.h \
    include/Base/abstractautoplayer.h \
    test/include/Base/playerinfowrapper-test.h


USE_TEST_HS {
    SOURCES += test/src/main_tesths.cpp
} else {
    SOURCES += src/main_hideseek.cpp
}

USE_GUI_DEBUG {
    HEADERS += include/POMCP/treewindow.h \
        include/rungamethread.h \
        include/Utils/storeqttree.h \
        include/GUI/setparamsdialog.h
    SOURCES += src/POMCP/treewindow.cpp \
        src/rungamethread.cpp \
        src/Utils/storeqttree.cpp \
        src/GUI/setparamsdialog.cpp


    FORMS += src/POMCP/treewindow.ui \
        src/GUI/setparamsdialog.ui
}

################## LIBS and INCLUDES ##################

#IRI: libutils + people prediction
LIBS += -L/usr/local/lib/iridrivers -liriutils #-lpeople_prediction
#OpenCV
LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui
LIBS += -lopencv_imgproc -lopencv_imgcodecs  # FOR OpenCV 3 !!
#LIBS += -L/usr/local/lib -L/usr/lib -L/usr/lib/x86_64-linux-gnu -lopencv_core -lopencv_imgproc #-lopencv_highgui
#all includes
INCLUDEPATH += /usr/local/include \
      src \
      include \
#      /usr/include \
      /usr/local/include/iridrivers \
#      /usr/local/include/opencv \
#      /usr/include/opencv \
#      /usr/include/openc \
#      /usr/include/opencv2 \
#      ../ext_include/opencv \
#      ext_include/opencv \
      ext_include \
      ../ext_include \
      /usr/include/eigen3
#\      /usr/local/include/iridrivers/people_prediction

