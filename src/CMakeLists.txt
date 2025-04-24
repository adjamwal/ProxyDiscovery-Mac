# Copyright (c) 2025 Cisco Systems, Inc. All Rights Reserved.

set(component_name "ProxyDiscovery")

add_library(${component_name} STATIC
    ../include/IProxyDiscoveryEngine.h
    ../include/IProxyLogger.h
    ../include/ProxyDef.h
    ../include/ProxyRecord.h
    ProxyLogger.cpp
    ProxyLoggerDef.hpp
    ProxyRecord.cpp
)

target_compile_definitions(${component_name}
    PRIVATE
        PROXY_DISCOVERY_MODULE_API_EXPORTS
)

if(APPLE)
    target_sources(${component_name} PRIVATE
        darwin/ProxyDiscoveryEngine.h
        darwin/ProxyDiscoveryEngine.mm
        darwin/ProxyDiscoveryEngineFactory.mm
        darwin/ScopedGuard.hpp
        darwin/StringUtil.hpp
        darwin/StringUtil.mm
        darwin/ISystemConfigurationAPI.h
        darwin/SystemConfigurationAPI.h
        darwin/SystemConfigurationAPI.mm
    )
    target_include_directories(${component_name} PRIVATE
        ${PROJECT_SOURCE_DIR}/src/darwin   
    )   
    set_property(TARGET ${component_name} APPEND_STRING PROPERTY 
                COMPILE_FLAGS "-fobjc-arc")
    
    target_link_libraries(${component_name}
                PRIVATE
                    $<LINK_LIBRARY:FRAMEWORK,Foundation>
                    $<LINK_LIBRARY:FRAMEWORK,SystemConfiguration>)
elseif(LINUX)
    target_sources(${component_name} PRIVATE
        linux/ProxyDiscoveryEngine.cpp
        linux/ProxyDiscoveryEngine.hpp
        linux/ProxyCommandExec.cpp
        linux/ProxyCommandExec.hpp
        linux/IProxyCommandExec.hpp
        linux/ProxyVerifier.cpp
        linux/ProxyVerifier.hpp
        linux/IProxyVerifier.hpp
        linux/ProxyDiscoveryEngineFactory.cpp
    )

    target_include_directories(${component_name} PRIVATE
        ${CURL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src/linux      
    )

    target_link_libraries(${component_name} 
        curl
    )
endif()

target_include_directories(${component_name}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src

        ${cm-client_SOURCE_DIR}
)

# install rules
install(TARGETS ${component_name} DESTINATION lib)
install(FILES 
    "${CMAKE_SOURCE_DIR}/include/IProxyDiscoveryEngine.h"
    "${CMAKE_SOURCE_DIR}/include/IProxyLogger.h"
    "${CMAKE_SOURCE_DIR}/include/ProxyDef.h"
    "${CMAKE_SOURCE_DIR}/include/ProxyRecord.h"
   
    DESTINATION include/${component_name})
if(LINUX)
    install(FILES
        "${CMAKE_SOURCE_DIR}/src/linux/IProxyVerifier.hpp"
        "${CMAKE_SOURCE_DIR}/src/linux/IProxyCommandExec.hpp"
        "${CMAKE_SOURCE_DIR}/src/linux/ProxyDiscoveryEngine.hpp"
        DESTINATION include/${component_name})
endif()