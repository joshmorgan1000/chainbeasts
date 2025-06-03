#pragma once

#include "neuropet/proof_system.hpp"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#ifdef __unix__
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace neuropet {

/** In-memory collection of STARK proofs identified by root hash. */
class ProofAggregator {
  public:
    /// Store or update a proof. Existing entries are overwritten.
    void submit(const StarkProof& p) {
        std::lock_guard<std::mutex> lock(mutex_);
        proofs_[p.root] = {p.proof, p.loss};
    }

    /// Return all known proofs.
    std::vector<StarkProof> all() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<StarkProof> out;
        out.reserve(proofs_.size());
        for (const auto& kv : proofs_)
            out.push_back({kv.first, kv.second.first, kv.second.second});
        return out;
    }

  private:
    mutable std::mutex mutex_{};
    std::unordered_map<std::string, std::pair<std::string, float>> proofs_{};
};

#ifdef __unix__

/// Network statistics for profiling throughput.
struct AggregatorNetStats {
    std::atomic<std::uint64_t> bytes_sent{0};
    std::atomic<std::uint64_t> bytes_received{0};
    std::atomic<std::uint64_t> proofs_sent{0};
    std::atomic<std::uint64_t> proofs_received{0};
};

/// Simple TCP server that distributes proofs to connected peers.
class ProofAggregatorServer {
  public:
    explicit ProofAggregatorServer(unsigned short port = 0) : port_hint_{port} {}
    ~ProofAggregatorServer() { stop(); }

    unsigned short port() const { return port_; }

    /// Inspect current network statistics.
    const AggregatorNetStats& stats() const { return stats_; }

    /// Set a callback invoked whenever a proof is received.
    void set_callback(std::function<void(const StarkProof&)> cb) { on_proof_ = std::move(cb); }

    /// Connect to another aggregator peer.
    bool connect(const std::string& host, unsigned short port) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            return false;
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
            ::close(fd);
            return false;
        }
        addr.sin_port = htons(port);
        if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(fd);
            return false;
        }
        set_nodelay(fd);
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.push_back(fd);
        }
        std::thread(&ProofAggregatorServer::handle_client, this, fd).detach();
        for (const auto& p : agg_.all())
            send_proof(fd, p, &stats_);
        return true;
    }

    /// Start listening for connections.
    void start() {
        stop();
        running_ = true;
        server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            running_ = false;
            return;
        }
        set_nodelay(server_fd_);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port_hint_);
        if (::bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(server_fd_);
            running_ = false;
            return;
        }
        if (::listen(server_fd_, 4) < 0) {
            ::close(server_fd_);
            running_ = false;
            return;
        }
        socklen_t len = sizeof(addr);
        ::getsockname(server_fd_, reinterpret_cast<sockaddr*>(&addr), &len);
        port_ = ntohs(addr.sin_port);
        accept_thread_ = std::thread(&ProofAggregatorServer::accept_loop, this);
    }

    /// Stop the server and close all connections.
    void stop() {
        if (!running_)
            return;
        running_ = false;
        if (server_fd_ >= 0) {
            ::shutdown(server_fd_, SHUT_RDWR);
            ::close(server_fd_);
            server_fd_ = -1;
        }
        if (accept_thread_.joinable())
            accept_thread_.join();
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (int c : clients_)
            ::close(c);
        clients_.clear();
    }

    /// Submit a proof from the local process.
    void submit(const StarkProof& p) {
        agg_.submit(p);
        broadcast(p);
        if (on_proof_)
            on_proof_(p);
    }

  private:
    static bool send_string(int fd, const std::string& s) {
        std::uint32_t len = static_cast<std::uint32_t>(s.size());
        if (::send(fd, &len, sizeof(len), 0) != sizeof(len))
            return false;
        std::size_t off = 0;
        while (off < s.size()) {
            auto n = ::send(fd, s.data() + off, s.size() - off, 0);
            if (n <= 0)
                return false;
            off += static_cast<std::size_t>(n);
        }
        return true;
    }

    static bool recv_string(int fd, std::string& out) {
        std::uint32_t len = 0;
        if (::recv(fd, &len, sizeof(len), MSG_WAITALL) != sizeof(len))
            return false;
        out.resize(len);
        std::size_t off = 0;
        while (off < len) {
            auto n = ::recv(fd, out.data() + off, len - off, 0);
            if (n <= 0)
                return false;
            off += static_cast<std::size_t>(n);
        }
        return true;
    }

    static bool send_float(int fd, float f) {
        std::uint32_t bits;
        static_assert(sizeof(float) == sizeof(bits), "float size");
        std::memcpy(&bits, &f, sizeof(bits));
        bits = htonl(bits);
        return ::send(fd, &bits, sizeof(bits), 0) == sizeof(bits);
    }

    static bool recv_float(int fd, float& f) {
        std::uint32_t bits = 0;
        if (::recv(fd, &bits, sizeof(bits), MSG_WAITALL) != sizeof(bits))
            return false;
        bits = ntohl(bits);
        std::memcpy(&f, &bits, sizeof(bits));
        return true;
    }

    static void set_nodelay(int fd) {
        int flag = 1;
        ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    }

    static bool send_proof(int fd, const StarkProof& p, AggregatorNetStats* s = nullptr) {
        if (!send_string(fd, p.root) || !send_string(fd, p.proof) || !send_float(fd, p.loss))
            return false;
        if (s) {
            s->bytes_sent += 4 + p.root.size();
            s->bytes_sent += 4 + p.proof.size();
            s->bytes_sent += 4;
            ++s->proofs_sent;
        }
        return true;
    }

    static bool recv_proof(int fd, StarkProof& p, AggregatorNetStats* s = nullptr) {
        if (!recv_string(fd, p.root) || !recv_string(fd, p.proof) || !recv_float(fd, p.loss))
            return false;
        if (s) {
            s->bytes_received += 4 + p.root.size();
            s->bytes_received += 4 + p.proof.size();
            s->bytes_received += 4;
            ++s->proofs_received;
        }
        return true;
    }

    void accept_loop() {
        while (running_) {
            int c = ::accept(server_fd_, nullptr, nullptr);
            if (c < 0) {
                if (running_)
                    continue;
                else
                    break;
            }
            set_nodelay(c);
            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                clients_.push_back(c);
            }
            std::thread(&ProofAggregatorServer::handle_client, this, c).detach();
        }
    }

    void handle_client(int fd) {
        StarkProof p{};
        while (running_) {
            if (!recv_proof(fd, p, &stats_))
                break;
            agg_.submit(p);
            broadcast(p);
            if (on_proof_)
                on_proof_(p);
        }
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.erase(std::remove(clients_.begin(), clients_.end(), fd), clients_.end());
        }
        ::close(fd);
    }

    void broadcast(const StarkProof& p) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto it = clients_.begin(); it != clients_.end();) {
            if (!send_proof(*it, p, &stats_)) {
                ::close(*it);
                it = clients_.erase(it);
                continue;
            }
            ++it;
        }
    }

    /// Return all proofs currently stored.
    std::vector<StarkProof> all() const { return agg_.all(); }

    unsigned short port_hint_{0};
    int server_fd_{-1};
    unsigned short port_{0};
    ProofAggregator agg_{};
    std::thread accept_thread_{};
    std::vector<int> clients_{};
    std::mutex clients_mutex_{};
    std::atomic<bool> running_{false};
    std::function<void(const StarkProof&)> on_proof_{};
    AggregatorNetStats stats_{};
};
#endif // __unix__

} // namespace neuropet
