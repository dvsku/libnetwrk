#pragma once

#include <memory>
#include <string>
#include <regex>

namespace libnetwrk {
    class tcp_resolver {
    public:
        // Context type for this resolver
        using context_t = asio::io_context;

        // Endpoint type for this resolver
        using endpoint_t = asio::ip::tcp::endpoint;
    
    private:
        using native_resolver_t = asio::ip::tcp::resolver;
    
    public:
        tcp_resolver()                    = delete;
        tcp_resolver(const tcp_resolver&) = delete;
        tcp_resolver(tcp_resolver&&)      = delete;

        tcp_resolver(context_t& context)
            : m_context(context)
        {
            m_native = std::make_unique<native_resolver_t>(context);
        }

        tcp_resolver& operator=(const tcp_resolver&) = delete;
        tcp_resolver& operator=(tcp_resolver&&)      = delete;

    public:
        bool get_endpoint(const std::string& host, uint16_t port, endpoint_t& out_endpoint) {
            if (!m_native) return false;

            static const std::regex pattern("([a-zA-Z0-9]+):\\/\\/([a-zA-Z0-9.-]+)\\/*");

            std::string l_host = "";
            std::smatch matches;

            if (std::regex_match(host, matches, pattern)) {
                if (matches.size() != 3) return false;

                asio::ip::tcp::resolver::query        query(matches[2], matches[1]);
                asio::ip::tcp::resolver::results_type endpoints = m_native->resolve(query);

                for (const auto& endpoint : endpoints) {
                    if (endpoint.endpoint().protocol() == asio::ip::tcp::v4()) {
                        l_host = endpoint.endpoint().address().to_string();
                        break;
                    }
                }
            }
            else if (host == "localhost") {
                l_host = "127.0.0.1";
            }
            else {
                l_host = host;
            }

            if (l_host == "")
                return false;

            try {
                out_endpoint = endpoint_t(asio::ip::address::from_string(l_host), port);
            }
            catch (...) {
                return false;
            }
            
            return true;
        }

    private:
        context_t&                         m_context;
        std::unique_ptr<native_resolver_t> m_native = nullptr;
    };
}
