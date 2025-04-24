# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

cmake_minimum_required(VERSION 3.25)
project(ProxyDiscovery)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#pass below values via command line argument to cmake command, like: -DCMAKE_CXX_STANDARD=17
#set(CMAKE_C_STANDARD 99)
#set(CMAKE_CXX_STANDARD 17)
#set(CXX_EXTENSIONS ON)

if(APPLE)
    message("** Building on macOS, generator ${CMAKE_GENERATOR} **")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)

    #
    # The `;` must be escaped when passed into ExternalProject_Add()
    string(REPLACE ";" "$<SEMICOLON>" CMAKE_OSX_ARCHITECTURES_ "${CMAKE_OSX_ARCHITECTURES}")

    set(XCODE_TOOLCHAIN_BIN "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin")
    set(XCODE_MACOS_SDK "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
    set(XCODE_CC "${XCODE_TOOLCHAIN_BIN}/clang -mmacosx-version-min=10.15 -isysroot ${XCODE_MACOS_SDK} -g -arch x86_64 -arch arm64")
    set(XCODE_CPP "${XCODE_TOOLCHAIN_BIN}/clang -mmacosx-version-min=10.15 -isysroot ${XCODE_MACOS_SDK} -E")
    set(XCODE_AR "${XCODE_TOOLCHAIN_BIN}/ar r")
    set(XCODE_NM "${XCODE_TOOLCHAIN_BIN}/nm")
    set(XCODE_RANLIB "${XCODE_TOOLCHAIN_BIN}/ranlib")

    if(DEFINED SIGNING_CERT)
        message("--Build will sign using ${SIGNING_CERT}")
    endif()
endif()

# remove ZERO_CHECK target from xcode
set(CMAKE_SUPPRESS_REGENERATION true)

# gtest
message( "Using test gtest_LIBRARY=${gtest_LIBRARY} gtest_INCLUDE_DIRS=${gtest_INCLUDE_DIRS}" )
if(NOT DEFINED gtest_LIBRARY OR NOT DEFINED gtest_INCLUDE_DIRS)
    message( FATAL_ERROR "Missing gtest" )
endif()

if(LINUX) 
    message( "Using CURL_LIBRARY_DIR=${CURL_LIBRARY_DIR} CURL_INCLUDE_DIRS=${CURL_INCLUDE_DIR}" )
    if(NOT DEFINED CURL_LIBRARY_DIR OR NOT DEFINED CURL_INCLUDE_DIR)
        message( FATAL_ERROR "Missing curl paths" )
    endif()
    if(NOT EXISTS "${CURL_LIBRARY_DIR}/libcurl.a") 
        message( FATAL_ERROR "Curl library does not exist at path ${CURL_LIBRARY_DIR}" )
    endif()
    add_library(curl STATIC IMPORTED)
    set_target_properties(curl PROPERTIES IMPORTED_LOCATION "${CURL_LIBRARY_DIR}/libcurl.a")
endif()

add_subdirectory(src)

##
## test executable
add_subdirectory(test)