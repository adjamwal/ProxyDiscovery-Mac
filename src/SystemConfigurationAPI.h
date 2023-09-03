#pragma once

#include "ISystemConfigurationAPI.h"

namespace proxy
{
/**
 * @brief Class that implements ISystemConfiguration interface.
 */
class SystemConfigurationAPI: public ISystemConfigurationAPI
{
public:
    CFRunLoopSourceRef executeProxyAutoConfigurationURL(NSURL* testUrl, NSURL* scriptURL,
        CFProxyAutoConfigurationResultCallback cb,
        CFStreamClientContext* clientContext) override;
    
    NSDictionary* dynamicStoreCopyProxies() override;
    NSArray* copyProxiesForURL(NSURL* testUrl, NSDictionary* proxySettings) override;
};

}
