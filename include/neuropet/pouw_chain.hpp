#pragma once

#include "neuropet/validator.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace neuropet {

/** Simple in-memory PoUW blockchain that finalizes checkpoints once a
 *  quorum of attestations is reached and rewards the miner. */
class PoUWChain {
  public:
    static constexpr std::uint32_t CORE_PER_ENERGY = 1;

    explicit PoUWChain(std::size_t quorum = 3) : validator_(quorum) {}

    struct Block {
        std::uint32_t index{0};
        std::uint32_t creature_id{0};
        std::uint32_t epoch_id{0};
        std::string root_hash{};
        std::string miner{};
        float loss{0.0f};
        std::uint32_t energy_spent{0};
    };

    void submit_checkpoint(std::uint32_t creature_id, std::uint32_t epoch_id,
                           const std::string& root_hash, const std::string& miner,
                           float loss = 0.0f, std::uint32_t energy_spent = 0) {
        pending_[root_hash] = {creature_id, epoch_id, miner, loss, energy_spent};
    }

    /// Mark a checkpoint as proven by a valid STARK proof.
    void submit_proof(const std::string& root_hash) { proven_.insert(root_hash); }

    void attest(const std::string& root_hash) { validator_.attest(root_hash); }

    bool finalize_checkpoint(const std::string& root_hash) {
        // A checkpoint is only finalized when either a sufficient number of
        // attestations were collected or a valid proof was submitted.
        if (!validator_.has_quorum(root_hash) && !proven_.count(root_hash))
            return false;

        // Locate the pending entry; skip if unknown or already finalized.
        auto it = pending_.find(root_hash);
        if (it == pending_.end() || finalized_.count(root_hash))
            return false;

        // Enforce that epochs are finalized in sequence for each creature.
        auto ce_it = next_epoch_.find(it->second.creature_id);
        std::uint32_t expected = ce_it == next_epoch_.end() ? 0 : ce_it->second;
        if (it->second.epoch_id != expected)
            return false;

        // Reject checkpoints that regress in loss compared to the last
        // finalized epoch. This mirrors the rule used by validators.
        auto loss_it = last_loss_.find(it->second.creature_id);
        float prev_loss = loss_it == last_loss_.end() ? -1.f : loss_it->second;
        if (prev_loss >= 0.f && it->second.loss > prev_loss)
            return false;

        // Create a new block representing the finalized checkpoint.
        Block blk{};
        blk.index = static_cast<std::uint32_t>(chain_.size() + 1);
        blk.creature_id = it->second.creature_id;
        blk.epoch_id = it->second.epoch_id;
        blk.root_hash = root_hash;
        blk.miner = it->second.miner;
        blk.loss = it->second.loss;
        blk.energy_spent = it->second.energy_spent;
        chain_.push_back(blk); // append to the canonical chain
        energy_balance_[blk.miner] += it->second.energy_spent;
        core_balance_[blk.miner] +=
            static_cast<std::uint64_t>(it->second.energy_spent) * CORE_PER_ENERGY;
        finalized_.insert(root_hash);
        proven_.erase(root_hash);
        next_epoch_[blk.creature_id] = blk.epoch_id + 1;
        last_loss_[blk.creature_id] = blk.loss;
        pending_.erase(it);
        return true;
    }

    const std::vector<Block>& chain() const { return chain_; }

    std::uint64_t energy_balance(const std::string& miner) const {
        auto it = energy_balance_.find(miner);
        return it == energy_balance_.end() ? 0 : it->second;
    }

    std::uint64_t core_balance(const std::string& miner) const {
        auto it = core_balance_.find(miner);
        return it == core_balance_.end() ? 0 : it->second;
    }

  private:
    struct Pending {
        std::uint32_t creature_id;
        std::uint32_t epoch_id;
        std::string miner;
        float loss;
        std::uint32_t energy_spent;
    };

    std::vector<Block> chain_{};
    std::unordered_map<std::string, Pending> pending_{};
    std::unordered_set<std::string> finalized_{};
    std::unordered_set<std::string> proven_{};
    std::unordered_map<std::string, std::uint64_t> energy_balance_{};
    std::unordered_map<std::string, std::uint64_t> core_balance_{};
    std::unordered_map<std::uint32_t, std::uint32_t> next_epoch_{};
    std::unordered_map<std::uint32_t, float> last_loss_{};
    Validator validator_{};
};

} // namespace neuropet
