/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "ProxyRecord.h"

namespace proxy {

ProxyRecord::ProxyRecord(std::string strUrl, uint32_t p, ProxyTypes prType):
    url(std::move(strUrl)),
    port(p),
    proxyType(prType)
{
}

std::unordered_map<ProxyTypes, std::string, ProxyTypesHasher> ProxyRecord::s_proxyTypeToName {
    {ProxyTypes::autoConfigurationURL, "autoConfigurationURL"},
    {ProxyTypes::autoConfigurationJavaScript, "autoConfigurationJavaScript"},
    {ProxyTypes::FTP, "ftp"},
    {ProxyTypes::HTTP, "http"},
    {ProxyTypes::HTTPS, "https"},
    {ProxyTypes::SOCKS, "socks"},
    {ProxyTypes::None, ""},
};

std::string ProxyRecord::getProxyTypeName() const
{
    std::string strRet;
    const auto it = s_proxyTypeToName.find(proxyType);
    if (it != s_proxyTypeToName.end())
        strRet = it->second;
    return strRet;
}

} //proxy namespace
