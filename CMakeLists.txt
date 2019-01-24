#
# OpenSC CMakeLists.txt
# jozsefd
#

cmake_minimum_required(VERSION 3.10)

project(opensc_build CXX C)

# enable static build
        set(flags_configs "")
        foreach(config ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER ${config} config)
            list(APPEND flags_configs CMAKE_C_FLAGS_${config})
            list(APPEND flags_configs CMAKE_CXX_FLAGS_${config})
        endforeach()
        foreach(flags ${flags_configs})
            if(${flags} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flags} "${${flags}}")
            endif()
        endforeach()


set(DEBUG_FILE "%TEMP%\\opensc-debug.log")
set(DEFAULT_PCSC_PROVIDER "winscard.dll")
set(DEFAULT_PKCS11_PROVIDER "opensc-pkcs11.dll")
set(DEFAULT_SM_MODULE "smm-local.dll")
set(DEFAULT_SM_MODULE_PATH "# module_path = \"\";")
set(DYN_LIB_EXT ".dll")
set(LIBDIR "")
set(LIB_PRE "")
set(MINIDRIVER_VERSION_FIX "0")
set(MINIDRIVER_VERSION_MAJOR "3")
set(MINIDRIVER_VERSION_MINOR "2")
set(MINIDRIVER_VERSION_REVISION "0")
set(OPENSC_SCM_REVISION "OpenSC-OpenSC 0.19.0")
set(OPENSC_VERSION_FIX "0")
set(OPENSC_VERSION_MAJOR "0")
set(OPENSC_VERSION_MINOR "19")
set(OPENSC_VERSION_REVISION "79")
set(OPENSC_VS_FF_COMMENTS "Provided under the terms of the GNU Lesser General Public License (LGPLv2.1+).")
set(OPENSC_VS_FF_COMPANY_NAME "OpenSC Project")
set(OPENSC_VS_FF_COMPANY_URL "https://github.com/OpenSC")
set(OPENSC_VS_FF_LEGAL_COPYRIGHT "OpenSC Project")
set(OPENSC_VS_FF_PRODUCT_NAME "OpenSC smartcard framework")
set(OPENSC_VS_FF_PRODUCT_UPDATES "https://github.com/OpenSC/OpenSC/releases")
set(OPENSC_VS_FF_PRODUCT_URL "https://github.com/OpenSC/OpenSC")
set(PACKAGE_NAME "opensc")
set(PACKAGE_SUFFIX "20181026002943")
set(PACKAGE_VERSION "0.19.0")
set(PRODUCT_BUGREPORT "https://github.com/OpenSC/OpenSC/issues")
set(PROFILE_DIR "\"\"")
set(PROFILE_DIR_DEFAULT "obtained from windows registers")
set(SPECIAL_BUILD "BA/OPS4 Build")
set(VS_FF_PRODUCT_NAME "OpenSC")
configure_file (
  "${PROJECT_SOURCE_DIR}/etc/opensc.conf.example.in"
  "${PROJECT_BINARY_DIR}/etc/opensc.conf"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/src/minidriver/versioninfo-minidriver.rc.in"
  "${PROJECT_BINARY_DIR}/src/minidriver/versioninfo-minidriver.rc"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/src/pkcs11/versioninfo-pkcs11-spy.rc.in"
  "${PROJECT_BINARY_DIR}/src/pkcs11/versioninfo-pkcs11-spy.rc"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/src/pkcs11/versioninfo-pkcs11.rc.in"
  "${PROJECT_BINARY_DIR}/src/pkcs11/versioninfo-pkcs11.rc"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/src/tools/versioninfo-tools.rc.in"
  "${PROJECT_BINARY_DIR}/src/tools/versioninfo-tools.rc"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/win32/OpenSC.iss.in"
  "${PROJECT_BINARY_DIR}/win32/OpenSC.iss"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/win32/OpenSC.wxs.in"
  "${PROJECT_BINARY_DIR}/win32/OpenSC.wxs"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/win32/versioninfo-customactions.rc.in"
  "${PROJECT_BINARY_DIR}/win32/versioninfo-customactions.rc"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/win32/versioninfo.rc.in"
  "${PROJECT_BINARY_DIR}/win32/versioninfo.rc"
)
configure_file (
  "${PROJECT_SOURCE_DIR}/win32/winconfig.h.in"
  "${PROJECT_BINARY_DIR}/src/config.h"
)
configure_file(
  "${PROJECT_SOURCE_DIR}/win32/DDORes.dll_14_2302.ico"
  "${PROJECT_BINARY_DIR}/win32/DDORes.dll_14_2302.ico"
COPYONLY)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/src)

add_definitions(-DHAVE_CONFIG_H -DWIN32_LEAN_AND_MEAN)
#_WIN32_WINNT=0x0502
#_CRT_SECURE_NO_WARNINGS
#_CRT_NONSTDC_NO_DEPRECATE
#_WIN32_WINNT=0x0502

# set(CMAKE_CXX_FLAGS_RELEASE -D_RELEASELOG)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src/common)
add_subdirectory(src/libopensc)
add_subdirectory(src/minidriver)
add_subdirectory(src/pkcs11)
add_subdirectory(src/pkcs15init)
add_subdirectory(src/scconf)
# no sm add_subdirectory(src/sm)
# no sm add_subdirectory(src/smm)
# no tests add_subdirectory(src/tests)
add_subdirectory(src/tools)
add_subdirectory(src/ui)

# add_subdirectory(src/esign_qes_emu)
