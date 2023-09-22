/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "ProxyDiscoveryEngine.h"

#include "ScopedGuard.hpp"
#include "ProxyLoggerDef.hpp"
#include "StringUtil.hpp"
#include "SystemConfigurationAPI.h"

#include <limits>

#import <CoreServices/CoreServices.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <Foundation/Foundation.h>
#import <CFNetwork/CFNetwork.h>

namespace
{

const std::string kPrivateRunLoopMode = "com.cisco.unified_connector.proxy_discovery";
const CFTimeInterval kVeryLongTimeInterval = 1.0e10;

void resultCallback(void* client, CFArrayRef proxies, CFErrorRef error)
{
    assert( (proxies != nullptr) == (error == nullptr) );
    CFTypeRef* resultPtr = (CFTypeRef*)client;
    assert(resultPtr != nullptr);
    assert(*resultPtr == nullptr);
    
    if (error != nullptr) {
        *resultPtr = CFRetain(error);
    } else {
        *resultPtr = CFRetain(proxies);
    }
    CFRunLoopStop(CFRunLoopGetCurrent());
}

void CFQRelease(CFTypeRef cf)
{
    if (cf != nullptr) {
        CFRelease(cf);
    }
}

void expandPACProxy(proxy::ISystemConfigurationAPI* pConfigurationAPI, NSURL* testUrl, NSURL* scriptURL, NSMutableArray* retProxies)
{
    CFTypeRef result = nullptr;
    CFStreamClientContext context = {0, &result, nullptr, nullptr, nullptr};
    CFRunLoopSourceRef rls = pConfigurationAPI->executeProxyAutoConfigurationURL(testUrl, scriptURL, resultCallback, &context);
     
    if (rls == nullptr)
    {
        PROXY_LOG_ERROR("Error occured during CFNetworkExecuteProxyAutoConfigurationURL call: returned CFRunLoopSourceRef is zero.");
        CFQRelease(result);
        return;
    }
    
    auto fnRelease = [&rls] () {
        CFQRelease(rls);
    };
    util::scoped_guard<decltype(fnRelease)> guard{fnRelease};
    
    NSString* nsPrivateRunLoopMode = [NSString stringWithUTF8String:kPrivateRunLoopMode.c_str()];
    CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, (__bridge CFStringRef)nsPrivateRunLoopMode);
    //Runloop should be stopped in resultCallback.
    //Suppose that resultCallback is always called by the SystemConfigurationAPI::executeProxyAutoConfigurationURL.
    CFRunLoopRunInMode((__bridge CFStringRef)nsPrivateRunLoopMode, kVeryLongTimeInterval, false);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, (__bridge CFStringRef)nsPrivateRunLoopMode);
    
    if (result == nullptr)
    {
        PROXY_LOG_ERROR("The result of the CFNetworkExecuteProxyAutoConfigurationURL call is zero.");
        return;
    }
    
    if ( CFGetTypeID(result) == CFErrorGetTypeID() ) {
        CFErrorRef cfError = (CFErrorRef)result;
        NSError* err = (__bridge_transfer NSError*)cfError;
        long errorCode = [err code];
        std::string errDomain = util::convertNSStringToStdString([err domain]);
        std::string errDescr = util::convertNSStringToStdString([err localizedDescription]);
        PROXY_LOG_ERROR("Error during CFNetworkExecuteProxyAutoConfigurationURL call occured: "
            "code: [%ld]. Domain: [%s]. Description: [%s].",
            errorCode, errDomain.c_str(), errDescr.c_str());
        return;
    } else if ( CFGetTypeID(result) == CFArrayGetTypeID() ) {
        NSArray* expandedProxies = (__bridge_transfer NSArray*)result;
        for (NSDictionary* dictionary in expandedProxies) {
            [retProxies addObject:dictionary];
        }
    } else {
        PROXY_LOG_ERROR("Unknown error occured during CFNetworkExecuteProxyAutoConfigurationURL call.");
        CFQRelease(result);
        return;
    }
}

NSArray* expandPACProxies(proxy::ISystemConfigurationAPI* pConfigurationAPI, NSURL* testUrl, NSArray* inputProxies)
{
    NSMutableArray* retProxies = [[NSMutableArray alloc] init];
    for (NSDictionary* dictionary in inputProxies) {
        NSString* proxyType = [dictionary objectForKey: (__bridge NSString*)kCFProxyTypeKey];
        if ([proxyType isEqualToString: (__bridge NSString*)kCFProxyTypeAutoConfigurationURL])
        {
            NSURL* scriptURL = [dictionary objectForKey: (__bridge NSURL*)kCFProxyAutoConfigurationURLKey];
            if (scriptURL == nullptr)
                continue;
            expandPACProxy(pConfigurationAPI, testUrl, scriptURL, retProxies);
        }
        else
        {
            [retProxies addObject:dictionary];
        }
       
    }
    return retProxies;
}

proxy::ProxyTypes convertProxyNameToProxyType(NSString* strProxy)
{
    proxy::ProxyTypes proxyType = proxy::ProxyTypes::None;
    if ([strProxy isEqualToString: (__bridge NSString*)kCFProxyTypeNone]) {
        proxyType = proxy::ProxyTypes::None;
    } else if ([strProxy isEqualToString:
                (__bridge NSString*)kCFProxyTypeAutoConfigurationURL]) {
        proxyType = proxy::ProxyTypes::autoConfigurationURL;
    } else if ([strProxy isEqualToString:
                (__bridge NSString*)kCFProxyTypeAutoConfigurationJavaScript]) {
        proxyType = proxy::ProxyTypes::autoConfigurationJavaScript;
    } else if ([strProxy isEqualToString:(__bridge NSString*)kCFProxyTypeFTP]) {
        proxyType = proxy::ProxyTypes::FTP;
    } else if ([strProxy isEqualToString:(__bridge NSString*)kCFProxyTypeHTTP]) {
        proxyType = proxy::ProxyTypes::HTTP;
    } else if ([strProxy isEqualToString:(__bridge NSString*)kCFProxyTypeHTTPS]) {
        proxyType = proxy::ProxyTypes::HTTPS;
    } else if ([strProxy isEqualToString:(__bridge NSString*)kCFProxyTypeSOCKS]) {
        proxyType = proxy::ProxyTypes::SOCKS;
    }
    return proxyType;
}

} //unnamed namespace

namespace proxy
{
ProxyDiscoveryEngine::ProxyDiscoveryEngine(
            std::shared_ptr<ISystemConfigurationAPI> pConfigurationAPI):
    m_pConfigurationAPI(std::move(pConfigurationAPI))
{
}

std::list<ProxyRecord> ProxyDiscoveryEngine::getProxiesInternal(const std::string& testUrlStr, const std::string &pacUrlStr)
{
    std::list<ProxyRecord> proxyList;
    
    NSDictionary* proxySettings = m_pConfigurationAPI->dynamicStoreCopyProxies();
    
    if (proxySettings == nullptr)
    {
        PROXY_LOG_WARNING("The SCDynamicStoreCopyProxies call returned zero dictionary as a proxy settings.");
        return proxyList;
    }
    
    NSString* nsTestUrlStr = [NSString stringWithUTF8String:testUrlStr.c_str()];
    NSURL* testUrl = [NSURL URLWithString: nsTestUrlStr];
    
    NSMutableArray* proxies = [[NSMutableArray alloc] init];
    if (!pacUrlStr.empty())
    {
        PROXY_LOG_DEBUG("Pac url is provided for the proxy discovery: %s", pacUrlStr.c_str());
        NSString* nsPacUrlStr = [NSString stringWithUTF8String:pacUrlStr.c_str()];
        NSURL* pacUrl = [NSURL URLWithString: nsPacUrlStr];
        expandPACProxy(m_pConfigurationAPI.get(), testUrl, pacUrl, proxies);
    }
    
    NSArray* systemProxies = m_pConfigurationAPI->copyProxiesForURL(testUrl, proxySettings);
    
    NSArray* expandedProxies = expandPACProxies(m_pConfigurationAPI.get(), testUrl, systemProxies);
    [proxies addObjectsFromArray:expandedProxies];
    
    for (NSDictionary* dictionary in proxies) {
        //TODO: We might need to extend ProxyRecord type to include
        //credentials fields: kCFProxyUsernameKey, kCFProxyPasswordKey.
        NSString* nsStrProxyType = [dictionary objectForKey:(__bridge NSString*)kCFProxyTypeKey];
        auto proxyType = convertProxyNameToProxyType(nsStrProxyType);
        
        NSString* nsStrHost = [dictionary objectForKey:(__bridge NSString*)kCFProxyHostNameKey];
        
        NSString* nsStrUrl = [dictionary objectForKey:(__bridge NSString*)kCFProxyAutoConfigurationURLKey];
        
        std::string url;
        if (nsStrUrl == nullptr)
        {
            if (nsStrHost != nullptr)
            {
                url = util::convertNSStringToStdString(nsStrHost);
            }
            else
            {
                PROXY_LOG_DEBUG("Unable to determine proxy URL: both kCFProxyHostNameKey and kCFProxyHostNameKey are empty in the dictionary.");
                continue;
            }
        }
        else
        {
            url = util::convertNSStringToStdString(nsStrUrl);
        }
        
        NSNumber* nsPort = [dictionary objectForKey:(__bridge NSNumber*)kCFProxyPortNumberKey];
        uint32_t port = [nsPort intValue];
        
        ProxyRecord proxy{url, port, proxyType};
        proxyList.push_back(proxy);
    }
    
    return proxyList;
}

void ProxyDiscoveryEngine::addObserver(IProxyObserver* pObserver)
{
    m_observers.push_back(pObserver);
}

void ProxyDiscoveryEngine::requestProxiesAsync(const std::string& testUrl, const std::string &pacUrlStr, const std::string& guid)
{
    if (m_thread && m_thread->joinable())
    {
        //wait for the previous discovery completed.
        m_thread->join();
    }
    m_thread = std::make_shared<std::thread>([this, testUrl, pacUrlStr, guid](){
        std::list<ProxyRecord> proxies = getProxiesInternal(testUrl, pacUrlStr);
        notifyObservers(proxies, guid);
    });
}

void ProxyDiscoveryEngine::waitPrevOpCompleted()
{
    if (m_thread && m_thread->joinable())
    {
        //wait for the previous discovery completed.
        m_thread->join();
    }
}

void ProxyDiscoveryEngine::notifyObservers(const std::list<ProxyRecord>& proxies, const std::string& guid)
{
    for(auto* pObserver: m_observers)
    {
        if (pObserver) {
            //TODO: maybe we need to call observer on the main thread?
            pObserver->updateProxyList(proxies, guid);
        }
    }
}

std::list<ProxyRecord> ProxyDiscoveryEngine::getProxies(const std::string& testUrl, const std::string &pacUrl)
{
    //We need to call getProxiesInternal in a separate thread even for the
    //synchronous call since expandPACProxy function is running event loop.
    //if we do it in separate thread event loop of this separate thread will be run.
    //Event loop of the main thread is not touched.
    if (m_threadSync)
    {
        //wait for the previous discovery completed.
        m_threadSync->join();
    }
    std::list<ProxyRecord> proxies;
    m_threadSync = std::make_shared<std::thread>([this, &testUrl, &pacUrl, &proxies](){
        proxies = getProxiesInternal(testUrl, pacUrl);
    });
    //Wait for the thread to make call synchronous
    m_threadSync->join();
    return proxies;
}

ProxyDiscoveryEngine::~ProxyDiscoveryEngine()
{
    //wait for the threads completion
    if (m_thread && m_thread->joinable())
        m_thread->join();
    
    if (m_threadSync && m_threadSync->joinable())
        m_threadSync->join();
}

} //proxy namespace
