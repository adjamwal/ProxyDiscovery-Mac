/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "ProxyRecord.h"

#include <string>

namespace proxy {

class  IProxyVerifier
{
public:
    virtual ~IProxyVerifier() = default;
    /**
     * @brief Attempts to reach proxy server and perform connection to test url through proxy
     * @param testUrl the url to perform a test connection to
     * @param proxyRecord the proxy server to test
     * @return True if proxy server is valid, otherwise false
     */
    virtual bool verifyProxy(const std::string &testUrl, const ProxyRecord &proxyRecord) = 0;
};

} //proxy