cmake_minimum_required(VERSION 2.8.11)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0005 NEW)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

PROJECT(fafconnection)


SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

FIND_PACKAGE(Doxygen)

FIND_PATH(IRI_DOC_DIR doxygen.conf ${CMAKE_SOURCE_DIR}/doc/iri_doc/)
IF (IRI_DOC_DIR)
  ADD_CUSTOM_TARGET (doc ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/iri_doc/doxygen.conf)
ELSE (IRI_DOC_DIR)
  ADD_CUSTOM_TARGET (doc ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/doxygen.conf)
ENDIF (IRI_DOC_DIR)

ADD_CUSTOM_TARGET (distclean @echo cleaning cmake files)

set(CMAKE_AUTOMOC ON)

IF (UNIX)
  ADD_CUSTOM_COMMAND(
    COMMENT "distribution clean"
    COMMAND make ARGS clean
    COMMAND rm ARGS -rf ${CMAKE_SOURCE_DIR}/build/*

    TARGET  distclean
  )
ELSE(UNIX)
  ADD_CUSTOM_COMMAND(
    COMMENT "distclean only implemented in unix"
    TARGET  distclean
  )
ENDIF(UNIX)

ADD_CUSTOM_TARGET (uninstall @echo uninstall package)

IF (UNIX)
  ADD_CUSTOM_COMMAND(
    COMMENT "uninstall package"
    COMMAND xargs ARGS rm < install_manifest.txt

    TARGET  uninstall
  )
ELSE(UNIX)
  ADD_CUSTOM_COMMAND(
    COMMENT "uninstall only implemented in unix"
    TARGET  uninstall
  )
ENDIF(UNIX)

#add_definitions( -DDEBUG_ON )


# sources of the different apps

SET(SOURCES  src/qtserver.cpp
            src/qtclient.cpp
            src/fafconnection.cpp
)
SET(HEADERS  include/qtserver.h
            include/qtclient.h
            include/qtscglobal.h
            include/fafconnection.h
            include/fafconnectionhandler.h
)

SET(OTHERS  include/tester.h
            src/tester.cpp
)


# Qt
# Find the QtWidgets library
#find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5Core)



# include files
include_directories(include)
#include_directories(src)

# create execs
ADD_EXECUTABLE(qtserver ${SOURCES} src/qtserver_main.cpp ${HEADERS} ${OTHERS})
TARGET_LINK_LIBRARIES(qtserver Qt5::Network Qt5::Core)

ADD_EXECUTABLE(qtclient ${SOURCES} src/qtclient_main.cpp ${HEADERS} ${OTHERS})
TARGET_LINK_LIBRARIES(qtclient Qt5::Network Qt5::Core)

ADD_LIBRARY(fafconnection SHARED ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(fafconnection Qt5::Network Qt5::Core)

INSTALL(TARGETS fafconnection
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/iridrivers
        ARCHIVE DESTINATION lib/iridrivers)
INSTALL(DIRECTORY include/
        DESTINATION include/iridrivers/fafconnection
        PATTERN "*.h"
        PATTERN ".svn" EXCLUDE)
INSTALL(FILES Findfafconnection.cmake DESTINATION ${CMAKE_ROOT}/Modules/)



