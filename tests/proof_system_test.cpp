#include "neuropet/validator.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(ProofSystemTest, GenerateAndVerify) {
    neuropet::Validator v(3, neuropet::Blake3ProofSystem::instance());
    std::vector<std::vector<int8_t>> tensors{{2, 1, 0, 0, 3, 2}, {1, 0, 0, 0, 3, 1}};
    auto proof = v.generate_stark_proof(tensors);
    EXPECT_EQ(v.verify_stark_proof(tensors, proof), true);
    float expected_loss = neuropet::compute_loss(tensors);
    EXPECT_EQ(std::fabs(proof.loss - expected_loss) < 1e-5f, true);
}

TEST(ProofSystemTest, RejectsTamperedProof) {
    neuropet::Validator v(3, neuropet::Blake3ProofSystem::instance());
    std::vector<std::vector<int8_t>> tensors{{1, 2, 3}};
    auto proof = v.generate_stark_proof(tensors);
    neuropet::StarkProof bad = proof;
    if (!bad.root.empty())
        bad.root[0] ^= 1;
    EXPECT_EQ(v.verify_stark_proof(tensors, bad), false);

    bad = proof;
    bad.loss += 1.f;
    EXPECT_EQ(v.verify_stark_proof(tensors, bad), false);

    bad = proof;
    if (!bad.proof.empty())
        bad.proof[0] ^= 1;
    EXPECT_EQ(v.verify_stark_proof(tensors, bad), false);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
