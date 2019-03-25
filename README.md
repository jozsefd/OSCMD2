# OpenSC documentation

Wiki is [available online](https://github.com/OpenSC/OpenSC/wiki)

Please take a look at the documentation before trying to use OpenSC.

[![Travis CI Build Status](https://travis-ci.org/OpenSC/OpenSC.svg)](https://travis-ci.org/OpenSC/OpenSC/branches) [![AppVeyor CI Build Status](https://ci.appveyor.com/api/projects/status/github/OpenSC/OpenSC?branch=master&svg=true)](https://ci.appveyor.com/project/LudovicRousseau/OpenSC/branch/master) [![Coverity Scan Status](https://scan.coverity.com/projects/4026/badge.svg)](https://scan.coverity.com/projects/4026)

Build and test status of specific cards:

| Cards                 | Status |
|-----------------------|--------|
| CAC                   | [![CAC](https://gitlab.com/redhat-crypto/OpenSC/badges/cac/build.svg)](https://gitlab.com/redhat-crypto/OpenSC/pipelines) |
| Coolkey               | [![Coolkey](https://gitlab.com/redhat-crypto/OpenSC/badges/coolkey/build.svg)](https://gitlab.com/redhat-crypto/OpenSC/pipelines) |

## Building with CMake

It was tested with cmake 3.10.3, it should work with versions newer than 3.8, although for *Visual Studio 15* v3.10 is required.

The cmake configuration builds the following OpenSC binaries:

1. minidriver
1. pkcs11 module
1. opensc-explorer
1. opensc-tool

and the following OpenSC libraties:

1. common
1. libopensc
1. pkcs11
1. pkcs15init
1. scconf
1. ui

### Dependencies

- can be built with or without OpenSSL (see Build Configuration below)
- no OpenPace and Zlib support

### Build Configuration
The following CMake configuration options are available:

| Option                 | Description |
|------------------------|-------------|
| ENABLE_OPENSSL         | ON/OFF (default OFF) |
| FILE_VERSION_MAJOR     | Major File Version (default 3)
| FILE_VERSION_MINOR     | Minor File Version (default 0) |
| FILE_VERSION_FIX       | File Version Fix (default 0) |
| FILE_VERSION_REVISION  | File Version Revision (default 0) |

The provided File Version will be used in the Version resource of the minidriver and p11 DLLs in order to allow build identification. The Product Version show the OpenSC Version unchanged.

### Example build

```
mkdir build
cd build
cmake ..\OpenSC -G "Visual Studio 15" \
  -DFILE_VERSION_MAJOR=3 \
  -DFILE_VERSION_MINOR=2 \
  -DFILE_VERSION_FIX=1 \
  -DFILE_VERSION_REVISION=1 \
  -DENABLE_OPENSSL=ON
cmake --build . --config Release
cmake --build . --config Debug
```

## eSign/QES PKCS15 Emulator

A PKCS15 Emulator was added for G&D StarCOS 3.x cards with an eSign/QES card profile. It will be automatically probed and selected, but it can be forced too:

```
app default {
    ...
	framework pkcs15 {
		try_emulation_first = yes;
		builtin_emulators = esign_qes;
	}
}
```
