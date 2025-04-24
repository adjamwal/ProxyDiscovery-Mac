#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "FakeSystemConfigurationAPI.hpp"
#include "ProxyDiscoveryEngine.h"
#include "ProxyRecord.h"
#include "StringUtil.hpp"

#include <memory>
#include <string>

namespace
{
    const std::string kEmptyPACUrl = "";
}

class TestProxyDiscoveryPAC : public ::testing::Test
{
public:
    TestProxyDiscoveryPAC(): pSystemAPI_(std::make_shared<proxy::FakeSystemConfigurationAPI>()),
        pProxyDiscoveryEngine_(std::make_shared<proxy::ProxyDiscoveryEngine>(pSystemAPI_))
    {
        systemProxies_.push_back({"proxy1.example.com", 8081, proxy::ProxyTypes::HTTP});
        systemProxies_.push_back({strPACScriptUrl_, 8081, proxy::ProxyTypes::autoConfigurationURL});
        systemProxies_.push_back({"proxy3.example.com", 1080, proxy::ProxyTypes::SOCKS});
        pSystemAPI_->addProxies(strUrl_, systemProxies_);
        pSystemAPI_->addPACProxies(strPACScriptUrl_, pacProxies_);
    }
    
protected:
    std::string strUrl_ = "https://www.wikipedia.org";
    std::string strPACScriptUrl_ = "https://pac.autoconf.com";
    std::list<proxy::ProxyRecord> systemProxies_;
    std::list<proxy::ProxyRecord> pacProxies_ =
        { { "proxy1.example1.com", 8081, proxy::ProxyTypes::HTTP },
        { "proxy2.example1.com", 3129, proxy::ProxyTypes::HTTPS },
        { "proxy3.example1.com", 1081, proxy::ProxyTypes::SOCKS } };
    std::list<proxy::ProxyRecord> expectedProxies_ =
        { { "proxy1.example.com", 8081, proxy::ProxyTypes::HTTP },
        { "proxy1.example1.com", 8081, proxy::ProxyTypes::HTTP },
        { "proxy2.example1.com", 3129, proxy::ProxyTypes::HTTPS },
        { "proxy3.example1.com", 1081, proxy::ProxyTypes::SOCKS },
        {"proxy3.example.com", 1080, proxy::ProxyTypes::SOCKS } };
    std::list<proxy::ProxyRecord> expectedProxiesOnPacError_ =
        { { "proxy1.example.com", 8081, proxy::ProxyTypes::HTTP },
        {"proxy3.example.com", 1080, proxy::ProxyTypes::SOCKS } };
    std::shared_ptr<proxy::FakeSystemConfigurationAPI> pSystemAPI_;
    std::shared_ptr<proxy::ProxyDiscoveryEngine> pProxyDiscoveryEngine_;
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
        pProxyDiscoveryEngine_(std::make_shared<proxy::ProxyDiscoveryEngine>(pSystemAPI_)) {
    }

protected:
	std::shared_ptr<proxy::FakeSystemConfigurationAPI> pSystemAPI_;
    std::shared_ptr<proxy::ProxyDiscoveryEngine> pProxyDiscoveryEngine_;

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
        "https://www.youtube.com",
        {{ "proxy1.example.com", 8080, proxy::ProxyTypes::HTTP },
        { "proxy2.example.com", 3128, proxy::ProxyTypes::HTTPS },
        { "proxy3.example.com", 1080, proxy::ProxyTypes::SOCKS }}
    ),
    std::make_pair<std::string, std::list<proxy::ProxyRecord>>(
        "https://www.cisco.com",
        {}
    ),
    std::make_pair<std::string, std::list<proxy::ProxyRecord>>(
        "https://www.wikipedia.org",
        {{ "proxy1.example1.com", 8081, proxy::ProxyTypes::HTTP },
        { "proxy2.example1.com", 3129, proxy::ProxyTypes::HTTPS },
        { "proxy3.example1.com", 1081, proxy::ProxyTypes::SOCKS }}
    )
));


int main(int argc, char** argv) {
    @autoreleasepool {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
}
