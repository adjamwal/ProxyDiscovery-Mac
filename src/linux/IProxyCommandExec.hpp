/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <vector>
#include <string>

struct CommandOutput 
{
    int exitCode_;
    std::string output_;
};

class  IProxyCommandExec
{
public:
    virtual ~IProxyCommandExec() = default;
    /**
     * @brief Executes a command and captures the output.
     * @param[in] cmd The command to execute.
     * @param[in] argv The arguments to the command.
     * @return A composite type containing the exit code and the output of the command
     */
    virtual CommandOutput ExecuteCommandCaptureOutput(const std::string &cmd, const std::vector<std::string> &argv) = 0;

    virtual std::string getEnvironmentVar(const std::string &name) = 0;
};