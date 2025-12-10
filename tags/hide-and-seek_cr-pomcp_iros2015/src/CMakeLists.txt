
#AG150123: removed

set(headers         
    HSGame/gmap.h
    HSGame/gplayer.h
#    PathPlan/exceptions.h
#    PathPlan/map_grid/ppm_image_map.h
#    PathPlan/map_grid/map_grid.h
#    PathPlan/astar/astar.h
    PathPlan/pathplanner.h
    PathPlan/propdistplanner.h
    PathPlan/astarplanner.h
    Utils/hslog.h
    Utils/timer.h
    hsconfig.h
#    gameconnectorclient.h
##---MOMDP---
#    Solver/hsmomdp.h
#    Solver/hsmomdp_layered.h
#    Solver/hsmomdp_runpolicy.h
#    Solver/hssolver.h
#    Segment/segment.h
#    Segment/kmeanssegmenter.h
#    HSMOMDPLoader/hsmomdploader.h
#    Segment/robotcenteredsegmentation.h
#    Segment/basesegmenter.h
#    Segment/testsegmenter.h
#    Segment/combinecenteredsegmentation.h
##---
    Utils/generic.h
    hsglobaldata.h
    autoplayer.h
    seekerhs.h
    seekerhsparams.h
##---MCVI---
#    mcvi/ParticlesBelief.h
#    mcvi/RandSource.h
#    mcvi/PolicyGraph.h
#    mcvi/MState.h
#    mcvi/MAction.h
#    mcvi/MBeliefNode.h
#    mcvi/MUtils.h
#    mcvi/ValueIteration.h
#    mcvi/MBeliefForest.h
#    mcvi/MObs.h
#    mcvi/MBelief.h
#    mcvi/History.h
#    mcvi/Simulator.h
#    mcvi/MBeliefSet.h
#    mcvi/MActNode.h
#    mcvi/Solver.h
#    mcvi/MBounds.h
#    mcvi/MInclude.h
#    mcvi/hideseekDiscrete/HideSeekModel.h
#    mcvi/Particle.h
#    mcvi/Model.h
#    mcvi/ParticlesBeliefSet.h
#    mcvi/MObsEdge.h
#    mcvi/MBeliefTree.h
#    mcvi/hideseekDiscrete/hideseekmcvi.h
##---
    Utils/hsexception.h
    Smart/smartseeker.h
    POMCP/node.h
    POMCP/hssimulator.h
    POMCP/simulator.h
    POMCP/state.h
    POMCP/hsstate.h
    POMCP/belief.h
    POMCP/mcts.h
    POMCP/history.h
    POMCP/hspomcp.h
    Utils/readwriteimage.h
#    AutoHider/randomhider.h
#    AutoHider/randomlisthider.h
#    AutoHider/smarthider.h
#    AutoHider/verysmarthider.h
    AutoHider/autohider.h
#    AutoHider/verysmarthider2.h
#    gamenoserver.h
    game.h
    HSGame/pos.h
    POMCP/hssimulatorcont.h
    Smart/combinedseeker.h
    Smart/follower.h
    HSGame/idpos.h
    Smart/highestbelieffollower.h
    POMCP/hssimulatorpred.h
    PeoplePrediction/personpathpredconsumer.h
    Utils/normalbivardist.h
    PeoplePrediction/peoplepredictionwrapper.h
    Smart/multiseekerhbexplorer.h
)

set(headers "${../include/${headers}}")       

set(sources         
    HSGame/gmap.cpp
    HSGame/gplayer.cpp
#    PathPlan/exceptions.cpp
#    PathPlan/map_grid/map_grid.cpp
#    PathPlan/map_grid/ppm_image_map.cpp
#    PathPlan/astar/astar.cpp
    PathPlan/pathplanner.cpp
    PathPlan/propdistplanner.cpp
    PathPlan/astarplanner.cpp
    Segment/segment.cpp
    Utils/hslog.cpp
    Utils/timer.cpp
#    gameconnectorclient.cpp
##---MOMDP---
#    Solver/hsmomdp.cpp
#    Solver/hsmomdp_layered.cpp
#    Solver/hsmomdp_runpolicy.cpp
#    Solver/hssolver.cpp
#    Segment/kmeanssegmenter.cpp
#    HSMOMDPLoader/hsmomdploader.cpp
#    Segment/robotcenteredsegmentation.cpp
#    Segment/basesegmenter.cpp
#    Segment/testsegmenter.cpp
#    Segment/combinecenteredsegmentation.cpp
##---
    hsglobaldata.cpp
    Utils/generic.cpp
    seekerhs.cpp
    seekerhsparams.cpp
##---MCVI---
#    mcvi/MBeliefTree.cc
#    mcvi/ParticlesBelief.cc
#    mcvi/MObsEdge.cc
#    mcvi/MObs.cc
#    mcvi/Solver.cc
#    mcvi/ParticlesBeliefSet.cc
#    mcvi/PolicyGraph.cc
#    mcvi/ValueIteration.cc
#    mcvi/MBeliefForest.cc
#    mcvi/MActNode.cc
#    mcvi/Simulator.cc
#    mcvi/MBounds.cc
#    mcvi/hideseekDiscrete/HideSeekSolver.cc
#    mcvi/hideseekDiscrete/HideSeekSimulator.cc
#    mcvi/hideseekDiscrete/HideSeekModel.cc
#    mcvi/MAction.cc
#    mcvi/hideseekDiscrete/hideseekmcvi.cpp
##---
    Smart/smartseeker.cpp
    autoplayer.cpp
    POMCP/node.cpp
    POMCP/hssimulator.cpp
    POMCP/state.cpp
    POMCP/hsstate.cpp
    POMCP/belief.cpp
    POMCP/mcts.cpp
    POMCP/history.cpp
    POMCP/hspomcp.cpp
    Utils/readwriteimage.cpp
#    AutoHider/randomhider.cpp
#    AutoHider/randomlisthider.cpp
#    AutoHider/smarthider.cpp
#    AutoHider/verysmarthider.cpp
    AutoHider/autohider.cpp
#    AutoHider/verysmarthider2.cpp
#    gamenoserver.cpp
    HSGame/pos.cpp
    POMCP/hssimulatorcont.cpp
    Smart/combinedseeker.cpp
    Smart/follower.cpp
    HSGame/idpos.cpp
    Smart/highestbelieffollower.cpp
    POMCP/hssimulatorpred.cpp
    PeoplePrediction/personpathpredconsumer.cpp
    Utils/normalbivardist.cpp
    POMCP/simulator.cpp
    PeoplePrediction/peoplepredictionwrapper.cpp
    Smart/multiseekerhbexplorer.cpp
)



#set(headersconv
#        hsglobaldata.h
#        hsconfig.h
#        HSGame/gmap.h
#        HSGame/gplayer.h
#        HSGame/pos.h
#        HSGame/idpos.h
##        PathPlan/astar/astar.cpp
#        PathPlan/pathplanner.h
#        PathPlan/propdistplanner.h
##        PathPlan/map_grid/ppm_image_map.cpp
##        PathPlan/map_grid/map_grid.cpp
##        PathPlan/exceptions.cpp
#        hsglobaldata.h
#        Utils/generic.h
#        Utils/hslog.h
#        Utils/timer.h
        #Utils/readwriteimage.h
#)

#set(headersconv "${../include/${headersconv}}")

#set(sourcesconv
#	main_map_convert.cpp
#        hsglobaldata.cpp
#        HSGame/gplayer.cpp
#        HSGame/gmap.cpp
#        HSGame/pos.cpp
#        HSGame/idpos.cpp
##        PathPlan/astar/astar.cpp
#        PathPlan/pathplanner.cpp
#        PathPlan/propdistplanner.cpp
##        PathPlan/map_grid/ppm_image_map.cpp
##        PathPlan/map_grid/map_grid.cpp
##        PathPlan/exceptions.cpp
#        Utils/generic.cpp
#        Utils/hslog.cpp
#        Utils/timer.cpp
        #Utils/readwriteimage.cpp
#)



INCLUDE_DIRECTORIES(../include)

#set(CMAKE_MODULE_PATH ${CMAKE_ROOT}/Modules/)

FIND_PACKAGE(iriutils REQUIRED)
INCLUDE_DIRECTORIES(${iriutils_INCLUDE_DIR})

set(CMAKE_MODULE_PATH /usr/share/OpenCV/)

# locate the necessary dependencies
FIND_PACKAGE(OpenCV REQUIRED)
message(STATUS "${OpenCV_INCLUDE_DIRS}")
#140305: fix of OpenCV, choose OpenCV version (/usr/lib or ros) by changing:
#export CMAKE_PREFIX_PATH=/usr/share/OpenCV/:$CMAKE_PREFIX_PATH
#AG140116: put the OpenCV version that worked, instead of the default ROS fuerte which doesn't compile with g++ 4.8 (C++11)
#FIND_LIBRARY(OpenCV_LIBS NAMES opencv_core PATHS ../lib)
#INCLUDE_DIRECTORIES(../ext_include/opencv)

# add the necessary include directories
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#AG140904: Eigen
find_package(Eigen3 REQUIRED)

include_directories(EIGEN3_INCLUDE_DIR)


#AG150123: disabled include APPL
## APPL-0.95 library (should be compiled first in include/appl-0.95)
#FIND_LIBRARY(APPL_LIB NAMES appl PATHS ../lib )
## WHY does it compile appl lib ... if it should use the library

#INCLUDE_DIRECTORIES(../ext_include/appl-0.95/Models/MOMDP
#	../ext_include/appl-0.95/Bounds
#        ../ext_include/appl-0.95/Core
#        ../ext_include/appl-0.95/Utils
#        ../ext_include/appl-0.95/Utils/boost/config
#        ../ext_include/appl-0.95/Utils/boost/smart_ptr
#        ../ext_include/appl-0.95/Utils/boost/exception
#       ../ext_include/appl-0.95/Utils/boost/detail
#        ../ext_include/appl-0.95/Utils/boost/smart_ptr/detail
#	../ext_include/appl-0.95/OfflineSolver
#	../ext_include/appl-0.95/Algorithms
#	../ext_include/appl-0.95/Core
#	../ext_include/appl-0.95/Evaluator
#	../ext_include/appl-0.95/MathLib
#	../ext_include/appl-0.95/Parser
#	../ext_include/appl-0.95/Parser/POMDPX
#	../ext_include/appl-0.95/Parser/Cassandra
#	../ext_include/appl-0.95/Algorithms
#	../ext_include/appl-0.95/Algorithms/SARSOP
#)



#ADD_EXECUTABLE(convpgmmap ${sourcesconv} ${headersconv})
#TARGET_LINK_LIBRARIES(convpgmmap ${OpenCV_LIBS} ${iriutils_LIBRARY})

ADD_LIBRARY(seekerhs SHARED ${sources} ${headers})
TARGET_LINK_LIBRARIES(seekerhs ${OpenCV_LIBS} ${iriutils_LIBRARY}) #${APPL_LIB}

INSTALL(TARGETS seekerhs
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/iridrivers
        ARCHIVE DESTINATION lib/iridrivers)
INSTALL(DIRECTORY ../include/ 
        DESTINATION include/iridrivers/hide-and-seek
        PATTERN "*.h"
        PATTERN ".svn" EXCLUDE)
INSTALL(FILES ../Findseekerhs.cmake DESTINATION ${CMAKE_ROOT}/Modules/)
#INSTALL(FILES ../lib/libappl.a DESTINATION lib/iridrivers)



#ADD_SUBDIRECTORY(examples)
