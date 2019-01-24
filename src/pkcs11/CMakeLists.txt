set(SRC_FILES pkcs11-global.c pkcs11-session.c pkcs11-object.c misc.c slot.c mechanism.c openssl.c framework-pkcs15.c framework-pkcs15init.c debug.c pkcs11-display.c)
add_definitions(-DMODULE_APP_NAME=\"onepin-opensc-pkcs11\")
add_library(pkcs11 ${SRC_FILES})

set(EXP_FILE pkcs11.exports)
set(DEF_FILE ${CMAKE_CURRENT_BINARY_DIR}/pkcs11.def)
set(MANIFEST_FILE opensc-pkcs11.dll.manifest)

file(READ ${EXP_FILE} EXPORTS)
file(WRITE ${DEF_FILE} "LIBRARY p11\n")
file(APPEND ${DEF_FILE} "EXPORTS\n")
file(APPEND ${DEF_FILE} ${EXPORTS})

add_library(p11 SHARED ${SRC_FILES} ${MANIFEST_FILE} ${DEF_FILE} ${CMAKE_CURRENT_BINARY_DIR}/versioninfo-pkcs11.rc)

target_link_libraries(p11 common)
target_link_libraries(p11 scconf)
target_link_libraries(p11 libopensc)
target_link_libraries(p11 pkcs15init)
target_link_libraries(p11 pkcs11)
target_link_libraries(p11 ui)
#->target_link_libraries(pkcs11 sm)
#->target_link_libraries(pkcs11 smm)

target_link_libraries(p11 Ws2_32)
target_link_libraries(p11 Shell32)
target_link_libraries(p11 Comctl32)
