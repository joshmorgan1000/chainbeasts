#include "neuropet/onchain_verifier.hpp"
#include "neuropet/validator.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(ValidatorTest, GenerateAndVerify) {
    neuropet::Validator v(3, neuropet::Blake3ProofSystem::instance());
    std::vector<std::vector<int8_t>> tensors{{2, 1, 0, 0, 3, 2}, {1, 0, 0, 0, 3, 1}};
    auto proof = v.generate_stark_proof(tensors);
    EXPECT_EQ(v.verify_stark_proof(tensors, proof), true);
    float expected_loss = neuropet::compute_loss(tensors);
    EXPECT_EQ(std::fabs(proof.loss - expected_loss) < 1e-5f, true);
    EXPECT_EQ(v.replay_and_verify(tensors, proof.root), true);
}

TEST(ValidatorTest, ProveAndSubmit) {
    neuropet::Validator v(3, neuropet::Blake3ProofSystem::instance());
    std::vector<std::vector<int8_t>> tensors{{2, 1, 0, 0, 3, 2}, {1, 0, 0, 0, 3, 1}};
    neuropet::OnchainProofVerifier onchain("127.0.0.1", 0, "0xdeadbeef");
    EXPECT_EQ(v.prove_and_submit(tensors, onchain), false);
}

TEST(ValidatorTest, RejectsLossIncrease) {
    neuropet::Validator v;
    std::vector<std::vector<int8_t>> t1{{2, 1, 0, 0, 3, 2}, {1, 0, 0, 0, 3, 1}};
    auto p1 = v.generate_stark_proof(t1);
    EXPECT_EQ(v.verify_stark_proof(t1, p1), true);
    std::vector<std::vector<int8_t>> t2{{2, 1, 0, 0, 3, 2}, {0, 1, 0, 0, 3, 1}};
    auto p2 = v.generate_stark_proof(t2);
    EXPECT_EQ(v.verify_stark_proof(t2, p2), false);
}

TEST(ValidatorTest, ReplayMismatch) {
    neuropet::Validator v;
    std::vector<std::vector<int8_t>> t{{1, 2, 3}};
    auto p = v.generate_stark_proof(t);
    std::string other_root = p.root;
    if (!other_root.empty())
        other_root[0] ^= 1;
    EXPECT_EQ(v.replay_and_verify(t, other_root), false);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
