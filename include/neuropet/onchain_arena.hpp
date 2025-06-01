#pragma once

#include "neuropet/arena.hpp"
#include "neuropet/eth_encoding.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "neuropet/http_client.hpp"

namespace neuropet {

/** Arena implementation sending battle requests to an on-chain matchmaker. */
class OnchainArena : public Arena {
  public:
    OnchainArena(const std::string& host, unsigned short port, const std::string& contract,
                 MatchLedger* ledger = nullptr)
        : Arena(BattleEngine(), ledger), host_{host}, port_{port}, contract_{contract} {}

    std::uint32_t battle(const CreatureStats& a, const CreatureStats& b, std::uint32_t seed = 0) {
        std::uint32_t winner = Arena::battle(a, b, seed);
        send_tx(a, b, seed);
        return winner;
    }

    /** Send a battle transaction asynchronously. */
    std::future<bool> send_tx_async(const CreatureStats& a, const CreatureStats& b,
                                    std::uint32_t seed) {
        return std::async(std::launch::async, [this, a, b, seed]() { return send_tx(a, b, seed); });
    }

  private:
    // Serialize the battle request and send it to the configured RPC endpoint.
    // The transaction encodes all creature stats using Ethereum ABI rules.
    bool send_tx(const CreatureStats& a, const CreatureStats& b, std::uint32_t seed) {
#if defined(_WIN32) || defined(__unix__)
        std::string data = "0x92a4f9e6";
        data += encode_uint32(a.id);
        data += encode_int32(a.power);
        data += encode_int32(a.defense);
        data += encode_int32(a.stamina);
        data += encode_uint32(b.id);
        data += encode_int32(b.power);
        data += encode_int32(b.defense);
        data += encode_int32(b.stamina);
        data += encode_uint32(seed);

        std::ostringstream body;
        body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",";
        body << "\"params\":[{\"to\":\"" << contract_ << "\",\"data\":\"" << data
             << "\"}],\"id\":1}";
        if (!http_post_json(host_, port_, body.str())) {
            std::cerr << "OnchainArena: failed to send battle transaction" << std::endl;
            return false;
        }
        return true;
#else
        (void)a;
        (void)b;
        (void)seed;
        return false;
#endif
    }

    std::string host_{};
    unsigned short port_{};
    std::string contract_{};
};

} // namespace neuropet
