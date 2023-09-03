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
struct CallbackParams
{
    CFProxyAutoConfigurationResultCallback cb;
    CFStreamClientContext* pClientContext;
    CFArrayRef proxyList;
    CFErrorRef __nullable error;
};

void callbackFunction(void* d)
{
    CallbackParams* pParams = reinterpret_cast<CallbackParams*>(d);
    pParams->cb(pParams->pClientContext->info, pParams->proxyList, pParams->error);
}

} //unnamed namespace

namespace proxy
{
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
    NSMutableArray* arrayOfDictionaries = [NSMutableArray array];
    for (auto&& proxyRec: proxyList)
    {
        NSMutableDictionary* dict = [NSMutableDictionary dictionary];
        NSNumber* portNum = [NSNumber numberWithUnsignedInt: proxyRec.port];
        [dict setObject:portNum forKey: (__bridge NSString*)kCFProxyPortNumberKey];

        NSString* strUrl = [NSString stringWithUTF8String: proxyRec.url.c_str()];
        [dict setObject:strUrl forKey: (__bridge NSString*)kCFProxyHostNameKey];

        NSString* strType = [NSString stringWithUTF8String: proxyRec.proxyType.c_str()];
        [dict setObject:strType forKey: (__bridge NSString*)kCFProxyTypeKey];
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
	using namespace std::chrono_literals;
    NSString* nsStrUrl = [testUrl absoluteString];
    std::string strUrl = util::convertNSStringToStdString(nsStrUrl);
    NSArray* nsProxyArray = [NSMutableArray array];
    auto it = proxyPACMap_.find(strUrl);
    if (it != proxyPACMap_.end())
    {
        nsProxyArray = convertProxies(it->second);
    }
    CFArrayRef proxyArray = (__bridge_retained CFArrayRef)nsProxyArray;
    CallbackParams params = {
        cb, clientContext, proxyArray, nullptr
    };
    CFRunLoopSourceContext runLoopSourceContext = {
	    .version = 0,
	    .info = &params,
	    .retain = nullptr,
	    .release = nullptr,
	    .copyDescription = nullptr,
	    .equal = nullptr,
	    .hash = nullptr,
	    .schedule = nullptr,
	    .cancel = nullptr,
	    .perform = &callbackFunction
	};
	CFRunLoopSourceRef runLoopSource = CFRunLoopSourceCreate(nullptr, 0, &runLoopSourceContext);

	auto threadFn = [&runLoopSource]() {
        std::this_thread::sleep_for(100ms);

        CFRunLoopSourceSignal(runLoopSource);
    };
    std::thread th{threadFn};
    th.join();
    return runLoopSource;
}

} //namespace proxy
