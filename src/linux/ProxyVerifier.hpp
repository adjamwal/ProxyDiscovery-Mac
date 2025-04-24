/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "IProxyVerifier.hpp"
#include <curl/curl.h>

#include <string>

namespace proxy {

class ProxyVerifier : public IProxyVerifier
{
public:
    ProxyVerifier() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    ~ProxyVerifier() {
        curl_global_cleanup();
    }
    /**
     * @brief Attempts to reach proxy server and perform connection to test url through proxy
     * @param testUrl the url to perform a test connection to
     * @param proxyRecord the proxy server to test
     * @return True of proxy server is valid, false if not
     */
    bool verifyProxy(const std::string &testUrl, const ProxyRecord &proxyRecord) override;
};

} //proxy