/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include "ProxyDiscoveryEngine.hpp"
#include "ProxyCommandExec.hpp"
#include "ProxyVerifier.hpp"

namespace proxy
{

std::shared_ptr<IProxyDiscoveryEngine> createProxyEngine()
{
    return std::make_shared<ProxyDiscoveryEngine>(
        std::make_shared<ProxyCommandExec>(), std::make_shared<ProxyVerifier>());
}

} //proxy namespace
