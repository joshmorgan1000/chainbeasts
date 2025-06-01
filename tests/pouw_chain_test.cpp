#include "neuropet/pouw_chain.hpp"
#include <gtest/gtest.h>

TEST(PoUWChainTest, RewardsAfterFinalization) {
    neuropet::PoUWChain chain(2);
    chain.submit_checkpoint(1, 0, "dead", "miner", 1.0f, 5);
    chain.attest("dead");
    chain.attest("dead");
    EXPECT_EQ(chain.finalize_checkpoint("dead"), true);
    EXPECT_EQ(chain.chain().size(), 1u);
    EXPECT_EQ(chain.energy_balance("miner"), 5u);
    EXPECT_EQ(chain.core_balance("miner"), 5u * neuropet::PoUWChain::CORE_PER_ENERGY);
}

TEST(PoUWChainTest, FinalizeWithProof) {
    neuropet::PoUWChain chain(3);
    chain.submit_checkpoint(1, 0, "p0", "miner", 0.0f, 7);
    chain.submit_proof("p0");
    EXPECT_EQ(chain.finalize_checkpoint("p0"), true);
    EXPECT_EQ(chain.chain().size(), 1u);
    EXPECT_EQ(chain.energy_balance("miner"), 7u);
    EXPECT_EQ(chain.core_balance("miner"), 7u * neuropet::PoUWChain::CORE_PER_ENERGY);
}

TEST(PoUWChainTest, RejectEpochMismatch) {
    neuropet::PoUWChain chain(1);
    chain.submit_checkpoint(1, 0, "r0", "miner", 1.0f, 3);
    chain.attest("r0");
    EXPECT_EQ(chain.finalize_checkpoint("r0"), true);

    chain.submit_checkpoint(1, 2, "r2", "miner", 0.5f, 3);
    chain.attest("r2");
    EXPECT_EQ(chain.finalize_checkpoint("r2"), false);
}

TEST(PoUWChainTest, RejectLossIncrease) {
    neuropet::PoUWChain chain(1);
    chain.submit_checkpoint(1, 0, "r0", "miner", 1.0f, 4);
    chain.attest("r0");
    EXPECT_EQ(chain.finalize_checkpoint("r0"), true);

    chain.submit_checkpoint(1, 1, "r1", "miner", 2.0f, 4);
    chain.attest("r1");
    EXPECT_EQ(chain.finalize_checkpoint("r1"), false);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
