/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "IProxyCommandExec.hpp"

class  ProxyCommandExec : public IProxyCommandExec
{
public:
    CommandOutput ExecuteCommandCaptureOutput(const std::string &cmd, const std::vector<std::string> &argv) override;

    std::string getEnvironmentVar(const std::string &name) override;
};