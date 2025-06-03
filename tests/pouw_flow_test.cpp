#include "neuropet/pouw_chain.hpp"
#include "neuropet/proof_aggregator.hpp"
#include "neuropet/validator.hpp"
#include <gtest/gtest.h>

TEST(PoUWFlowTest, FinalizeWithP2P) {
    neuropet::ProofAggregatorServer p2p(0);
    p2p.start();

    neuropet::PoUWChain chain(1, neuropet::Blake3ProofSystem::instance());
    p2p.set_callback([&chain](const neuropet::StarkProof& p) { chain.attest(p.root); });

    neuropet::Validator v(1, neuropet::Blake3ProofSystem::instance(), &p2p);
    std::vector<std::vector<int8_t>> tensors{{1, 2, 3}};
    auto proof = v.generate_stark_proof(tensors);
    chain.submit_checkpoint(1, 0, proof.root, "miner", proof.loss, 6);
    chain.attest(proof.root);

    EXPECT_EQ(chain.finalize_checkpoint(proof.root), true);
    EXPECT_EQ(chain.chain().size(), 1u);
    EXPECT_EQ(chain.energy_balance("miner"), 6u);
    EXPECT_EQ(chain.core_balance("miner"), 6u * neuropet::PoUWChain::CORE_PER_ENERGY);

    p2p.stop();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
