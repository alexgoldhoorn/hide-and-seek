
set(headers         
	#Server/hsserver.h
        #Server/hsserverconfig.h
        #Server/hsgamelogdb.h
        #Server/hsgamelog.h
        #Server/hstcpserver.h
        POMCP/hsstate.h
        POMCP/belief.h
        POMCP/simulator.h
        POMCP/state.h
        POMCP/history.h
        POMCP/hspomcp.h
        POMCP/hssimulator.h
        POMCP/mcts.h
        POMCP/node.h
        hsconfig.h
        HSGame/gmap.h
        HSGame/gplayer.h
        gameconnector.h
        seekerhs.h
        autoplayer.h
#        GUI/gmapwidget.h
#        GUI/simmaingui.h
#        GUI/popup.h
        Segment/testsegmenter.h
        Segment/kmeanssegmenter.h
        Segment/basesegmenter.h
        Segment/robotcenteredsegmentation.h
        Segment/segment.h
        Segment/combinecenteredsegmentation.h
#        mcvi/MBelief.h
#        mcvi/ParticlesBelief.h
#        mcvi/RandSource.h
#        mcvi/MBeliefForest.h
#        mcvi/MObs.h
#        mcvi/PolicyGraph.h
#        mcvi/State.h
#        mcvi/MBeliefNode.h
#        mcvi/MActNode.h
#        mcvi/ValueIteration.h
#        mcvi/MState.h
#        mcvi/History.h
#        mcvi/Simulator.h
#        mcvi/Solver.h
#        mcvi/MBeliefTree.h
#        mcvi/MBounds.h
#        mcvi/hideseekDiscrete/hideseekmcvi.h
#        mcvi/hideseekDiscrete/HideSeekModel.h
#        mcvi/Particle.h
#        mcvi/Model.h
#        mcvi/MObsEdge.h
#        mcvi/ParticlesBeliefSet.h
#        mcvi/MAction.h
#        mcvi/MUtils.h
#        mcvi/MInclude.h
#        mcvi/MBeliefSet.h
        Solver/hsmomdp.h
        Solver/hsmomdp_layered.h
        Solver/hssolver.h
        Solver/hsmomdp_runpolicy.h
        PathPlan/astar/astar.h
        PathPlan/pathplanner.h
        PathPlan/map_grid/map_grid.h
        PathPlan/map_grid/ppm_image_map.h
        PathPlan/exceptions.h
#        HSMOMDPLoader/hsmomdploader.h
        Smart/smartseeker.h
        AutoHider/randomhider.h
#        AutoHider/gmapwidget.h
#        AutoHider/main_autohider.h
#        AutoHider/randomlisthider.h
        AutoHider/smarthider.h
        AutoHider/autohider.h
        AutoHider/verysmarthider.h
#        AutoHider/gameconnectorhider.h
#        ActionListGenerator/actionlistgenerator.h
#        RandomTester/randomtester.h
        hsglobaldata.h
#        Utils/hsexception.h
        Utils/generic.h
        Utils/hslog.h
        Utils/timer.h
	Utils/readwriteimage.h
)

set(sources         
	#Server/hsgamelog.cpp
        #Server/hstcpserver.cpp
        #Server/hsserverconfig.cpp
        #Server/hsserver.cpp
        #Server/hsgamelogdb.cpp
        POMCP/hsstate.cpp
        POMCP/state.cpp
        POMCP/node.cpp
        POMCP/history.cpp
        POMCP/belief.cpp
        POMCP/hssimulator.cpp
        POMCP/hspomcp.cpp
        POMCP/mcts.cpp
        POMCP/simulator.cpp
        hsglobaldata.cpp
        HSGame/gplayer.cpp
        HSGame/gmap.cpp
#        GUI/simmaingui.cpp
#        GUI/gmapwidget.cpp
#        GUI/popup.cpp
#        GUI/main_gui.cpp
        Segment/testsegmenter.cpp
        Segment/kmeanssegmenter.cpp
        Segment/segment.cpp
        Segment/combinecenteredsegmentation.cpp
        Segment/robotcenteredsegmentation.cpp
        Segment/basesegmenter.cpp
#        mcvi/ParticlesBelief.cc
#        mcvi/MBounds.cc
#        mcvi/MBeliefForest.cc
#        mcvi/MObsEdge.cc
#        mcvi/Solver.cc
#        mcvi/MBeliefTree.cc
#        mcvi/ParticlesBeliefSet.cc
#        mcvi/PolicyGraph.cc
#        mcvi/MAction.cc
#        mcvi/ValueIteration.cc
#        mcvi/MObs.cc
#        mcvi/Simulator.cc
#        mcvi/hideseekDiscrete/HideSeekSolver.cc
#        mcvi/hideseekDiscrete/HideSeekSimulator.cc
#        mcvi/hideseekDiscrete/hideseekmcvi.cpp
#        mcvi/hideseekDiscrete/HideSeekModel.cc
#        mcvi/MActNode.cc
        Solver/hsmomdp_runpolicy.cpp
        Solver/hssolver.cpp
        Solver/hsmomdp.cpp
        Solver/hsmomdp_layered.cpp
#        gameconnector.cpp
        seekerhs.cpp
        PathPlan/astar/astar.cpp
        PathPlan/pathplanner.cpp
        PathPlan/map_grid/ppm_image_map.cpp
        PathPlan/map_grid/map_grid.cpp
        PathPlan/exceptions.cpp
#        HSMOMDPLoader/hsmomdploader.cpp
        Smart/smartseeker.cpp
#        main_hideseek.cpp
#        main_server.cpp
#        AutoHider/randomlisthider.cpp
        AutoHider/autohider.cpp
#        AutoHider/gameconnectorhider.cpp
        AutoHider/randomhider.cpp
        AutoHider/smarthider.cpp
#        AutoHider/gmapwidget.cpp
#        AutoHider/main_autohider.cpp
        AutoHider/verysmarthider.cpp
#        ActionListGenerator/actionlistgenerator.cpp
#        ActionListGenerator/main_actionlistgen.cpp
#        RandomTester/main_randomtester.cpp
#        RandomTester/randomtester.cpp
        Utils/generic.cpp
        Utils/hslog.cpp
        Utils/timer.cpp
#        Utils/hsexception.cpp
	Utils/readwriteimage.cpp
)



set(headersconv       
        hsglobaldata.h  
        hsconfig.h
        HSGame/gmap.h
        HSGame/gplayer.h
        PathPlan/astar/astar.cpp
        PathPlan/pathplanner.cpp
        PathPlan/map_grid/ppm_image_map.cpp
        PathPlan/map_grid/map_grid.cpp
        PathPlan/exceptions.cpp
        hsglobaldata.h
        Utils/generic.h
        Utils/hslog.h
        Utils/timer.h
	Utils/readwriteimage.h
)

set(sourcesconv
	main_map_convert.cpp      
        hsglobaldata.cpp
        HSGame/gplayer.cpp
        HSGame/gmap.cpp
        PathPlan/astar/astar.cpp
        PathPlan/pathplanner.cpp
        PathPlan/map_grid/ppm_image_map.cpp
        PathPlan/map_grid/map_grid.cpp
        PathPlan/exceptions.cpp
        Utils/generic.cpp
        Utils/hslog.cpp
        Utils/timer.cpp
	Utils/readwriteimage.cpp
)



INCLUDE_DIRECTORIES(.)


# locate the necessary dependencies
FIND_PACKAGE(OpenCV REQUIRED)

# add the necessary include directories
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIR})

# APPL-0.95 library (should be compiled first in include/appl-0.95)
FIND_LIBRARY(APPL_LIB NAMES appl PATHS ../lib )

INCLUDE_DIRECTORIES(../include/appl-0.95/Models/MOMDP 
	../include/appl-0.95/Bounds 
	../include/appl-0.95/Core 
	../include/appl-0.95/Utils 
	../include/appl-0.95/Utils/boost/config 
	../include/appl-0.95/Utils/boost/smart_ptr 
	../include/appl-0.95/Utils/boost/exception 
	../include/appl-0.95/Utils/boost/detail 
	../include/appl-0.95/Utils/boost/smart_ptr/detail 
	../include/appl-0.95/OfflineSolver 
	../include/appl-0.95/Algorithms 
	../include/appl-0.95/Core 
	../include/appl-0.95/Evaluator 
	../include/appl-0.95/MathLib 
	../include/appl-0.95/Parser 
	../include/appl-0.95/Parser/POMDPX 
	../include/appl-0.95/Parser/Cassandra 
	../include/appl-0.95/Algorithms 
	../include/appl-0.95/Algorithms/SARSOP
)

FIND_PACKAGE(iriutils REQUIRED)
INCLUDE_DIRECTORIES(${iriutils_INCLUDE_DIR})



ADD_EXECUTABLE(convpgmmap ${sourcesconv} ${headersconv})
TARGET_LINK_LIBRARIES(convpgmmap ${OpenCV_LIBS} ${iriutils_LIBRARY})

ADD_LIBRARY(seekerhs SHARED ${sources} ${headers})
TARGET_LINK_LIBRARIES(seekerhs ${OpenCV_LIBS} ${APPL_LIB} ${iriutils_LIBRARY})

INSTALL(TARGETS seekerhs
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/iridrivers
        ARCHIVE DESTINATION lib/iridrivers)
INSTALL(FILES ${headers} DESTINATION include/iridrivers)
INSTALL(FILES ../Findseekerhs.cmake DESTINATION ${CMAKE_ROOT}/Modules/)
INSTALL(FILES ../lib/libappl.a DESTINATION lib/iridrivers)



