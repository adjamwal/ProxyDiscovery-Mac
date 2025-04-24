/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include "ProxyCommandExec.hpp"
#include "ProxyLoggerDef.hpp"
#include <string>
#include <vector>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <memory>

CommandOutput ProxyCommandExec::ExecuteCommandCaptureOutput(const std::string &cmd, const std::vector<std::string> &argv) {
    int status = 1;
    pid_t pid = -1;
    pid_t waitPid = -1;
    extern char **environ;
    posix_spawn_file_actions_t childFdActions = {0};
    int out [2] = {-1, -1};
    ssize_t bytesRead = 0;
    const int waitPidOptions = 0;   

    if (cmd.empty() || argv.empty()) {
        throw std::runtime_error("missing arguments");
    }

    if ('/' != cmd[0]) {
        throw std::runtime_error("command must be an absolute path");
    }                                                                   

    if (posix_spawn_file_actions_init(&childFdActions) != 0) {
        throw std::runtime_error("posix_spawn_file_actions_init failed");
    }

    // Lambda Function to delete file actions (Used as custom deleter)
    auto fileActionsDeleter = [](posix_spawn_file_actions_t *actions) {
        posix_spawn_file_actions_destroy(actions);
    };   

    auto childFdActionsPtr = std::unique_ptr<posix_spawn_file_actions_t, decltype(fileActionsDeleter)>(&childFdActions, fileActionsDeleter); // Just for cleaning purpose on return.

    // Lambda Function to close pipe (Used as custom deleter)
    auto close_pipe = [](int *out) {
        if (out[0] != -1) {
            (void)close(out[0]);
        }
        if (out[1] != -1) {
            (void)close(out[1]);
        }
    };

    auto outPtr = std::unique_ptr<int, decltype(close_pipe)>(out, close_pipe); // Just for cleaning purpose on return.

    if (pipe(out) == -1) {
        throw std::runtime_error("pipe failed");
    }

    int flags = fcntl(out[0], F_GETFL, 0); 
    if (flags == -1) {
        throw std::runtime_error("fcntl failed");
    }

    if (posix_spawn_file_actions_adddup2(&childFdActions, out[1], 1) != 0) {
        throw std::runtime_error("posix_spawn_file_actions_adddup2");
    }

    if (posix_spawn_file_actions_addclose(&childFdActions, out[0]) != 0) {
        throw std::runtime_error("posix_spawn_file_actions_addclose");
    }

    std::vector<char*> argv_cstr;
    for (const auto& arg : argv) {
        argv_cstr.push_back(const_cast<char*>(arg.c_str()));
    }
    argv_cstr.push_back(nullptr);

    if (posix_spawn(&pid, cmd.c_str(), &childFdActions, NULL, argv_cstr.data(), environ) != 0) {
        throw std::runtime_error({"posix_spawn failed: " + cmd});
    }

    PROXY_LOG_DEBUG("Spawned process for cmd %s: %d", cmd.c_str(), pid);

    fcntl(out[0], F_SETFL, flags | O_NONBLOCK);

    do {
        waitPid = waitpid(pid, &status, waitPidOptions);
    } while (waitPid < 0 && errno == EINTR);
    
    if (waitPid < 0) {
        throw std::runtime_error(std::string{"waitpid failed with error: %d", errno});
    } else if (waitPid > 0) {
        if (WIFEXITED(status)) {
            PROXY_LOG_DEBUG("Process '%s' terminated normally: %d (exit code: %d)", cmd.c_str(), pid, WEXITSTATUS(status));
            
            std::string output{""};

            char buffer[1024];
            while ((bytesRead = read(out[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytesRead] = '\0';
                output.append(buffer);
            }                       

            return { WEXITSTATUS(status), output };
        } else if (WIFSIGNALED(status)) {
            throw std::runtime_error("Process '" + cmd + "' terminated due to uncaught exception: " + std::to_string(pid));
        } else if (WIFSTOPPED(status)) {
            throw std::runtime_error("Process '" + cmd + "' stopped abnormally: " + std::to_string(pid));
        } else {
            throw std::runtime_error("Process '" + cmd + "' did not return: " + std::to_string(pid));
        }
    }

    return { -1, "" };
}

std::string ProxyCommandExec::getEnvironmentVar(const std::string &name) 
{
    const char * str = std::getenv(name.c_str());
    if (str) {
        return std::string(str);
    }
    return "";
}