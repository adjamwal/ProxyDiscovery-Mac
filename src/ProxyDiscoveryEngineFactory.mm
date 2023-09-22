/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "ProxyDiscoveryEngine.h"
#include "SystemConfigurationAPI.h"

namespace proxy
{

std::shared_ptr<IProxyDiscoveryEngine> createProxyEngine()
{
    return std::make_shared<ProxyDiscoveryEngine>(
        std::make_shared<SystemConfigurationAPI>());
}

} //proxy namespace
