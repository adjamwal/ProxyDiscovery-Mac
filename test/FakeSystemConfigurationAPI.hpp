#pragma once

#include "ISystemConfigurationAPI.h"
#include "ProxyRecord.h"

#include <unordered_map>
#include <string>
#include <list>

namespace proxy
{
/**
 * @brief Class that implements fake version of the ISystemConfiguration interface.
 */
class FakeSystemConfigurationAPI: public ISystemConfigurationAPI
{
public:
    CFRunLoopSourceRef executeProxyAutoConfigurationURL(NSURL* testUrl, NSURL* scriptURL,
        CFProxyAutoConfigurationResultCallback cb,
        CFStreamClientContext* clientContext) override;
    
    NSDictionary* dynamicStoreCopyProxies() override;
    NSArray* copyProxiesForURL(NSURL* testUrl, NSDictionary* proxySettings) override;

    void addProxies(const std::string& strUrl, const std::list<ProxyRecord>& proxies);
    void addPACProxies(const std::string& strUrl, const std::list<ProxyRecord>& proxies);
    
private:
    NSArray* convertProxies(const std::list<ProxyRecord>& proxyList);
    
    std::unordered_map<std::string, std::list<ProxyRecord>> proxyMap_;
    std::unordered_map<std::string, std::list<ProxyRecord>> proxyPACMap_;
};

}
