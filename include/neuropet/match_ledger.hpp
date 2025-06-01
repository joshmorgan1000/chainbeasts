#pragma once

#include <cstdint>
#include <vector>

namespace neuropet {

/** Simple in-memory ledger emulating on-chain battle recording. */
struct MatchResult {
    std::uint32_t battle_id;
    std::uint32_t winner;
    std::uint32_t loser;
};

class MatchLedger {
  public:
    virtual ~MatchLedger() = default;

    /** Record a match result and return the battle id. */
    virtual std::uint32_t record_result(std::uint32_t winner, std::uint32_t loser) {
        MatchResult r{next_id_++, winner, loser};
        results_.push_back(r);
        return r.battle_id;
    }

    /** Return all recorded results. */
    const std::vector<MatchResult>& results() const { return results_; }

  protected:
    std::uint32_t next_id_{1};
    std::vector<MatchResult> results_{};
};

} // namespace neuropet
