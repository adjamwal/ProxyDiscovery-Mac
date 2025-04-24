/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */
#pragma once

#include "IProxyVerifier.hpp"
#include "gmock/gmock.h"

namespace proxy {

class MockProxyVerifier : public IProxyVerifier
{
    public:
        MOCK_METHOD(bool, verifyProxy, (const std::string &testUrl, const ProxyRecord &proxyRecord), (override));
};

} //proxy