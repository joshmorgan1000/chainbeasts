#pragma once

#include "neuropet/eth_encoding.hpp"
#include "neuropet/match_ledger.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "neuropet/http_client.hpp"

namespace neuropet {

/** Match ledger sending results to a JSON-RPC endpoint. */
class OnchainMatchLedger : public MatchLedger {
  public:
    OnchainMatchLedger(const std::string& host, unsigned short port, const std::string& contract)
        : host_{host}, port_{port}, contract_{contract} {}

    std::uint32_t record_result(std::uint32_t winner, std::uint32_t loser) override {
        std::uint32_t id = next_id_;
        if (send_tx(id, winner, loser)) {
            ++next_id_;
            results_.push_back({id, winner, loser});
            return id;
        }
        return 0;
    }

    /** Send a result transaction asynchronously. */
    std::future<bool> send_tx_async(std::uint32_t id, std::uint32_t winner, std::uint32_t loser) {
        return std::async(std::launch::async,
                          [this, id, winner, loser]() { return send_tx(id, winner, loser); });
    }

  private:
    bool send_tx(std::uint32_t id, std::uint32_t winner, std::uint32_t loser) {
#if defined(_WIN32) || defined(__unix__)
        std::string data = "0x81adbe26";
        data += encode_uint32(id);
        data += encode_uint32(winner);
        data += encode_uint32(loser);

        std::ostringstream body;
        body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",";
        body << "\"params\":[{\"to\":\"" << contract_ << "\",\"data\":\"" << data
             << "\"}],\"id\":1}";
        if (!http_post_json(host_, port_, body.str())) {
            std::cerr << "OnchainMatchLedger: failed to record result" << std::endl;
            return false;
        }
        return true;
#else
        (void)id;
        (void)winner;
        (void)loser;
        return false;
#endif
    }

    std::string host_{};
    unsigned short port_{};
    std::string contract_{};
};

} // namespace neuropet
