# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.

set(GTEST_MAIN_FREE_LIBS
    gtestd
    gmockd
)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT USE_DEBUG_TEST_LIBRARIES)
    set(GTEST_MAIN_FREE_LIBS
        gtest
        gmock
    )
endif()

add_executable(
  ProxyDiscoveryTest
  ../include/IProxyDiscoveryEngine.h
  ../include/IProxyLogger.h
  ../include/ProxyDef.h
  ../include/ProxyRecord.h
  ../src/ProxyRecord.cpp
  ../src/ProxyDiscoveryEngine.h
  ../src/ProxyDiscoveryEngine.mm
  ../src/ScopedGuard.hpp
  ../src/ProxyLogger.cpp
  ../src/ProxyLoggerDef.hpp
  ../src/StringUtil.hpp
  ../src/StringUtil.mm
  ../src/ISystemConfigurationAPI.h
  # Test files
  TestProxyDiscovery.mm
  #Fake instances
  FakeSystemConfigurationAPI.hpp
  FakeSystemConfigurationAPI.mm
)

set_property(TARGET ProxyDiscoveryTest APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")

target_compile_features(ProxyDiscoveryTest PRIVATE cxx_std_11)

target_link_directories(
  ProxyDiscoveryTest BEFORE
  PRIVATE
      ${gtest_LIBRARY}
)

target_link_libraries(
  ProxyDiscoveryTest
  ${GTEST_MAIN_FREE_LIBS}
  "-framework CFNetwork"
  "-framework Foundation"
  "-framework SystemConfiguration"
)

target_include_directories(
  ProxyDiscoveryTest
  PRIVATE
      ${gtest_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/../include
      ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

include(GoogleTest)
gtest_discover_tests(ProxyDiscoveryTest DISCOVERY_MODE PRE_TEST)
