/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "ProxyDiscoveryEngine.h"
#include "SystemConfigurationAPI.h"

namespace proxy
{

std::unique_ptr<IProxyDiscoveryEngine> createProxyEngine()
{
    return std::make_unique<ProxyDiscoveryEngine>(
        std::make_shared<SystemConfigurationAPI>());
}

} //proxy namespace
