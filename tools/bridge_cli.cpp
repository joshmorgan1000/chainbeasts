#include "neuropet/eth_encoding.hpp"
#include "neuropet/onchain_verifier.hpp"
#include "neuropet/proof_system.hpp" // for to_hex and keccak256_digest
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

static std::vector<std::uint8_t> hex_to_bytes(const std::string& hex) {
    std::string h = strip_0x(hex);
    if (h.size() % 2)
        h = "0" + h;
    std::vector<std::uint8_t> out;
    out.reserve(h.size() / 2);
    for (std::size_t i = 0; i < h.size(); i += 2) {
        out.push_back(static_cast<std::uint8_t>(std::stoi(h.substr(i, 2), nullptr, 16)));
    }
    return out;
}

static void append_uint256(std::vector<std::uint8_t>& out, std::uint64_t v) {
    for (int i = 31; i >= 0; --i)
        out.push_back(static_cast<std::uint8_t>(v >> (i * 8)));
}

static void append_address(std::vector<std::uint8_t>& out, const std::string& addr) {
    std::string h = strip_0x(addr);
    if (h.size() < 40)
        h = std::string(40 - h.size(), '0') + h;
    for (std::size_t i = 0; i < h.size(); i += 2)
        out.push_back(static_cast<std::uint8_t>(std::stoi(h.substr(i, 2), nullptr, 16)));
}

static std::vector<std::uint8_t> read_bytes(const std::string& file) {
    std::ifstream in(file, std::ios::binary);
    return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(in)),
                                     std::istreambuf_iterator<char>());
}

static std::string compute_root_hash(std::uint32_t token,
                                     const std::string& to,
                                     std::uint32_t src_chain,
                                     const std::vector<std::uint8_t>& weights,
                                     const std::string& dna_hex) {
    std::vector<std::uint8_t> buf;
    append_uint256(buf, token);
    append_address(buf, to);
    append_uint256(buf, src_chain);
    buf.insert(buf.end(), weights.begin(), weights.end());
    auto dna_bytes = hex_to_bytes(dna_hex);
    if (dna_bytes.size() < 32)
        dna_bytes.insert(dna_bytes.begin(), 32 - dna_bytes.size(), 0);
    buf.insert(buf.end(), dna_bytes.begin(), dna_bytes.end());
    return keccak256_digest(buf.data(), buf.size());
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
        if (argc != 11) {
            std::cout << "Usage: " << argv[0]
                      << " in host:port bridgeContract verifierContract tokenId to srcChainId dstChainId weights.bin dnaHex"
                      << std::endl;
            return 1;
        }
        std::string hostport = argv[2];
        auto pos = hostport.find(':');
        std::string host = hostport.substr(0, pos);
        unsigned short port = static_cast<unsigned short>(std::stoi(hostport.substr(pos + 1)));
        std::string contract = argv[3];
        std::string verifier_addr = argv[4];
        std::uint32_t token = static_cast<std::uint32_t>(std::stoul(argv[5]));
        std::string to = argv[6];
        std::uint32_t src = static_cast<std::uint32_t>(std::stoul(argv[7]));
        std::uint32_t dst = static_cast<std::uint32_t>(std::stoul(argv[8]));
        std::vector<std::uint8_t> weights_bytes = read_bytes(argv[9]);
        std::string weights_hex = to_hex(weights_bytes.data(), weights_bytes.size());
        std::string dna_hex = argv[10];

        std::string root = compute_root_hash(token, to, src, weights_bytes, dna_hex);
        OnchainProofVerifier verifier(host, port, verifier_addr);
        // Wait for the STARK proof to be finalised on-chain. Retry a few times
        // so transient delays do not cause an unnecessary failure.
        bool verified = false;
        for (int i = 0; i < 5 && !verified; ++i) {
            if (verifier.verify(root)) {
                verified = true;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        if (!verified) {
            std::cerr << "Proof not finalised on-chain" << std::endl;
            return 1;
        }
        std::string data = "0x4b42d211";
        data += encode_uint256(token);
        data += encode_address(to);
        data += encode_uint256(src);
        data += encode_bytes(weights_hex);
        data += encode_bytes32(dna_hex);
        std::string proof;
        proof += encode_uint256(token);
        proof += encode_address(to);
        proof += encode_bytes(weights_hex);
        proof += encode_bytes32(dna_hex);
        data += encode_bytes(proof);
        send_tx(host, port, contract, data);
    } else {
        std::cout << "Unknown command: " << cmd << std::endl;
        return 1;
    }
    return 0;
}
