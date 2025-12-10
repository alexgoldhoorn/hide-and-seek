# Pre-requisites about cmake itself
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0005 NEW)
  cmake_policy(SET CMP0003 NEW)

endif(COMMAND cmake_policy)

# The project name and the type of project
PROJECT(seekerhs)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
#SET(CMAKE_INSTALL_PREFIX /usr/local)

IF (NOT CMAKE_BUILD_TYPE)
 SET(CMAKE_BUILD_TYPE "RELEASE")  # was debug
ENDIF (NOT CMAKE_BUILD_TYPE)

# AG140116: set g++ compile to at least v4.8
SET(CMAKE_CXX_COMPILER "/usr/bin/g++-4.8")
SET(CMAKE_CC_COMPILER "/usr/bin/gcc-4.8")

#AG150123:  - added DO_NOT_USE_MOMDP, which disables MOMDP; also removed include of APPL (because of all compile warnings)
#           - removed -fpermissive parameter of compiler, since we now use g++ 4.8 on all platforms (PC + Robot's PC's).

# DEBUG_SHS_ON is to show some feedback of SeekerHS
SET(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -g -Wall -D_REENTRANT -DDEBUG_SHS_ON -DDEBUG_PB_ON -DDEBUG_POMCP_ON -DDEBUG_CLIENT_ON -DDO_NOT_USE_MOMDP ")
#AG140411: Sergi added '-ffloat-store' to solve an issue with double comparison on the 32 bit machine (issue of Intel processors)
SET(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O3 -ffloat-store -D_REENTRANT -DDEBUG_SHS_ON -DDO_NOT_USE_MOMDP ") #  -DDEBUG_CLIENT_ON -DDEBUG_PB_ON -DDEBUG_POMCP_ON

ADD_SUBDIRECTORY(src)

FIND_PACKAGE(Doxygen)

FIND_PATH(IRI_DOC_DIR doxygen.conf ${CMAKE_SOURCE_DIR}/doc/iri_doc/)
IF (IRI_DOC_DIR)
  ADD_CUSTOM_TARGET (doc ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/iri_doc/doxygen.conf)
ELSE (IRI_DOC_DIR)
  ADD_CUSTOM_TARGET (doc ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/doxygen.conf)
ENDIF (IRI_DOC_DIR)

ADD_CUSTOM_TARGET (distclean @echo cleaning cmake files)


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



