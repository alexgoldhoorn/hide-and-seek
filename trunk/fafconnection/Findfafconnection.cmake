#edit the following line to add the librarie's header files
FIND_PATH(fafconnection_INCLUDE_DIR fafconnection.h /usr/include/iridrivers/hide-and-seek /usr/local/include/iridrivers/fafconnection)

FIND_LIBRARY(fafconnection_LIBRARY
    NAMES fafconnection
    PATHS /usr/lib /usr/local/lib /usr/local/lib/iridrivers/) 

IF (fafconnection_INCLUDE_DIR AND fafconnection_LIBRARY)
   SET(fafconnection_FOUND TRUE)
ENDIF (fafconnection_INCLUDE_DIR AND fafconnection_LIBRARY)

IF (fafconnection_FOUND)
   IF (NOT fafconnection_FIND_QUIETLY)
      MESSAGE(STATUS "Found fafconnection: ${fafconnection_LIBRARY}")
   ENDIF (NOT fafconnection_FIND_QUIETLY)
ELSE (fafconnection_FOUND)
   IF (fafconnection_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find fafconnection")
   ENDIF (fafconnection_FIND_REQUIRED)
ENDIF (fafconnection_FOUND)
