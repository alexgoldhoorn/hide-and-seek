#edit the following line to add the librarie's header files
FIND_PATH(seekerhs_INCLUDE_DIR Base/seekerhs.h /usr/include/iridrivers/hide-and-seek /usr/local/include/iridrivers/hide-and-seek)

FIND_LIBRARY(seekerhs_LIBRARY
    NAMES seekerhs
    PATHS /usr/lib /usr/local/lib /usr/local/lib/iridrivers/) 

IF (seekerhs_INCLUDE_DIR AND seekerhs_LIBRARY)
   SET(seekerhs_FOUND TRUE)
ENDIF (seekerhs_INCLUDE_DIR AND seekerhs_LIBRARY)

IF (seekerhs_FOUND)
   IF (NOT seekerhs_FIND_QUIETLY)
      MESSAGE(STATUS "Found seekerhs: ${seekerhs_LIBRARY}")
   ENDIF (NOT seekerhs_FIND_QUIETLY)
ELSE (seekerhs_FOUND)
   IF (seekerhs_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find seekerhs")
   ENDIF (seekerhs_FIND_REQUIRED)
ENDIF (seekerhs_FOUND)
