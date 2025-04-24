#pragma once

#include "ISystemConfigurationAPI.h"
#include "ProxyRecord.h"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <list>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"

namespace proxy
{
NSString* __nonnull const NetworkingErrorDomain = @"com.cisco.NetworkingErrorDomain";
typedef NS_ENUM(NSInteger, NetworkingErrorType) {
    InvalidURLError = 1001,
};
/**
 * @brief Class that implements fake version of the ISystemConfiguration interface.
 */
class FakeSystemConfigurationAPI: public ISystemConfigurationAPI
{
public:
    struct CallbackParams
    {
        CFProxyAutoConfigurationResultCallback __nonnull cb;
        CFStreamClientContext* __nonnull pClientContext;
        CFArrayRef __nullable proxyList;
        CFErrorRef __nullable error;
    };
    
    ~FakeSystemConfigurationAPI();
    
    CFRunLoopSourceRef executeProxyAutoConfigurationURL(NSURL* testUrl, NSURL* scriptURL,
        CFProxyAutoConfigurationResultCallback cb,
        CFStreamClientContext* clientContext) override;
    
    NSDictionary* dynamicStoreCopyProxies() override;
    NSArray* copyProxiesForURL(NSURL* testUrl, NSDictionary* proxySettings) override;

    void addProxies(const std::string& strUrl, const std::list<ProxyRecord>& proxies);
    void addPACProxies(const std::string& strUrl, const std::list<ProxyRecord>& proxies);
    void setGeneratePACError(bool bVal);
    
private:
    NSArray* convertProxies(const std::list<ProxyRecord>& proxyList);
    static void callbackFunction(void* d);
    
    std::unordered_map<std::string, std::list<ProxyRecord>> proxyMap_;
    std::unordered_map<std::string, std::list<ProxyRecord>> proxyPACMap_;
    std::unordered_set<CallbackParams*> m_params;
    bool bGeneratePACError_ = false;
};

}

#pragma clang diagnostic pop
