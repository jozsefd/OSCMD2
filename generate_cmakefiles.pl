use strict;
use warnings;

my $cmakelist = "CMakeLists.txt";

my %in_files = (
"win32/winconfig.h.in" => "win32/winconfig.h",
"src/minidriver/versioninfo-minidriver.rc.in" => "src/minidriver/versioninfo-minidriver.rc",
"src/pkcs11/versioninfo-pkcs11.rc.in" => "src/pkcs11/versioninfo-pkcs11.rc",
"src/pkcs11/versioninfo-pkcs11-spy.rc.in" => "src/pkcs11/versioninfo-pkcs11-spy.rc",
"src/tools/versioninfo-tools.rc.in" => "src/tools/versioninfo-tools.rc",
"win32/OpenSC.wxs.in" => "win32/OpenSC.wxs",
"win32/OpenSC.iss.in" => "win32/OpenSC.iss",
"win32/versioninfo-customactions.rc.in" => "win32/versioninfo-customactions.rc",
"win32/versioninfo.rc.in" => "win32/versioninfo.rc",
"win32/winconfig.h.in" => "src/config.h",
"etc/opensc.conf.example.in" => "etc/opensc.conf",
);

# 0.19.0-59-ga8db9cb4
my $git_description = `git describe`;
unless ( $git_description ) {
  die "Cannot get git description.\n";
}
my $git_suffix = git_suffix();
unless ( $git_suffix ) {
  die "Cannot get git suffix.\n";
}
my ($version, $revision, $tag) = split /\-/, $git_description;
my ($major, $minor, $fix) = split /\./, $version;


my %variables = (
DEFAULT_PCSC_PROVIDER => 'winscard.dll',
DEFAULT_PKCS11_PROVIDER => 'opensc-pkcs11.dll',
OPENSC_SCM_REVISION => "OpenSC-OpenSC $version",
OPENSC_VERSION_FIX => "$fix",
OPENSC_VERSION_MAJOR => "$major",
OPENSC_VERSION_MINOR => "$minor",
OPENSC_VERSION_REVISION => "$revision",
OPENSC_VS_FF_COMMENTS => 'Provided under the terms of the GNU Lesser General Public License (LGPLv2.1+).',
OPENSC_VS_FF_COMPANY_NAME => 'OpenSC Project',
OPENSC_VS_FF_COMPANY_URL => 'https://github.com/OpenSC',
OPENSC_VS_FF_LEGAL_COPYRIGHT => 'OpenSC Project',
OPENSC_VS_FF_PRODUCT_NAME => 'OpenSC smartcard framework',
OPENSC_VS_FF_PRODUCT_UPDATES => 'https://github.com/OpenSC/OpenSC/releases',
OPENSC_VS_FF_PRODUCT_URL => 'https://github.com/OpenSC/OpenSC',
PACKAGE_NAME => 'opensc',
PACKAGE_VERSION => "$version",
PRODUCT_BUGREPORT => 'https://github.com/OpenSC/OpenSC/issues',
VS_FF_PRODUCT_NAME => 'OpenSC',

DEBUG_FILE => '%TEMP%\\\opensc-debug.log',
DEFAULT_SM_MODULE => 'smm-local.dll',
DEFAULT_SM_MODULE_PATH => '# module_path = \\"\\";',
DYN_LIB_EXT => '.dll',
LIBDIR => '',
LIB_PRE => '',
PROFILE_DIR => '\\"\\"',
PROFILE_DIR_DEFAULT => 'obtained from windows registers',

MINIDRIVER_VERSION_MAJOR => '3',
MINIDRIVER_VERSION_MINOR => '2',
MINIDRIVER_VERSION_FIX   => '0',
MINIDRIVER_VERSION_REVISION => '0',

SPECIAL_BUILD => 'BA/OPS4 Build',

PACKAGE_SUFFIX => "$git_suffix",
);

open MAIN, ">$cmakelist" or die "Cannot create $cmakelist: $!\n";
print MAIN <<HEAD;
#
# OpenSC CMakeLists.txt
# jozsefd
#

cmake_minimum_required(VERSION 3.10)

project(opensc_build CXX C)

# enable static build
        set(flags_configs "")
        foreach(config \${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER \${config} config)
            list(APPEND flags_configs CMAKE_C_FLAGS_\${config})
            list(APPEND flags_configs CMAKE_CXX_FLAGS_\${config})
        endforeach()
        foreach(flags \${flags_configs})
            if(\${flags} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" \${flags} "\${\${flags}}")
            endif()
        endforeach()


HEAD

foreach my $var ( sort keys %variables ) {
  print MAIN "set($var \"$variables{$var}\")\n";
}

foreach my $file ( sort keys %in_files ) {
  print MAIN <<FILE;
configure_file (
  "\${PROJECT_SOURCE_DIR}/$file"
  "\${PROJECT_BINARY_DIR}/$in_files{$file}"
)
FILE
}

print MAIN <<TAIL;
configure_file(
  "\${PROJECT_SOURCE_DIR}/win32/DDORes.dll_14_2302.ico"
  "\${PROJECT_BINARY_DIR}/win32/DDORes.dll_14_2302.ico"
COPYONLY)

include_directories(\${PROJECT_SOURCE_DIR}/src)
include_directories(\${PROJECT_BINARY_DIR}/src)

add_definitions(-DHAVE_CONFIG_H -DWIN32_LEAN_AND_MEAN)
#_WIN32_WINNT=0x0502
#_CRT_SECURE_NO_WARNINGS
#_CRT_NONSTDC_NO_DEPRECATE
#_WIN32_WINNT=0x0502

# set(CMAKE_CXX_FLAGS_RELEASE -D_RELEASELOG)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY \${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY \${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src/common)
add_subdirectory(src/libopensc)
add_subdirectory(src/minidriver)
add_subdirectory(src/pkcs11)
add_subdirectory(src/pkcs15init)
add_subdirectory(src/scconf)
# no secure messaging add_subdirectory(src/sm)
# no secure messaging add_subdirectory(src/smm)
# no tests add_subdirectory(src/tests)
add_subdirectory(src/tools)
add_subdirectory(src/ui)

TAIL

close MAIN;

my %projects = (
  'src/common' => 'file(GLOB SRC_FILES *.c)
add_library(common ${SRC_FILES})',

  'src/libopensc' => 'file(GLOB SRC_FILES *.c)

# requires PACE
list(REMOVE_ITEM SRC_FILES card-npa.c)

get_filename_component(NPA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/card-npa.c ABSOLUTE)
#message("${NPA_SOURCE}")

list(REMOVE_ITEM SRC_FILES "${NPA_SOURCE}")
#message("${SRC_FILES}")

add_library(libopensc ${SRC_FILES})',

  'src/minidriver' => 'set(WINDOWS_KITS_PATH "C:/Program Files (x86)/Windows Kits")
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

target_link_libraries(minidriver Winmm)
target_link_libraries(minidriver Ws2_32)
target_link_libraries(minidriver crypt32)
target_link_libraries(minidriver rpcrt4)
target_link_libraries(minidriver bcrypt)
target_link_libraries(minidriver Comctl32)',

  'src/pkcs11' => 'file(GLOB SRC_FILES *.c)
add_library(pkcs11 ${SRC_FILES})',

  'src/pkcs15init' => 'file(GLOB SRC_FILES *.c)
add_library(pkcs15init ${SRC_FILES})',

  'src/scconf' => 'file(GLOB SRC_FILES *.c)
add_library(scconf ${SRC_FILES})',

  'src/sm' => 'file(GLOB SRC_FILES *.c)
add_library(sm ${SRC_FILES})',

  'src/smm' => 'file(GLOB SRC_FILES *.c)
add_library(smm ${SRC_FILES})',

  'src/tools' => '#add_executable(opensc-explorer util.c opensc-explorer.c ${PROJECT_BINARY_DIR}/src/minidriver/version-tools.rc)
set(MANIFEST_FILE exe.manifest)
add_executable(opensc-explorer util.c opensc-explorer.c ${MANIFEST_FILE})
target_link_libraries(opensc-explorer common)
target_link_libraries(opensc-explorer scconf)
target_link_libraries(opensc-explorer libopensc)
target_link_libraries(opensc-explorer pkcs15init)
target_link_libraries(opensc-explorer pkcs11)
target_link_libraries(opensc-explorer ui)
target_link_libraries(opensc-explorer Comctl32)
target_link_libraries(opensc-explorer Ws2_32)

add_executable(opensc-tool util.c opensc-tool.c ${MANIFEST_FILE})
target_link_libraries(opensc-tool common)
target_link_libraries(opensc-tool scconf)
target_link_libraries(opensc-tool libopensc)
target_link_libraries(opensc-tool pkcs15init)
target_link_libraries(opensc-tool pkcs11)
target_link_libraries(opensc-tool ui)
target_link_libraries(opensc-tool Comctl32)
target_link_libraries(opensc-tool Ws2_32)',

  'src/ui' => 'file(GLOB SRC_FILES *.c)
add_library(ui ${SRC_FILES})',
);

foreach my $project ( sort keys %projects ) {
  if ( open OUT, ">$project/$cmakelist" ) {
    print OUT $projects{$project}, "\n";
    close OUT;
  } else {
    warn "Failed to write $project/$cmakelist: $!\n";
  }
}

# # SUFFIX=g`git log -1 --pretty=fuller --date=iso | grep CommitDate: | sed -E 's/^CommitDate:\s(.*)/\1/' | sed -E 's/(.*)-(.*)-(.*) (.*):(.*):(.*)\s+.*/\1\2\3\4\5\6/'`
# CommitDate: 2018-10-04 09:41:31 +0200
sub git_suffix {
  my $git_log = `git log -1 --pretty=fuller --date=iso`;
  return undef unless $git_log;

  if ( $git_log =~ /CommitDate:\s+(\S+)\s+(\S+)/m ) {
    my $suffix = "$1$2";
    $suffix =~ s/[\-\:]//g;
    return $suffix;
  }
  return undef;
}