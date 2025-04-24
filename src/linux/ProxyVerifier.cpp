/**
 * @file
 *
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include "ProxyVerifier.hpp"
#include "ProxyLoggerDef.hpp"
#include <unistd.h>
#include <curl/curl.h>

namespace proxy {

static curl_proxytype _detectProxyType(const std::string& proxyUrl) {
    if (proxyUrl.rfind("https://", 0) == 0) {
        return CURLPROXY_HTTPS;
    } else if (proxyUrl.rfind("http://", 0) == 0) {
        return CURLPROXY_HTTP;
    } else if (proxyUrl.rfind("socks5://", 0) == 0) {
        return CURLPROXY_SOCKS5;
    } else if (proxyUrl.rfind("socks4://", 0) == 0) {
        return CURLPROXY_SOCKS4;
    }
    return CURLPROXY_HTTP;
}

static std::string getCABundlePath() {
    // different paths for rhel/debian
    const std::string paths[] {
        "/etc/pki/tls/certs/ca-bundle.crt",
        "/etc/ssl/certs/ca-certificates.crt"
    };
    for (const std::string path : paths) {
        if(access(path.c_str(), R_OK) == 0) {
            return path;
        }
    }
    return "";
}

bool ProxyVerifier::verifyProxy(const std::string &testUrl, const ProxyRecord &proxyRecord)
{
    CURL *curl;
    CURLcode res;
    bool ret = false;
    /* get a curl handle */
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxyRecord.url.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, _detectProxyType(proxyRecord.url));
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxyRecord.port);
        curl_easy_setopt(curl, CURLOPT_URL, testUrl.c_str());
        std::string caPath = getCABundlePath();
        if (!caPath.empty()) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, caPath.c_str());
        }

        /* Perform the request, res gets the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK) {
            PROXY_LOG_ERROR("proxy %s failed verification: %s\n", proxyRecord.url.c_str(), curl_easy_strerror(res));
        } else {
            PROXY_LOG_INFO("proxy %s passed verification\n", proxyRecord.url.c_str());
            ret = true;
        }
    
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return ret;
}

} //proxy