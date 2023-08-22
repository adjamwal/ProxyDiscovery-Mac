# Copyright (c) 2023 Cisco Systems, Inc. All Rights Reserved.

cmake_minimum_required(VERSION 3.19)
project(ProxyDiscovery)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CXX_EXTENSIONS ON)

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

list(APPEND PROXY_DISCOVERY_SOURCES
    include/IProxyDiscoveryEngine.h
    include/IProxyLogger.h
    include/ProxyDef.h
    include/ProxyRecord.h
    src/ProxyDiscoveryEngine.h
    src/ProxyDiscoveryEngine.mm
    src/ScopedGuard.hpp
    src/ProxyLogger.cpp
    src/ProxyLoggerDef.hpp
    src/StringUtil.hpp
    src/StringUtil.mm
)

add_library(ProxyDiscovery STATIC
    ${PROXY_DISCOVERY_SOURCES}
)

target_compile_definitions(ProxyDiscovery
    PRIVATE
        PROXY_DISCOVERY_MODULE_API_EXPORTS
)

set_property(TARGET ProxyDiscovery APPEND_STRING PROPERTY 
              COMPILE_FLAGS "-fobjc-arc")

target_include_directories(ProxyDiscovery
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${cm-client_SOURCE_DIR}
)

target_link_libraries(ProxyDiscovery
    PRIVATE
        "-framework Foundation"
        "-framework SystemConfiguration"
)
