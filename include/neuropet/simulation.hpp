#pragma once

#include <random>
#include <vector>

#include "neuropet/battle.hpp"
#include "neuropet/match_ledger.hpp"
#include "neuropet/training.hpp"

namespace neuropet {

/** No-op metrics streamer used for offline simulation. */
struct NullStreamer {
    void push(const TrainingMetrics&) {}
};

/** Generate a random creature with power, defense and stamina in [1, 10]. */
inline CreatureStats random_creature(std::uint32_t id, std::mt19937& rng) {
    std::uniform_int_distribution<int> stat(1, 10);
    CreatureStats c{id, stat(rng), stat(rng), stat(rng)};
    return c;
}

/**
 * Run offline training and battles using random creatures.
 * @param creature_count Number of creatures to spawn
 * @param steps Training steps per creature
 * @param battles Number of battles to run
 * @param ledger Ledger recording battle results
 * @param rng Random number generator
 */
inline void run_simulation(std::size_t creature_count, std::size_t steps, std::size_t battles,
                           MatchLedger& ledger, std::mt19937& rng) {
    std::vector<CreatureStats> creatures;
    creatures.reserve(creature_count);
    for (std::size_t i = 0; i < creature_count; ++i)
        creatures.push_back(random_creature(static_cast<std::uint32_t>(i + 1), rng));

    for (std::size_t i = 0; i < creature_count; ++i) {
        NullStreamer ns;
        train_with_metrics(steps, ns);
    }

    Matchmaker mm(BattleEngine{}, &ledger);
    std::uniform_int_distribution<std::size_t> dist(0, creature_count - 1);
    for (std::size_t i = 0; i < battles; ++i) {
        std::size_t a = dist(rng);
        std::size_t b = dist(rng);
        while (a == b)
            b = dist(rng);
        mm.enqueue(creatures[a]);
        mm.enqueue(creatures[b]);
        mm.try_match(static_cast<std::uint32_t>(rng()));
    }
}

} // namespace neuropet
