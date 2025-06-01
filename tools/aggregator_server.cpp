#include "neuropet/proof_aggregator.hpp"
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

using namespace neuropet;

namespace {
ProofAggregatorServer* server = nullptr;

void handle_sigint(int) {
    if (server)
        server->stop();
}

void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " [--port N] [--connect host:port ...]" << std::endl;
}
} // namespace

int main(int argc, char** argv) {
    unsigned short port = 0;
    std::vector<std::pair<std::string, unsigned short>> peers;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--port" && i + 1 < argc) {
            port = static_cast<unsigned short>(std::stoi(argv[++i]));
        } else if (arg == "--connect" && i + 1 < argc) {
            std::string hp(argv[++i]);
            auto pos = hp.find(':');
            if (pos == std::string::npos) {
                usage(argv[0]);
                return 1;
            }
            std::string host = hp.substr(0, pos);
            unsigned short p = static_cast<unsigned short>(std::stoi(hp.substr(pos + 1)));
            peers.emplace_back(host, p);
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    ProofAggregatorServer srv(port);
    server = &srv;
    std::signal(SIGINT, handle_sigint);
    srv.start();
    std::cout << "Proof aggregator listening on port " << srv.port() << std::endl;
    for (const auto& peer : peers)
        srv.connect(peer.first, peer.second);

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
