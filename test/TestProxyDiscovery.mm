#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "FakeSystemConfigurationAPI.hpp"
#include "ProxyDiscoveryEngine.h"
#include "ProxyRecord.h"
#include "StringUtil.hpp"

#include <memory>
#include <string>


using namespace std::string_literals;

namespace
{
    const std::string kEmptyPACUrl = ""s; 
}

class TestProxyDiscoveryPAC : public ::testing::Test
{
public:
    TestProxyDiscoveryPAC(): pSystemAPI_(std::make_shared<proxy::FakeSystemConfigurationAPI>()),
        pProxyDiscoveryEngine_(std::make_unique<proxy::ProxyDiscoveryEngine>(pSystemAPI_))
    {
        NSString* nStrPACType = (__bridge NSString*)kCFProxyTypeAutoConfigurationURL;
        std::string strPACType = util::convertNSStringToStdString(nStrPACType);
        systemProxies_.push_back({"proxy1.example.com"s, 8081, "HTTP"s});
        systemProxies_.push_back({strPACScriptUrl_, 8081, strPACType});
        systemProxies_.push_back({"proxy3.example.com"s, 1080, "SOCKS"s});
        pSystemAPI_->addProxies(strUrl_, systemProxies_);
        pSystemAPI_->addPACProxies(strPACScriptUrl_, pacProxies_);
    }
    
protected:
    std::string strUrl_ = "https://www.wikipedia.org"s;
    std::string strPACScriptUrl_ = "https://pac.autoconf.com"s;
    std::list<proxy::ProxyRecord> systemProxies_;
    std::list<proxy::ProxyRecord> pacProxies_ =
        { { "proxy1.example1.com"s, 8081, "HTTP"s },
        { "proxy2.example1.com"s, 3129, "HTTPS"s },
        { "proxy3.example1.com"s, 1081, "SOCKS"s } };
    std::list<proxy::ProxyRecord> expectedProxies_ =
        { { "proxy1.example.com"s, 8081, "HTTP"s },
        { "proxy1.example1.com"s, 8081, "HTTP"s },
        { "proxy2.example1.com"s, 3129, "HTTPS"s },
        { "proxy3.example1.com"s, 1081, "SOCKS"s },
        {"proxy3.example.com"s, 1080, "SOCKS"s } };
    std::list<proxy::ProxyRecord> expectedProxiesOnPacError_ =
        { { "proxy1.example.com"s, 8081, "HTTP"s },
        {"proxy3.example.com"s, 1080, "SOCKS"s } };
    std::shared_ptr<proxy::FakeSystemConfigurationAPI> pSystemAPI_;
    std::unique_ptr<proxy::ProxyDiscoveryEngine> pProxyDiscoveryEngine_;
};

TEST_F(TestProxyDiscoveryPAC, CheckReturningPACProxies)
{
    std::list<proxy::ProxyRecord> proxyList = pProxyDiscoveryEngine_->getProxies(strUrl_, kEmptyPACUrl);
    ASSERT_THAT(proxyList, ::testing::ContainerEq(expectedProxies_));
}

TEST_F(TestProxyDiscoveryPAC, CheckReturningPACProxiesOnError)
{
    pSystemAPI_->setGeneratePACError(true);
    std::list<proxy::ProxyRecord> proxyList = pProxyDiscoveryEngine_->getProxies(strUrl_, kEmptyPACUrl);
    ASSERT_THAT(proxyList, ::testing::ContainerEq(expectedProxiesOnPacError_));
}

class TestProxyDiscovery : public ::testing::TestWithParam<std::pair<std::string, std::list<proxy::ProxyRecord>>> {
public:
	TestProxyDiscovery():
        pSystemAPI_(std::make_shared<proxy::FakeSystemConfigurationAPI>()),
        pProxyDiscoveryEngine_(std::make_unique<proxy::ProxyDiscoveryEngine>(pSystemAPI_)) {
    }

protected:
	std::shared_ptr<proxy::FakeSystemConfigurationAPI> pSystemAPI_;
    std::unique_ptr<proxy::ProxyDiscoveryEngine> pProxyDiscoveryEngine_;

};

TEST_P(TestProxyDiscovery, CheckReturningCorrectProxyList)
{
    std::pair<std::string, std::list<proxy::ProxyRecord>> proxyData = GetParam();
    pSystemAPI_->addProxies(proxyData.first, proxyData.second);
    std::list<proxy::ProxyRecord> proxyList = pProxyDiscoveryEngine_->getProxies(proxyData.first, kEmptyPACUrl);
    ASSERT_THAT(proxyList, ::testing::ContainerEq(proxyData.second));
}

INSTANTIATE_TEST_SUITE_P(Default, TestProxyDiscovery, ::testing::Values(
    std::make_pair<std::string, std::list<proxy::ProxyRecord>>(
        "https://www.youtube.com"s,
        {{ "proxy1.example.com"s, 8080, "HTTP"s },
        { "proxy2.example.com"s, 3128, "HTTPS"s },
        { "proxy3.example.com"s, 1080, "SOCKS"s }}
    ),
    std::make_pair<std::string, std::list<proxy::ProxyRecord>>(
        "https://www.cisco.com"s,
        {}
    ),
    std::make_pair<std::string, std::list<proxy::ProxyRecord>>(
        "https://www.wikipedia.org"s,
        {{ "proxy1.example1.com"s, 8081, "HTTP"s },
        { "proxy2.example1.com"s, 3129, "HTTPS"s },
        { "proxy3.example1.com"s, 1081, "SOCKS"s }}
    )
));


int main(int argc, char** argv) {
    @autoreleasepool {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
}
