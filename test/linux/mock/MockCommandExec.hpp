/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */
#pragma once

#include "IProxyCommandExec.hpp"
#include "gmock/gmock.h"

class MockCommandExec : public IProxyCommandExec
{
    public:
        MOCK_METHOD(CommandOutput, ExecuteCommandCaptureOutput, (const std::string &cmd, const std::vector<std::string> &argv), (override));
        MOCK_METHOD(std::string, getEnvironmentVar, (const std::string &name), (override));
};
