#pragma once

#import <CoreServices/CoreServices.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <Foundation/Foundation.h>
#import <CFNetwork/CFNetwork.h>

namespace proxy
{
/**
 * @brief An interface for the Mac SystemConfiguration API used in ProxyDiscovery library.
 */
class ISystemConfigurationAPI
{
public:
    virtual ~ISystemConfigurationAPI() = default;
    virtual CFRunLoopSourceRef executeProxyAutoConfigurationURL(NSURL* testUrl, NSURL* scriptURL,
        CFProxyAutoConfigurationResultCallback cb,
        CFStreamClientContext* clientContext) = 0;
    
    virtual NSDictionary* dynamicStoreCopyProxies() = 0;
    virtual NSArray* copyProxiesForURL(NSURL* testUrl, NSDictionary* proxySettings) = 0;
};

}
