#pragma once
#include <string>
#include <array>

struct Url {
    std::string host;
    std::string pathAndQuery;

    Url() = default;
    Url(std::string h, std::string pq) : host(std::move(h)), pathAndQuery(std::move(pq)) {}

    static Url ParseUrl(const std::string& url) {
        std::string host;
        std::string pathAndQuery;

        if (url.empty()) {
            return Url{host, pathAndQuery};
        }

        const std::string proto_sep = "://";
        auto proto_end = url.find(proto_sep);

        if (proto_end != std::string::npos) {
            // protocol
            host.append(url, 0, proto_end);
            host.append("://");

            const std::string remainder = url.substr(proto_end + proto_sep.size());
            auto path_start = remainder.find('/');

            if (path_start != std::string::npos) {
                // address/domain + port
                host.append(remainder, 0, path_start);
                // path + query
                pathAndQuery.append(remainder.substr(path_start));
            } else {
                host.append(remainder);
            }
        } else {
            // No protocol wow
            auto path_start = url.find('/');

            if (path_start != std::string::npos) {
                host.append(url, 0, path_start);
                pathAndQuery.append(url.substr(path_start));
            } else {
                host.append(url);
            }
        }

        return Url{host, pathAndQuery};
    }

    static bool ShouldRedirect(const std::string& host) {
        std::string stripped = host;
        // host
        if (stripped.rfind("http://", 0) == 0) {
            stripped = stripped.substr(7);
        } else if (stripped.rfind("https://", 0) == 0) {
            stripped = stripped.substr(8);
        }
        // port
        auto colon = stripped.find(':');
        if (colon != std::string::npos) {
            stripped = stripped.substr(0, colon);
        }

        static constexpr const char* EPIC_DOMAINS[] = {
            "game-social.epicgames.com",
            "ol.epicgames.com",
            "ol.epicgames.net",
            "on.epicgames.com",
            "ak.epicgames.com",
            "epicgames.dev",
        };

        for (const auto& domain : EPIC_DOMAINS) {
            if (stripped.size() >= std::strlen(domain) &&
                stripped.compare(
                    stripped.size() - std::strlen(domain),
                    std::strlen(domain),
                    domain
                ) == 0) {
                return true;
            }
        }

        return false;
    }

    static std::string CreateUrl(const std::string& host, const std::string& pathAndQuery) {
        std::string url;
        url.reserve(host.size() + pathAndQuery.size());
        url.append(host);
        url.append(pathAndQuery);
        return url;
    }
};
