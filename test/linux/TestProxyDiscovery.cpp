/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "MockCommandExec.hpp"
#include "MockProxyVerifier.hpp"
#include "ProxyDiscoveryEngine.hpp"

using testing::StrictMock;
using testing::Return;
using testing::_;

namespace proxy {

class TestProxyDiscovery : public ::testing::Test
{
protected:
   void SetUp() override
   {
      commandExecutorPtr_ = std::make_shared<MockCommandExec>();
      proxyVerifierPtr_ = std::make_shared<MockProxyVerifier>();
      proxyDiscoveryEngine_ = std::make_unique<ProxyDiscoveryEngine>(commandExecutorPtr_, proxyVerifierPtr_);
   }
   void TearDown() override
   {
   }
   std::shared_ptr<MockCommandExec> commandExecutorPtr_;
   std::shared_ptr<MockProxyVerifier> proxyVerifierPtr_;
   std::unique_ptr<ProxyDiscoveryEngine> proxyDiscoveryEngine_;
};

const std::string XDG_CURRENT_DESKTOP{ "XDG_CURRENT_DESKTOP" };
const std::string HTTP_PROXY{ "http_proxy" };
const std::string HTTPS_PROXY{ "https_proxy" };
const std::string SOCKS_PROXY( "socks_proxy" );
const std::string FTP_PROXY( "ftp_proxy" );
const std::string ALL_PROXY( "all_proxy" );
const std::string test_url{ "test_url" };

const std::string valid_http_host{ "httpproxy.com" };
const std::string valid_https_host{ "httpsproxy.com" };
const std::string valid_socks_host( "socksproxy.com" );
const std::string valid_ftp_host( "ftpproxy.com" );

const uint32_t valid_http_port{8080};
const uint32_t valid_https_port{3333};
const uint32_t valid_socks_port{8080};
const uint32_t valid_ftp_port{3333};

const std::string valid_http_url{ "http://" + valid_http_host };
const std::string valid_https_url{ "https://" + valid_https_host };
const std::string valid_socks_url{ "socks5://" + valid_socks_host };
const std::string valid_ftp_url{ "http://" + valid_ftp_host };

const std::string valid_http_url_port{ valid_http_url + ":" + std::to_string(valid_http_port)};
const std::string valid_https_url_port{ valid_https_url + ":" + std::to_string(valid_https_port)};
const std::string valid_socks_url_port{ valid_socks_url + ":" + std::to_string(valid_socks_port)};
const std::string valid_ftp_url_port{ valid_ftp_url + ":" + std::to_string(valid_ftp_port)};

const std::string authenticated_http{ "http://username:password@httpproxy.com" };
const std::string authenticated_https{ "https://username:password@httpsproxy.com" };


TEST_F(TestProxyDiscovery, validEnvUrls)
{  
   auto &commandExecutor{ *commandExecutorPtr_ };
   auto &proxyVerifier{ *proxyVerifierPtr_ };

   std::list<ProxyRecord> expectedProxies = {
      { valid_http_url_port, valid_http_port, ProxyTypes::HTTP },
      { valid_https_url_port, valid_https_port, ProxyTypes::HTTPS },
      { valid_socks_url_port, valid_socks_port, ProxyTypes::SOCKS },
      { valid_ftp_url_port, valid_ftp_port, ProxyTypes::FTP }
   };

   EXPECT_CALL(commandExecutor, getEnvironmentVar(XDG_CURRENT_DESKTOP)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTP_PROXY)).WillOnce(testing::Return(valid_http_url_port));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTPS_PROXY)).WillOnce(testing::Return(valid_https_url_port));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(SOCKS_PROXY)).WillOnce(testing::Return(valid_socks_url_port));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(FTP_PROXY)).WillOnce(testing::Return(valid_ftp_url_port));
   EXPECT_CALL(proxyVerifier, verifyProxy(_,_)).Times(4).WillRepeatedly(testing::Return(true));
   auto actualProxies = proxyDiscoveryEngine_->getProxies(test_url, "");
   EXPECT_EQ(actualProxies.size(), expectedProxies.size());
   for (auto it = actualProxies.begin(), eit = expectedProxies.begin(); it != actualProxies.end() && eit != expectedProxies.end(); ++it, ++eit) {
      EXPECT_EQ(*it, *eit);
   }
}
TEST_F(TestProxyDiscovery, validEnvUrlsDefaultPorts)
{  
   auto &commandExecutor{ *commandExecutorPtr_ };
   auto &proxyVerifier{ *proxyVerifierPtr_ };

   std::list<ProxyRecord> expectedProxies = {
      { valid_http_url, 80, ProxyTypes::HTTP },
      { valid_https_url, 443, ProxyTypes::HTTPS },
      { valid_socks_url, 1080, ProxyTypes::SOCKS },
      { valid_ftp_url, 80, ProxyTypes::FTP},
   };

   EXPECT_CALL(commandExecutor, getEnvironmentVar(XDG_CURRENT_DESKTOP)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTP_PROXY)).WillOnce(testing::Return(valid_http_url));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTPS_PROXY)).WillOnce(testing::Return(valid_https_url));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(SOCKS_PROXY)).WillOnce(testing::Return(valid_socks_url));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(FTP_PROXY)).WillOnce(testing::Return(valid_ftp_url));
   EXPECT_CALL(proxyVerifier, verifyProxy(_,_)).Times(4).WillRepeatedly(testing::Return(true));
   auto actualProxies = proxyDiscoveryEngine_->getProxies(test_url, "");
   EXPECT_EQ(actualProxies.size(), expectedProxies.size());
   for (auto it = actualProxies.begin(), eit = expectedProxies.begin(); it != actualProxies.end() && eit != expectedProxies.end(); ++it, ++eit) {
      EXPECT_EQ(*it, *eit);
   }
}

TEST_F(TestProxyDiscovery, invalidEnvUrls)
{  
   auto &commandExecutor{ *commandExecutorPtr_ };
   auto &proxyVerifier{ *proxyVerifierPtr_ };

   EXPECT_CALL(commandExecutor, getEnvironmentVar(XDG_CURRENT_DESKTOP)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTP_PROXY)).WillOnce(testing::Return("this is not a url"));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTPS_PROXY)).WillOnce(testing::Return("hello world"));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(SOCKS_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(FTP_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(ALL_PROXY)).WillOnce(testing::Return(""));
   auto actualProxies = proxyDiscoveryEngine_->getProxies(test_url, "");
   EXPECT_EQ(actualProxies.size(), 0);
}


TEST_F(TestProxyDiscovery, authenticatedUrls)
{  
   auto &commandExecutor{ *commandExecutorPtr_ };
   auto &proxyVerifier{ *proxyVerifierPtr_ };

   EXPECT_CALL(commandExecutor, getEnvironmentVar(XDG_CURRENT_DESKTOP)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTP_PROXY)).WillOnce(testing::Return(authenticated_http));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTPS_PROXY)).WillOnce(testing::Return(authenticated_https));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(SOCKS_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(FTP_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(ALL_PROXY)).WillOnce(testing::Return(""));
   auto actualProxies = proxyDiscoveryEngine_->getProxies(test_url, "");
   EXPECT_EQ(actualProxies.size(), 0);
}

TEST_F(TestProxyDiscovery, validGnomeUrls)
{  
   auto &commandExecutor{ *commandExecutorPtr_ };
   auto &proxyVerifier{ *proxyVerifierPtr_ };

   std::list<ProxyRecord> expectedProxies = {
      { valid_http_url, valid_http_port, ProxyTypes::HTTP },
      { valid_https_url, valid_https_port, ProxyTypes::HTTPS }
   };

   EXPECT_CALL(commandExecutor, getEnvironmentVar(XDG_CURRENT_DESKTOP)).WillOnce(testing::Return("GNOME"));
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_)).Times(9)
                                 .WillOnce(testing::Return(CommandOutput{0, "'manual'"}))
                                 .WillOnce(testing::Return(CommandOutput{0, valid_http_host}))
                                 .WillOnce(testing::Return(CommandOutput{0, "8080"}))
                                 .WillOnce(testing::Return(CommandOutput{0, valid_https_host}))
                                 .WillOnce(testing::Return(CommandOutput{0, "3333"}))
                                 .WillOnce(testing::Return(CommandOutput{0, ""}))
                                 .WillOnce(testing::Return(CommandOutput{0, ""}))
                                 .WillOnce(testing::Return(CommandOutput{0, ""}))
                                 .WillOnce(testing::Return(CommandOutput{0, ""}));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTP_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTPS_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(SOCKS_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(FTP_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(ALL_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(proxyVerifier, verifyProxy(_,_)).Times(2).WillOnce(testing::Return(true)).WillOnce(testing::Return(true));
   auto actualProxies = proxyDiscoveryEngine_->getProxies(test_url, "");
   EXPECT_EQ(actualProxies.size(), expectedProxies.size());
   for (auto it = actualProxies.begin(), eit = expectedProxies.begin(); it != actualProxies.end() && eit != expectedProxies.end(); ++it, ++eit) {
      EXPECT_EQ(*it, *eit);
   }
}

TEST_F(TestProxyDiscovery, gnomeModeUnknown)
{  
   auto &commandExecutor{ *commandExecutorPtr_ };
   auto &proxyVerifier{ *proxyVerifierPtr_ };

   EXPECT_CALL(commandExecutor, getEnvironmentVar(XDG_CURRENT_DESKTOP)).WillOnce(testing::Return("GNOME"));
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_)).WillOnce(testing::Return(CommandOutput{0, "'invalid mode'"}));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTP_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTPS_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(SOCKS_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(FTP_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(ALL_PROXY)).WillOnce(testing::Return(""));
   auto actualProxies = proxyDiscoveryEngine_->getProxies(test_url, "");
   EXPECT_EQ(actualProxies.size(), 0);
}

TEST_F(TestProxyDiscovery, gnomeModeCommandExecError)
{  
   auto &commandExecutor{ *commandExecutorPtr_ };
   auto &proxyVerifier{ *proxyVerifierPtr_ };

   EXPECT_CALL(commandExecutor, getEnvironmentVar(XDG_CURRENT_DESKTOP)).WillOnce(testing::Return("GNOME"));
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_)).WillOnce(testing::Return(CommandOutput{-1, "'manual'"}));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTP_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(HTTPS_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(SOCKS_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(FTP_PROXY)).WillOnce(testing::Return(""));
   EXPECT_CALL(commandExecutor, getEnvironmentVar(ALL_PROXY)).WillOnce(testing::Return(""));
   auto actualProxies = proxyDiscoveryEngine_->getProxies(test_url, "");
   EXPECT_EQ(actualProxies.size(), 0);
}

} //proxy

int main(int argc, char **argv) {
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
} 