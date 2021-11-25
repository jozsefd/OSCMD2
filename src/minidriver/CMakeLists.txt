set(CARDMOD_HEADER "cardmod.h")
set(CARDMOD_DIR "C:/Program Files (x86)/Microsoft CNG Development Kit/Include")

include_directories(${CARDMOD_DIR})
add_definitions(-DENABLE_MINIDRIVER -DDISABLE_PINPAD)

set(DEF_FILE ${CMAKE_CURRENT_BINARY_DIR}/minidriver.def)
file(WRITE ${DEF_FILE} "LIBRARY minidriver\n")
file(APPEND ${DEF_FILE} "EXPORTS\n")
file(APPEND ${DEF_FILE} "CardAcquireContext\n")

# set(MANIFEST_FILE opensc-minidriver.dll.manifest)

# add_library(minidriver SHARED minidriver.c ${MANIFEST_FILE} ${DEF_FILE} ${CMAKE_CURRENT_BINARY_DIR}/versioninfo-minidriver.rc)
add_library(minidriver SHARED minidriver.c ${DEF_FILE} ${CMAKE_CURRENT_BINARY_DIR}/versioninfo-minidriver.rc)

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
# target_link_libraries(minidriver Comctl32)
if(${ENABLE_OPENSSL})
target_link_libraries(minidriver ${OPENSSL_LIB})
endif()
