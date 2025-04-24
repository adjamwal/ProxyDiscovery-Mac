/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include "ProxyDiscoveryEngine.hpp"
#include "ProxyLoggerDef.hpp"
#include <regex>

namespace proxy {

ProxyDiscoveryEngine::ProxyDiscoveryEngine(std::shared_ptr<IProxyCommandExec> commandExecutor, std::shared_ptr<IProxyVerifier> proxyVerifier) : m_commandExecutor(commandExecutor), m_proxyVerifier(proxyVerifier) {}

ProxyDiscoveryEngine::~ProxyDiscoveryEngine() {
    waitPrevOpCompleted();
};

//gsettings returns values surrounded by single quotes
static void _trim_gsettings_output(std::string &output)
{
    size_t start = output.find_first_of("'");
    if (start == std::string::npos) {
        return;
    }
    size_t end = output.find_last_of("'");
    if (start == end) {
        return;
    }
    output = output.substr(start + 1, end - start - 1);
}

static std::string _construct_url(const std::string& host, const std::string& port, const std::string& protocol) {
    if (host.empty()) {
        PROXY_LOG_ERROR("Could not construct proxy URL, host is empty");
        return "";
    }
    std::string url{ protocol + "://" + host };
    if (port.empty()) {
        url = url + ":" + port;
    }
    return url;
}


static uint32_t _get_port(const std::string& url) {
    std::regex urlPattern{R"(^(?:https?|socks5|socks4|ftp)://[a-zA-Z0-9._-]+(?:\:\d+)$)"};
    if (!std::regex_match(url, urlPattern)) {
        if (url.rfind("https://", 0) == 0) {
            return 443;
        } else if (url.rfind("http://", 0) == 0) {
            return 80;
        } else if (url.rfind("socks5://", 0) == 0) {
            return 1080;
        } else if (url.rfind("socks4://", 0) == 0) {
            return 1080;
        }
        return 0;
    }
    return static_cast<uint32_t>(std::stoi(url.substr(url.find_last_of(":") + 1)));
}

static bool _valid_url(const std::string& url) {
    std::regex authenticatedUrlPattern{R"(^(https?|socks5|socks4|ftp)://[^:@]+:[^/:]+@[^/:]+(:\d+)?)"};
    std::regex urlPattern{R"(^(?:https?|socks5|socks4|ftp)://[a-zA-Z0-9._-]+(?:\:\d+)?$)"};
    if (std::regex_match(url, authenticatedUrlPattern)) {
        PROXY_LOG_WARNING("Authenticated proxy servers not supported");
        return false;
    } 
    return std::regex_match(url, urlPattern);
}

void ProxyDiscoveryEngine::addObserver(IProxyObserver& pObserver) {
    m_observers.push_back(&pObserver);
}

void ProxyDiscoveryEngine::requestProxiesAsync(const std::string &testUrl, const std::string &, const std::string &guid)    {
    waitPrevOpCompleted();
    m_thread = std::make_shared<std::thread>([this, testUrl, guid](){
        std::list<ProxyRecord> proxySettings = getProxiesInternal();
        proxySettings.remove_if([this, &testUrl](const ProxyRecord &proxy) { return !m_proxyVerifier->verifyProxy(testUrl, proxy); });
        notifyObservers(proxySettings, guid);
    });
}

void ProxyDiscoveryEngine::waitPrevOpCompleted() {
    if (m_thread && m_thread->joinable()) {
        //wait for the previous discovery completed.
        m_thread->join();
    }
}

void ProxyDiscoveryEngine::notifyObservers(const std::list<ProxyRecord> &proxies, const std::string &guid)
{
    for(auto* pObserver: m_observers)
    {
        pObserver->updateProxyList(proxies, guid);
    }
}

std::list<ProxyRecord> ProxyDiscoveryEngine::getProxiesInternal() {
    std::list<ProxyRecord> proxySettings;
    std::string desktop = m_commandExecutor->getEnvironmentVar("XDG_CURRENT_DESKTOP");
    try {
        if (desktop.find("GNOME") != std::string::npos) {
            proxySettings = gnomeProxy();
        } else if (desktop.find("KDE") != std::string::npos) { 
            proxySettings = kdeProxy();
        }

        bool httpSet = false;
        const std::string httpProxy{ m_commandExecutor->getEnvironmentVar("http_proxy") };
        if (_valid_url(httpProxy)) {
            proxySettings.push_back({httpProxy, _get_port(httpProxy), ProxyTypes::HTTP});
            httpSet = true;
        }

        bool httpsSet = false;
        const std::string httpsProxy{ m_commandExecutor->getEnvironmentVar("https_proxy") };
        if (_valid_url(httpsProxy)) {
            proxySettings.push_back({httpsProxy, _get_port(httpsProxy), ProxyTypes::HTTPS});
            httpsSet = true;
        }

        bool socksSet = false;
        const std::string socksProxy{ m_commandExecutor->getEnvironmentVar("socks_proxy") };
        if (_valid_url(socksProxy)) {
            proxySettings.push_back({socksProxy, _get_port(socksProxy), ProxyTypes::SOCKS});
            socksSet = true;
        }

        bool ftpSet = false;
        const std::string ftpProxy{ m_commandExecutor->getEnvironmentVar("ftp_proxy") };
        if (_valid_url(ftpProxy)) {
            proxySettings.push_back({ftpProxy, _get_port(ftpProxy), ProxyTypes::FTP});
            ftpSet = true;
        }

        if (!httpSet || !httpsSet || !socksSet || !ftpSet) {
            const std::string allProxy{ m_commandExecutor->getEnvironmentVar("all_proxy") };
            if (_valid_url(allProxy)) {
                if (!httpSet) {
                    proxySettings.push_back({allProxy, _get_port(allProxy), ProxyTypes::HTTP});
                }
                if (!httpsSet) {
                    proxySettings.push_back({allProxy, _get_port(allProxy), ProxyTypes::HTTPS});
                }  
                if (!socksSet) {
                    proxySettings.push_back({allProxy, _get_port(allProxy), ProxyTypes::SOCKS});
                }
                if (!ftpSet) {
                    proxySettings.push_back({allProxy, _get_port(allProxy), ProxyTypes::FTP});
                }
            }
        }
    } catch (const std::exception &e) {
        PROXY_LOG_ERROR("Caught exception %s", e.what());
    }

    return proxySettings;
}

std::list<ProxyRecord> ProxyDiscoveryEngine::getProxies(const std::string& testUrl, const std::string &) {
    std::list<ProxyRecord> proxySettings = getProxiesInternal();
    proxySettings.remove_if([this, &testUrl](const ProxyRecord &proxy) { return !m_proxyVerifier->verifyProxy(testUrl, proxy); });
    return proxySettings;
}


std::list<ProxyRecord> ProxyDiscoveryEngine::gnomeProxy() {

    std::list<ProxyRecord> records;
    const std::string gsettingsCmd{ "/usr/bin/gsettings" };
    std::vector<std::string> modeCmd{gsettingsCmd, "get", "org.gnome.system.proxy", "mode"};

    try {
        CommandOutput modeOutput = m_commandExecutor->ExecuteCommandCaptureOutput(gsettingsCmd, modeCmd);
        if (0 != modeOutput.exitCode_) {
            PROXY_LOG_ERROR("Error obtaining gnome proxy mode");
            return records;
        }

        _trim_gsettings_output(modeOutput.output_);
    
        if (modeOutput.output_ == "none") {
            PROXY_LOG_INFO("Proxy disabled in gnome settings");
            return records;
        } else if (modeOutput.output_ == "auto") {
            //pac not supported (yet?)
            PROXY_LOG_WARNING("Proxy auto configuration not supported");
            return records;
        } else if (modeOutput.output_ == "manual") {
            auto httpProxy = parseGnomeProxy("http");
            if (httpProxy.proxyType != ProxyTypes::None){
                records.push_back(std::move(httpProxy));
            }
            auto httpsProxy = parseGnomeProxy("https");
            if (httpsProxy.proxyType != ProxyTypes::None){
                records.push_back(std::move(httpsProxy));
            }
            auto ftpProxy = parseGnomeProxy("ftp");
            if (ftpProxy.proxyType != ProxyTypes::None){
                records.push_back(std::move(ftpProxy));
            }
            auto socksProxy = parseGnomeProxy("socks");
            if (socksProxy.proxyType != ProxyTypes::None){
                records.push_back(std::move(socksProxy));
            }
        } else {
            PROXY_LOG_WARNING("Unrecognized proxy mode");
        }
    } catch (const std::exception &e) {
        PROXY_LOG_ERROR("Command executor threw exception %s", e.what());
    }
    return records;
}

ProxyRecord ProxyDiscoveryEngine::parseGnomeProxy(const std::string& protocol) {
    ProxyTypes proxyType;
    std::string urlPrefix{ protocol };
    uint32_t defaultPort{ 0 };
    if (protocol == "http") {
        proxyType = ProxyTypes::HTTP;
        defaultPort = 80;
    } else if (protocol == "https") {
        proxyType = ProxyTypes::HTTPS;
        defaultPort = 443;
    } else if (protocol == "ftp") {
        proxyType = ProxyTypes::FTP;
        urlPrefix = "http";
        defaultPort = 80;
    } else if (protocol == "socks") {
        proxyType = ProxyTypes::SOCKS;
        urlPrefix = "socks5";
        defaultPort = 1080;
    } else {
        PROXY_LOG_ERROR("proxy protocol is invalid");
        return { "", 0, ProxyTypes::None };
    }

    const std::string gsettingsCmd{ "/usr/bin/gsettings" };
    const std::string gnomeSystemProxySchema{ "org.gnome.system.proxy" };

    std::vector<std::string> hostCmd{gsettingsCmd, "get", gnomeSystemProxySchema + "." + protocol, "host"};
    std::vector<std::string> portCmd{gsettingsCmd, "get", gnomeSystemProxySchema + "." + protocol, "port"};
    CommandOutput hostOutput = m_commandExecutor->ExecuteCommandCaptureOutput(hostCmd[0], hostCmd);
    CommandOutput portOutput = m_commandExecutor->ExecuteCommandCaptureOutput(portCmd[0], portCmd);
    if (0 != hostOutput.exitCode_) {
        PROXY_LOG_ERROR("Error obtaining gnome %s proxy host", protocol);
    }
    if (0 != portOutput.exitCode_) {
        PROXY_LOG_ERROR("Error obtaining gnome %s proxy port", protocol);
    }
    _trim_gsettings_output(hostOutput.output_);
    _trim_gsettings_output(portOutput.output_);


    std::string url = _construct_url(hostOutput.output_, portOutput.output_, urlPrefix);
    uint32_t port = 0;
    if(portOutput.output_ != "") {
        try {
            port = (uint32_t)std::stoi(portOutput.output_);
        } catch (const std::exception& e) {
            PROXY_LOG_WARNING("Port could not be parsed");
        }
    }
    if (port == 0) {
        port = defaultPort;
    } 

    if (!url.empty() && _valid_url(url)) {
        return {url, port, proxyType};
    } 
    return { "", 0, ProxyTypes::None };
}

std::list<ProxyRecord> ProxyDiscoveryEngine::kdeProxy() {
    PROXY_LOG_WARNING("KDE Proxy settings not supported");
    return std::list<ProxyRecord>{};
}

} //proxy