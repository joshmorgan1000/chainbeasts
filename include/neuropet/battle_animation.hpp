#pragma once

#include <cstdint>
#include <vector>

#include "neuropet/battle.hpp"

namespace neuropet {

/** Frame of a deterministic battle animation. */
struct BattleFrame {
    bool attacker_is_a{true};
    int hp_a{0};
    int hp_b{0};
    int stamina_a{0};
    int stamina_b{0};
};

/**
 * Records the sequence of actions during a battle so the outcome can
 * be deterministically replayed.
 */
class BattleAnimator {
  public:
    /** Run the battle simulation and record frames. */
    void run(const CreatureStats& a, const CreatureStats& b, std::uint32_t seed = 0) {
        frames_.clear();
        std::uint32_t state = seed ? seed : (a.id ^ b.id);
        auto rng = [&state]() {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return state;
        };
        int hpA = a.total_defense();
        int hpB = b.total_defense();
        int staA = a.total_stamina();
        int staB = b.total_stamina();
        bool turn = rng() & 1u;
        while (hpA > 0 && hpB > 0 && (staA > 0 || staB > 0)) {
            if (turn && staA > 0) {
                hpB -= a.total_power();
                --staA;
                frames_.push_back({true, hpA, hpB, staA, staB});
            } else if (!turn && staB > 0) {
                hpA -= b.total_power();
                --staB;
                frames_.push_back({false, hpA, hpB, staA, staB});
            }
            turn = rng() & 1u;
        }
        winner_ = (hpA >= hpB) ? a.id : b.id;
    }

    /** Return the recorded animation frames. */
    const std::vector<BattleFrame>& frames() const { return frames_; }

    /** ID of the battle winner after running. */
    std::uint32_t winner() const { return winner_; }

    /**
     * Replay a recorded animation verifying each frame and return the
     * resulting winner. This can be used by validators to deterministically
     * check battle results without running the RNG again.
     * @param a First creature
     * @param b Second creature
     * @param frames Sequence produced by `run`
     * @return Winner id or 0 if the frames are inconsistent
     */
    static std::uint32_t replay(const CreatureStats& a, const CreatureStats& b,
                                const std::vector<BattleFrame>& frames) {
        int hpA = a.total_defense();
        int hpB = b.total_defense();
        int staA = a.total_stamina();
        int staB = b.total_stamina();
        for (const auto& f : frames) {
            if (f.attacker_is_a) {
                if (staA <= 0)
                    return 0;
                hpB -= a.total_power();
                --staA;
            } else {
                if (staB <= 0)
                    return 0;
                hpA -= b.total_power();
                --staB;
            }
            if (f.hp_a != hpA || f.hp_b != hpB || f.stamina_a != staA || f.stamina_b != staB)
                return 0;
            if (hpA <= 0 || hpB <= 0 || (staA <= 0 && staB <= 0))
                break;
        }
        return (hpA >= hpB) ? a.id : b.id;
    }

  private:
    std::vector<BattleFrame> frames_{};
    std::uint32_t winner_{0};
};

} // namespace neuropet
