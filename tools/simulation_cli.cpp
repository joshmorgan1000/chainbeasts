#include "neuropet/simulation.hpp"
#include <iostream>

int main(int argc, char** argv) {
    std::size_t creatures = 5;
    std::size_t steps = 5;
    std::size_t battles = 3;
    unsigned seed = 0;
    if (argc > 1)
        creatures = static_cast<std::size_t>(std::stoul(argv[1]));
    if (argc > 2)
        steps = static_cast<std::size_t>(std::stoul(argv[2]));
    if (argc > 3)
        battles = static_cast<std::size_t>(std::stoul(argv[3]));
    if (argc > 4)
        seed = static_cast<unsigned>(std::stoul(argv[4]));

    std::mt19937 rng(seed ? seed : std::random_device{}());
    neuropet::MatchLedger ledger;
    neuropet::run_simulation(creatures, steps, battles, ledger, rng);

    for (const auto& r : ledger.results())
        std::cout << "Battle " << r.battle_id << ": winner=" << r.winner
                  << " loser=" << r.loser << std::endl;
    return 0;
}
