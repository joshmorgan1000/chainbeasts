#include "neuropet/onchain_arena.hpp"

int main() {
    neuropet::CreatureStats a{1, 10, 5, 3};
    neuropet::CreatureStats b{2, 8, 4, 3};
    neuropet::OnchainArena arena{"127.0.0.1", 8545, "0x0000000000000000000000000000000000000000"};
    arena.battle(a, b);
    return 0;
}
