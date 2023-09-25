#pragma once

#include "ProxyDef.h"
#include <string>
#include <unordered_map>

namespace proxy
{

enum class PROXY_DISCOVERY_MODULE_API ProxyTypes
{
    autoConfigurationURL,
    autoConfigurationJavaScript,
    FTP,
    HTTP,
    HTTPS,
    SOCKS,
    None
};

struct ProxyTypesHasher
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

struct PROXY_DISCOVERY_MODULE_API ProxyRecord
{
    ProxyRecord(std::string strUrl, uint32_t p, ProxyTypes prType);
    std::string url;
    uint32_t port = 0;
    ProxyTypes proxyType = ProxyTypes::None;
    
    bool operator ==( const ProxyRecord& rhs ) const
    {
        return ( url == rhs.url ) && ( port == rhs.port ) && ( proxyType == rhs.proxyType );
    }
    
    bool operator !=( const ProxyRecord& rhs ) const
    {
        return !( *this == rhs );
    }
    
    std::string getProxyTypeName() const;
    
private:
    static std::unordered_map<ProxyTypes, std::string, ProxyTypesHasher> s_proxyTypeToName;
};
} //proxy namespace
