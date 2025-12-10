
#AG150123: removed

set(headers         
AutoHider/autohider.h
#        AutoHider/fromlisthider.h
#        AutoHider/randomhider.h
#        AutoHider/randomlisthider.h
#        AutoHider/smarthider.h
#        AutoHider/verysmarthider.h
#        AutoHider/verysmarthider2.h
#        AutoHider/autowalker.h
#       AutoHider/sfmwalkers.h
        Base/autoplayer.h
#        Base/gameconnectorclient.h
        Base/game.h
        Base/hsconfig.h
        Base/hsglobaldata.h
        Base/playerinfo.h
#        Base/playerinfowrapper.h
#        Base/playerinfoserver.h
        Base/posxy.h
        Base/seekerhs.h
        Base/seekerhslog.h
        Base/seekerhsparams.h
        HSGame/gmap.h
        HSGame/gplayer.h
        HSGame/idpos.
        HSGame/pos.h
        HSGame/bpos.h
        POMCP/belief.h
        POMCP/history.h
        POMCP/hsobservation.h
        POMCP/hspomcp.h
        POMCP/hssimulator.h
        POMCP/hssimulatorcont.h
        POMCP/hssimulatorpred.h
        POMCP/hsstate.h
        POMCP/mcts.h
        POMCP/node.h
        POMCP/observation.h
        POMCP/simulator.h
        POMCP/state.h
        PathPlan/astarplanner.h
        PathPlan/pathplanner.h
        PathPlan/propdistplanner.h
#       PathPlan/map_grid/ppm_image_map.h
#       PathPlan/map_grid/map_grid.h
#       PathPlan/astar/astar.h
        PeoplePrediction/peoplepredictionwrapper.h
        PeoplePrediction/personpathpredconsumer.h
#        Server/hsserverclientcommunication.h
        Smart/combinedseeker.h
        Smart/follower.h
        Smart/highestbelieffollower.h
        Smart/multiseekerhbexplorer.h
        Smart/smartseeker.h
        Smart/twoseekerhbexplorer.h
        Utils/generic.h
        Utils/hsexception.h
        Utils/hslog.h
        Utils/normalbivardist.h
        Utils/readwriteimage.h
        Utils/timer.h
        Utils/consoleutils.h
        Filter/kalmanfilter.h 
        Filter/kalmanfilterplayer.h
        Filter/particlefilter.h
        Segment/basesegmenter.h 
        Segment/combinecenteredsegmentation.h 
        Segment/kmeanssegmenter.h 
        Segment/robotcenteredsegmentation.h 
        Segment/segment.h 
        Segment/testsegmenter.h 
)

set(headers "${../include/${headers}}")       

set(sources         
#    AutoHider/autohider.cpp
#        AutoHider/fromlisthider.cpp
#        AutoHider/randomhider.cpp
#        AutoHider/randomlisthider.cpp
#        AutoHider/smarthider.cpp
#        AutoHider/verysmarthider.cpp
#        AutoHider/verysmarthider2.cpp
#        AutoHider/autowalker.cpp
#       AutoHider/sfmwalkers.cpp
        Base/autoplayer.cpp
#       Base/gameconnectorclient.cpp
        Base/hsglobaldata.cpp
        Base/playerinfo.cpp
        Base/playerinfowrapper.cpp
#        Base/playerinfoserver.cpp
        Base/posxy.cpp
        Base/seekerhs.cpp
        Base/seekerhsparams.cpp
        Base/seekerhslog.cpp
        HSGame/gmap.cpp
        HSGame/gplayer.cpp
        HSGame/idpos.cpp
        HSGame/pos.cpp
        HSGame/bpos.cpp
        POMCP/belief.cpp
        POMCP/history.cpp
        POMCP/hsobservation.cpp
        POMCP/hspomcp.cpp
        POMCP/hssimulator.cpp
        POMCP/hssimulatorcont.cpp
        POMCP/hssimulatorpred.cpp
        POMCP/hsstate.cpp
        POMCP/mcts.cpp
        POMCP/node.cpp
        POMCP/observation.cpp
        POMCP/simulator.cpp
        POMCP/state.cpp
        PathPlan/astarplanner.cpp
        PathPlan/pathplanner.cpp
        PathPlan/propdistplanner.cpp
#    PathPlan/map_grid/map_grid.cpp
#    PathPlan/map_grid/ppm_image_map.cpp
#    PathPlan/astar/astar.cpp
        PeoplePrediction/peoplepredictionwrapper.cpp
        PeoplePrediction/personpathpredconsumer.cpp
        Segment/segment.cpp
        Smart/combinedseeker.cpp
        Smart/follower.cpp
        Smart/highestbelieffollower.cpp
        Smart/multiseekerhbexplorer.cpp
        Smart/smartseeker.cpp
        Smart/twoseekerhbexplorer.cpp
        Utils/generic.cpp
        Utils/hslog.cpp
        Utils/normalbivardist.cpp
        Utils/readwriteimage.cpp
        Utils/timer.cpp
        Filter/kalmanfilter.cpp 
        Filter/kalmanfilterplayer.cpp 
        Filter/particlefilter.cpp
        Segment/basesegmenter.cpp
        Segment/combinecenteredsegmentation.cpp 
        Segment/kmeanssegmenter.cpp 
        Segment/robotcenteredsegmentation.cpp 
        Segment/segment.cpp 
        Segment/testsegmenter.cpp 
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
