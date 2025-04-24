/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include "IProxyDiscoveryEngine.h"
#include "IProxyCommandExec.hpp"
#include "IProxyVerifier.hpp"

#include <deque>
#include <memory>
#include <thread>

namespace proxy
{

/**
 * @brief A class that performs available proxy settings discovery.
 */
class ProxyDiscoveryEngine: public IProxyDiscoveryEngine {
public:
    ~ProxyDiscoveryEngine();
    explicit ProxyDiscoveryEngine(std::shared_ptr<IProxyCommandExec> commandExecutor, std::shared_ptr<IProxyVerifier> proxyVerifier);
    ProxyDiscoveryEngine(const ProxyDiscoveryEngine&) = delete;
    ProxyDiscoveryEngine(ProxyDiscoveryEngine&&) = delete;
    ProxyDiscoveryEngine& operator = (const ProxyDiscoveryEngine&) = delete;
    ProxyDiscoveryEngine& operator = (ProxyDiscoveryEngine&&) = delete;
    
    void addObserver(IProxyObserver& pObserver) override;
    void requestProxiesAsync(const std::string& testUrl, const std::string &pacUrl, const std::string& guid) override;
    std::list<ProxyRecord> getProxies(const std::string& testUrl, const std::string &pacUrl) override;
    void waitPrevOpCompleted() override;
    
private:
    std::list<ProxyRecord> getProxiesInternal();
    void notifyObservers(const std::list<ProxyRecord>& proxies, const std::string& guid);
    std::list<ProxyRecord> gnomeProxy();
    std::list<ProxyRecord> kdeProxy();
    ProxyRecord parseGnomeProxy(const std::string& protocol);

    std::shared_ptr<IProxyCommandExec> m_commandExecutor;
    std::shared_ptr<IProxyVerifier> m_proxyVerifier;
    std::deque<IProxyObserver*> m_observers;
    std::shared_ptr<std::thread> m_thread;
};

} //proxy namespace
