# CMakeLists.txt
# Copyright 2023, Cisco Systems, Inc.
set(component_name "ProxyDiscoveryTest")

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
  ${component_name}
)
if(APPLE)
  target_sources(${component_name} PRIVATE
    # Test files
    darwin/TestProxyDiscovery.mm
    #Fake instances
    darwin/FakeSystemConfigurationAPI.hpp
    darwin/FakeSystemConfigurationAPI.mm
  )

  set_property(TARGET ${component_name} APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")

  target_link_libraries(${component_name}
    $<LINK_LIBRARY:FRAMEWORK,CFNetwork>
    $<LINK_LIBRARY:FRAMEWORK,Foundation>
    $<LINK_LIBRARY:FRAMEWORK,SystemConfiguration>
  )

  target_include_directories(${component_name} PUBLIC
    ${PROJECT_SOURCE_DIR}/src/darwin
  )

elseif(LINUX)
  target_sources(${component_name} PRIVATE
      linux/TestProxyDiscovery.cpp
      linux/mock/MockCommandExec.hpp
      linux/mock/MockProxyVerifier.hpp
  )

  target_include_directories(${component_name} PUBLIC
      ${PROJECT_SOURCE_DIR}/src/linux
      linux/mock
  )

  target_link_libraries(${component_name}
    pthread
  )   
endif()

target_link_libraries(${component_name}
    ProxyDiscovery
    ${GTEST_MAIN_FREE_LIBS}
)

target_link_directories(
  ${component_name} BEFORE
  PRIVATE
      ${gtest_LIBRARY}
)

target_include_directories(
  ${component_name}
  PRIVATE
      ${gtest_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/../include
      ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

include(GoogleTest)
gtest_discover_tests(ProxyDiscoveryTest DISCOVERY_MODE PRE_TEST)
