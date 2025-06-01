#pragma once

#include "harmonics/net_utils.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace neuropet {

inline bool http_post_json(const std::string& host, unsigned short port, const std::string& body,
                           std::string* response = nullptr, int attempts = 3,
                           int backoff_ms = 100) {
#if defined(_WIN32) || defined(__unix__)
    harmonics::net_init();
    int delay = backoff_ms;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        harmonics::socket_t fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd == harmonics::invalid_socket) {
            std::cerr << "socket() failed" << std::endl;
            continue;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
            std::cerr << "invalid host " << host << std::endl;
            harmonics::net_close(fd);
            return false;
        }
        addr.sin_port = htons(port);
        if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            std::cerr << "connection attempt " << attempt + 1 << " failed" << std::endl;
            harmonics::net_close(fd);
            if (attempt + 1 < attempts)
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            delay *= 2;
            continue;
        }
        std::ostringstream req;
        req << "POST / HTTP/1.1\r\n";
        req << "Host: " << host << ":" << port << "\r\n";
        req << "Content-Type: application/json\r\n";
        req << "Content-Length: " << body.size() << "\r\n";
        req << "Connection: close\r\n\r\n";
        req << body;
        std::string r = req.str();
        auto send_all = [&](const char* p, std::size_t n) -> bool {
            while (n > 0) {
                auto s = harmonics::net_write(fd, p, static_cast<int>(n));
                if (s <= 0)
                    return false;
                p += s;
                n -= static_cast<std::size_t>(s);
            }
            return true;
        };
        if (!send_all(r.c_str(), r.size())) {
            std::cerr << "send failed on attempt " << attempt + 1 << std::endl;
            harmonics::net_close(fd);
            if (attempt + 1 < attempts)
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            delay *= 2;
            continue;
        }
        std::string tmp;
        if (response)
            response->clear();
        char buf[256];
        int n;
        while ((n = harmonics::net_read(fd, buf, sizeof(buf))) > 0) {
            tmp.append(buf, n);
            if (response)
                response->append(buf, n);
        }
        harmonics::net_close(fd);
        auto& chk = response ? *response : tmp;
        auto pos = chk.find("HTTP/1.1");
        if (pos != std::string::npos && chk.size() >= pos + 12) {
            int code = std::atoi(chk.c_str() + pos + 9);
            if (code < 200 || code >= 300) {
                std::cerr << "HTTP error " << code << " on attempt " << attempt + 1 << std::endl;
                if (attempt + 1 < attempts)
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                delay *= 2;
                continue;
            }
        }
        return true;
    }
    std::cerr << "http_post_json failed after " << attempts << " attempts" << std::endl;
    return false;
#else
    (void)host;
    (void)port;
    (void)body;
    (void)response;
    (void)attempts;
    return false;
#endif
}

} // namespace neuropet
