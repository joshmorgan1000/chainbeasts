#include "neuropet/eth_encoding.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef __unix__
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace neuropet;

static std::string encode_address(const std::string& addr) {
    std::string h = strip_0x(addr);
    return std::string(64 - h.size(), '0') + h;
}

static void send_tx(const std::string& host, unsigned short port, const std::string& contract,
                    const std::string& data) {
#ifdef __unix__
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return;
    }
    addr.sin_port = htons(port);
    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return;
    }
    std::ostringstream body;
    body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",";
    body << "\"params\":[{\"to\":\"" << contract << "\",\"data\":\"" << data
         << "\"}],\"id\":1}";
    std::string b = body.str();
    std::ostringstream req;
    req << "POST / HTTP/1.1\r\n";
    req << "Host: " << host << ":" << port << "\r\n";
    req << "Content-Type: application/json\r\n";
    req << "Content-Length: " << b.size() << "\r\n";
    req << "Connection: close\r\n\r\n";
    req << b;
    std::string r = req.str();
    ::send(fd, r.c_str(), r.size(), 0);
    char buf[256];
    while (::recv(fd, buf, sizeof(buf), 0) > 0) {
    }
    ::close(fd);
#else
    (void)host;
    (void)port;
    (void)contract;
    (void)data;
#endif
}

static std::string read_hex(const std::string& file) {
    std::ifstream in(file, std::ios::binary);
    std::ostringstream out;
    unsigned char c;
    while (in.read(reinterpret_cast<char*>(&c), 1)) {
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02x", c);
        out << buf;
    }
    return out.str();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <out|in> ..." << std::endl;
        return 1;
    }
    std::string cmd = argv[1];
    if (cmd == "out") {
        if (argc != 6) {
            std::cout << "Usage: " << argv[0] << " out host:port contract tokenId dstChainId" << std::endl;
            return 1;
        }
        std::string hostport = argv[2];
        auto pos = hostport.find(':');
        std::string host = hostport.substr(0, pos);
        unsigned short port = static_cast<unsigned short>(std::stoi(hostport.substr(pos + 1)));
        std::string contract = argv[3];
        std::uint32_t token = static_cast<std::uint32_t>(std::stoul(argv[4]));
        std::uint32_t dst = static_cast<std::uint32_t>(std::stoul(argv[5]));
        std::string data = "0x3dc83f6e";
        data += encode_uint256(token);
        data += encode_uint256(dst);
        send_tx(host, port, contract, data);
    } else if (cmd == "in") {
        if (argc != 9) {
            std::cout << "Usage: " << argv[0]
                      << " in host:port contract tokenId to srcChainId weights.bin dnaHex" << std::endl;
            return 1;
        }
        std::string hostport = argv[2];
        auto pos = hostport.find(':');
        std::string host = hostport.substr(0, pos);
        unsigned short port = static_cast<unsigned short>(std::stoi(hostport.substr(pos + 1)));
        std::string contract = argv[3];
        std::uint32_t token = static_cast<std::uint32_t>(std::stoul(argv[4]));
        std::string to = argv[5];
        std::uint32_t src = static_cast<std::uint32_t>(std::stoul(argv[6]));
        std::string weights_hex = read_hex(argv[7]);
        std::string dna_hex = argv[8];
        std::string data = "0x4e463f9c";
        data += encode_uint256(token);
        data += encode_address(to);
        data += encode_uint256(src);
        data += encode_bytes(weights_hex);
        data += encode_bytes32(dna_hex);
        send_tx(host, port, contract, data);
    } else {
        std::cout << "Unknown command: " << cmd << std::endl;
        return 1;
    }
    return 0;
}
