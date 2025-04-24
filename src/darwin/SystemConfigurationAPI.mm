/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "SystemConfigurationAPI.h"

namespace proxy
{

CFRunLoopSourceRef SystemConfigurationAPI::executeProxyAutoConfigurationURL(NSURL* testUrl, NSURL* scriptURL,
    CFProxyAutoConfigurationResultCallback cb,
    CFStreamClientContext *clientContext)
{
    return CFNetworkExecuteProxyAutoConfigurationURL((__bridge CFURLRef)scriptURL, (__bridge CFURLRef)testUrl, cb, clientContext);
}

NSDictionary* SystemConfigurationAPI::dynamicStoreCopyProxies()
{
    NSDictionary* proxySettings =
    (__bridge_transfer NSDictionary*)SCDynamicStoreCopyProxies(nullptr);
    return proxySettings;
}

NSArray* SystemConfigurationAPI::copyProxiesForURL(NSURL* testUrl, NSDictionary* proxySettings)
{
    NSArray* systemProxies = (__bridge_transfer NSArray*)CFNetworkCopyProxiesForURL((__bridge CFURLRef)testUrl, (__bridge CFDictionaryRef)proxySettings);
    return systemProxies;
}

} //proxy namespace
