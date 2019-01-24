set(WINDOWS_KITS_PATH "C:/Program Files (x86)/Windows Kits")
set(CARDMOD_HEADER "cardmod.h")
set(CARDMOD_DIR "${WINDOWS_KITS_PATH}/8.0/Cryptographic Provider Development Kit/Include")

file(GLOB_RECURSE CARDMOD_HEADERS "${WINDOWS_KITS_PATH}/*/${CARDMOD_HEADER}")
foreach(path ${CARDMOD_HEADERS})
  get_filename_component(dir ${path} PATH)
  if( ${dir} )
    # override default
    set(CARDMOD_DIR ${dir})
  endif()
endforeach()

include_directories(${CARDMOD_DIR})
add_definitions(-DENABLE_MINIDRIVER)

set(DEF_FILE ${CMAKE_CURRENT_BINARY_DIR}/minidriver.def)
file(WRITE ${DEF_FILE} "LIBRARY opensc\n")
file(APPEND ${DEF_FILE} "EXPORTS\n")
file(APPEND ${DEF_FILE} "CardAcquireContext\n")

set(MANIFEST_FILE opensc-minidriver.dll.manifest)

#set_source_files_properties(${DEF_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
add_library(minidriver SHARED minidriver.c ${MANIFEST_FILE} ${DEF_FILE} ${CMAKE_CURRENT_BINARY_DIR}/versioninfo-minidriver.rc)
#add_library(minidriver SHARED minidriver.c)

target_link_libraries(minidriver common)
target_link_libraries(minidriver scconf)
target_link_libraries(minidriver libopensc)
target_link_libraries(minidriver pkcs15init)
target_link_libraries(minidriver pkcs11)
target_link_libraries(minidriver ui)
#target_link_libraries(minidriver sm)
#target_link_libraries(minidriver smm)

target_link_libraries(minidriver Winmm)
target_link_libraries(minidriver Ws2_32)
target_link_libraries(minidriver crypt32)
target_link_libraries(minidriver rpcrt4)
target_link_libraries(minidriver bcrypt)
target_link_libraries(minidriver Comctl32)
