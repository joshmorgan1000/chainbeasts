#pragma once

#include "neuropet/eth_encoding.hpp"
#include <future>
#include <sstream>
#include <string>

#ifdef __unix__
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace neuropet {

/** Client helper submitting checkpoints to the on-chain TrainingLedger. */
class OnchainTrainingLedger {
  public:
    OnchainTrainingLedger(const std::string& host, unsigned short port, const std::string& contract)
        : host_{host}, port_{port}, contract_{contract} {}

    bool submit_checkpoint(std::uint32_t creature_id, std::uint32_t epoch_id,
                           const std::string& root_hash, std::uint32_t model_bytes,
                           std::uint32_t loss, bool loss_drop, std::uint32_t energy_spent,
                           const std::string& rule_hash) const {
        return send_tx(creature_id, epoch_id, root_hash, model_bytes, loss, loss_drop, energy_spent,
                       rule_hash);
    }

    std::future<bool> submit_checkpoint_async(std::uint32_t creature_id, std::uint32_t epoch_id,
                                              const std::string& root_hash,
                                              std::uint32_t model_bytes, std::uint32_t loss,
                                              bool loss_drop, std::uint32_t energy_spent,
                                              const std::string& rule_hash) const {
        return std::async(std::launch::async, [this, creature_id, epoch_id, root_hash, model_bytes,
                                               loss, loss_drop, energy_spent, rule_hash]() {
            return send_tx(creature_id, epoch_id, root_hash, model_bytes, loss, loss_drop,
                           energy_spent, rule_hash);
        });
    }

    /** Finalize a checkpoint once a valid proof is available. */
    void finalize_checkpoint(const std::string& root_hash) const { send_finalize(root_hash); }

    /** Finalize a checkpoint asynchronously. */
    std::future<void> finalize_checkpoint_async(const std::string& root_hash) const {
        return std::async(std::launch::async, [this, root_hash]() { send_finalize(root_hash); });
    }

    /** Mint a reward to a miner address. */
    void reward_miner(const std::string& miner, std::uint32_t amount) const {
        send_reward(miner, amount);
    }

    /** Mint a reward asynchronously. */
    std::future<void> reward_miner_async(const std::string& miner, std::uint32_t amount) const {
        return std::async(std::launch::async,
                          [this, miner, amount]() { send_reward(miner, amount); });
    }

  private:
    bool send_tx(std::uint32_t creature_id, std::uint32_t epoch_id, const std::string& root_hash,
                 std::uint32_t model_bytes, std::uint32_t loss, bool loss_drop,
                 std::uint32_t energy_spent, const std::string& rule_hash) const {
#if defined(_WIN32) || defined(__unix__)
        std::string data = "0x5df4059d";
        data += encode_uint32(creature_id);
        data += encode_uint32(epoch_id);
        data += encode_bytes32(root_hash);
        data += encode_uint32(model_bytes);
        data += encode_uint32(loss);
        data += encode_bool(loss_drop);
        data += encode_uint32(energy_spent);
        data += encode_bytes32(rule_hash);

        std::ostringstream body;
        body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",";
        body << "\"params\":[{\"to\":\"" << contract_ << "\",\"data\":\"" << data
             << "\"}],\"id\":1}";
        if (!http_post_json(host_, port_, body.str())) {
            std::cerr << "OnchainTrainingLedger: failed to submit checkpoint" << std::endl;
            return false;
        }
        return true;
#else
        (void)creature_id;
        (void)epoch_id;
        (void)root_hash;
        (void)model_bytes;
        (void)loss;
        (void)loss_drop;
        (void)energy_spent;
        (void)rule_hash;
        return false;
#endif
    }

    void send_finalize(const std::string& root_hash) const {
#if defined(_WIN32) || defined(__unix__)
        std::string data = "0x70d13b75";
        data += encode_bytes32(root_hash);

        std::ostringstream body;
        body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",";
        body << "\"params\":[{\"to\":\"" << contract_ << "\",\"data\":\"" << data
             << "\"}],\"id\":1}";
        if (!http_post_json(host_, port_, body.str()))
            std::cerr << "OnchainTrainingLedger: finalize transaction failed" << std::endl;
#else
        (void)root_hash;
#endif
    }

    void send_reward(const std::string& miner, std::uint32_t amount) const {
#if defined(_WIN32) || defined(__unix__)
        std::string data = "0xdbe12956";
        data += encode_bytes32(miner);
        data += encode_uint256(amount);

        std::ostringstream body;
        body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",";
        body << "\"params\":[{\"to\":\"" << contract_ << "\",\"data\":\"" << data
             << "\"}],\"id\":1}";
        if (!http_post_json(host_, port_, body.str()))
            std::cerr << "OnchainTrainingLedger: reward transaction failed" << std::endl;
#else
        (void)miner;
        (void)amount;
#endif
    }

    std::string host_{};
    unsigned short port_{};
    std::string contract_{};
};

} // namespace neuropet
