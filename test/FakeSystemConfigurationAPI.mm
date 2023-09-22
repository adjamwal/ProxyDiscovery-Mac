/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "FakeSystemConfigurationAPI.hpp"
#include "StringUtil.hpp"

#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <CFNetwork/CFNetwork.h>

#include <chrono>
#include <thread>


namespace
{

NSError *createNetworkingError(proxy::NetworkingErrorType type, NSString* description) {
    NSDictionary* userInfo = @{NSLocalizedDescriptionKey: description};
    return [NSError errorWithDomain: proxy::NetworkingErrorDomain
                               code: type
                           userInfo: userInfo];
}

NSString* convertProxyTypeToProxyName(proxy::ProxyTypes proxyType)
{
    switch (proxyType)
    {
        case (proxy::ProxyTypes::autoConfigurationURL):
            return (__bridge NSString*)kCFProxyTypeAutoConfigurationURL;
        case (proxy::ProxyTypes::autoConfigurationJavaScript):
            return (__bridge NSString*)kCFProxyTypeAutoConfigurationJavaScript;
        case (proxy::ProxyTypes::FTP):
            return (__bridge NSString*)kCFProxyTypeFTP;
        case (proxy::ProxyTypes::HTTP):
            return (__bridge NSString*)kCFProxyTypeHTTP;
        case (proxy::ProxyTypes::HTTPS):
            return (__bridge NSString*)kCFProxyTypeHTTPS;
        case (proxy::ProxyTypes::SOCKS):
            return (__bridge NSString*)kCFProxyTypeSOCKS;
        case (proxy::ProxyTypes::None):
        default:
            return (__bridge NSString*)kCFProxyTypeNone;
    }
    return (__bridge NSString*)kCFProxyTypeNone;
}

}

namespace proxy
{

void FakeSystemConfigurationAPI::callbackFunction(void* d)
{
    auto* pParams = (CallbackParams*)d;
    pParams->cb(pParams->pClientContext->info, pParams->proxyList, pParams->error);
}

FakeSystemConfigurationAPI::~FakeSystemConfigurationAPI()
{
    for(auto* pParams: m_params)
    {
        delete pParams;
    }
}

NSDictionary* FakeSystemConfigurationAPI::dynamicStoreCopyProxies()
{
	//return any non empty dictionary
	//returning dictionary similar to the real one.
	NSMutableDictionary* dict = [NSMutableDictionary dictionary];

	[dict setObject:@1 forKey:@"SOCKSEnable"];
	[dict setObject:@1 forKey:@"HTTPEnable"];
	[dict setObject:@8182 forKey:@"HTTPPort"];
	[dict setObject:@"127.0.0.1" forKey:@"HTTPSProxy"];
	[dict setObject:@8182 forKey:@"HTTPSPort"];
	[dict setObject:@"127.0.0.1" forKey:@"HTTPProxy"];
	[dict setObject:@8197 forKey:@"SOCKSPort"];
	[dict setObject:@"127.0.0.1" forKey:@"SOCKSProxy"];
	[dict setObject:@1 forKey:@"HTTPSEnable"];

    return [NSDictionary dictionaryWithDictionary: dict];
}

NSArray* FakeSystemConfigurationAPI::convertProxies(const std::list<ProxyRecord>& proxyList)
{
    NSString* nsStrPACType = (__bridge NSString*)kCFProxyTypeAutoConfigurationURL;
    NSMutableArray* arrayOfDictionaries = [NSMutableArray array];
    for (auto&& proxyRec: proxyList)
    {
        NSMutableDictionary* dict = [NSMutableDictionary dictionary];
        NSString* strType = convertProxyTypeToProxyName(proxyRec.proxyType);
        if ([nsStrPACType isEqualToString: strType])
        {
            [dict setObject:strType forKey: (__bridge NSString*)kCFProxyTypeKey];
            
            NSString* strUrl = [NSString stringWithUTF8String: proxyRec.url.c_str()];
            NSURL* autoconfUrl = [NSURL URLWithString:strUrl];
            [dict setObject:autoconfUrl forKey: (__bridge NSString*)kCFProxyAutoConfigurationURLKey];
        }
        else
        {
            [dict setObject:strType forKey: (__bridge NSString*)kCFProxyTypeKey];
            
            NSNumber* portNum = [NSNumber numberWithUnsignedInt: proxyRec.port];
            [dict setObject:portNum forKey: (__bridge NSString*)kCFProxyPortNumberKey];
            
            NSString* strUrl = [NSString stringWithUTF8String: proxyRec.url.c_str()];
            [dict setObject:strUrl forKey: (__bridge NSString*)kCFProxyHostNameKey];
        }

        [arrayOfDictionaries addObject:dict];
    }
    return arrayOfDictionaries;
}

NSArray* FakeSystemConfigurationAPI::copyProxiesForURL(NSURL* testUrl, NSDictionary* proxySettings)
{
	NSString* nsStrUrl = [testUrl absoluteString];
	std::string strUrl = util::convertNSStringToStdString(nsStrUrl);
	auto it = proxyMap_.find(strUrl);
	if (it == proxyMap_.end())
		return nullptr;

	return convertProxies(it->second);
}

void FakeSystemConfigurationAPI::addProxies(const std::string& strUrl, const std::list<ProxyRecord>& proxies)
{
	proxyMap_.insert(std::make_pair(strUrl, proxies));
}

void FakeSystemConfigurationAPI::addPACProxies(const std::string& strUrl, const std::list<ProxyRecord>& proxies)
{
    proxyPACMap_.insert(std::make_pair(strUrl, proxies));
}

CFRunLoopSourceRef FakeSystemConfigurationAPI::executeProxyAutoConfigurationURL(NSURL* testUrl, NSURL* scriptURL,
        CFProxyAutoConfigurationResultCallback cb,
        CFStreamClientContext* clientContext)
{
    NSString* nsStrScriptUrl = [scriptURL absoluteString];
    std::string strScriptUrl = util::convertNSStringToStdString(nsStrScriptUrl);
    NSArray* nsProxyArray = [NSMutableArray array];
    auto it = proxyPACMap_.find(strScriptUrl);
    if (it != proxyPACMap_.end())
    {
        nsProxyArray = convertProxies(it->second);
    }
    CFArrayRef proxyArray = (__bridge_retained CFArrayRef)nsProxyArray;
    CallbackParams* pParams = nullptr;
    if (bGeneratePACError_)
    {
        NSError* er = createNetworkingError(proxy::InvalidURLError, @"The provided URL is invalid.");
        pParams = new CallbackParams {
            cb, clientContext, nullptr, (__bridge_retained CFErrorRef)er
        };
    }
    else
    {
        pParams = new CallbackParams {
            cb, clientContext, proxyArray, nullptr
        };
    }
    
    m_params.insert(pParams);
    CFRunLoopSourceContext runLoopSourceContext = {
	    0,
        pParams,
	    nullptr,
	    nullptr,
	    nullptr,
	    nullptr,
	    nullptr,
	    nullptr,
	    nullptr,
	    &FakeSystemConfigurationAPI::callbackFunction
	};
	CFRunLoopSourceRef runLoopSource = CFRunLoopSourceCreate(nullptr, 0, &runLoopSourceContext);

	auto threadFn = [&runLoopSource]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        CFRunLoopSourceSignal(runLoopSource);
    };
    std::thread th{threadFn};
    th.join();
    return runLoopSource;
}

void FakeSystemConfigurationAPI::setGeneratePACError(bool bVal)
{
    bGeneratePACError_ = bVal;
}

} //namespace proxy
