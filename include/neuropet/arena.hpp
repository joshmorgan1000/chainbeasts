#pragma once

#include "neuropet/battle.hpp"
#include "neuropet/match_ledger.hpp"

namespace neuropet {

/** Run individual battles and record results to a ledger. */
class Arena {
  public:
    explicit Arena(const BattleEngine& engine = BattleEngine(), MatchLedger* ledger = nullptr)
        : engine_(engine), ledger_(ledger) {}

    void set_ledger(MatchLedger* ledger) { ledger_ = ledger; }

    std::uint32_t battle(const CreatureStats& a, const CreatureStats& b, std::uint32_t seed = 0) {
        std::uint32_t winner = engine_.fight(a, b, seed);
        if (ledger_) {
            std::uint32_t loser = winner == a.id ? b.id : a.id;
            ledger_->record_result(winner, loser);
        }
        return winner;
    }

  private:
    BattleEngine engine_{};
    MatchLedger* ledger_{};
};

} // namespace neuropet
