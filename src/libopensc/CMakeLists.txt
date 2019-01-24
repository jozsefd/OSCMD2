file(GLOB SRC_FILES *.c)

# requires PACE
#!list(REMOVE_ITEM SRC_FILES card-npa.c)

#!get_filename_component(NPA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/card-npa.c ABSOLUTE)
#message("${NPA_SOURCE}")

#!list(REMOVE_ITEM SRC_FILES "${NPA_SOURCE}")
#message("${SRC_FILES}")

add_library(libopensc ${SRC_FILES})
