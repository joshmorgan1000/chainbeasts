#include "neuropet/validator.hpp"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>

TEST(ZkProofSystemTest, GenerateAndVerify) {
#if defined(__unix__)
    const char* path = "./libstark_prover_v2.so";
#elif defined(_WIN32)
    const char* path = "./stark_prover_v2.dll";
#endif
    if (!std::filesystem::exists(path))
        return;
#if defined(__unix__)
    setenv("ZK_PROVER_PATH", path, 1);
#else
    _putenv_s("ZK_PROVER_PATH", path);
#endif
    neuropet::Validator v(3, neuropet::ZkProofSystem::instance());
    std::vector<std::vector<int8_t>> tensors{{2, 1, 0, 0, 3, 2}, {1, 0, 0, 0, 3, 1}};
    auto proof = v.generate_stark_proof(tensors);
    EXPECT_EQ(v.verify_stark_proof(tensors, proof), true);
    float expected_loss = neuropet::compute_loss(tensors);
    EXPECT_EQ(std::fabs(proof.loss - expected_loss) < 1e-5f, true);
}

TEST(ZkProofSystemTest, HandlesSingleTensor) {
#if defined(__unix__)
    const char* path2 = "./libstark_prover_v2.so";
#elif defined(_WIN32)
    const char* path2 = "./stark_prover_v2.dll";
#endif
    if (!std::filesystem::exists(path2))
        return;
#if defined(__unix__)
    setenv("ZK_PROVER_PATH", path2, 1);
#else
    _putenv_s("ZK_PROVER_PATH", path2);
#endif
    neuropet::Validator v(3, neuropet::ZkProofSystem::instance());
    std::vector<std::vector<int8_t>> tensors{{1, 2, 3}};
    auto proof = v.generate_stark_proof(tensors);
    EXPECT_EQ(v.verify_stark_proof(tensors, proof), true);
    EXPECT_EQ(proof.loss, 0.0f);
}

TEST(ZkProofSystemTest, RejectsMismatchedData) {
#if defined(__unix__)
    const char* path3 = "./libstark_prover_v2.so";
#elif defined(_WIN32)
    const char* path3 = "./stark_prover_v2.dll";
#endif
    if (!std::filesystem::exists(path3))
        return;
#if defined(__unix__)
    setenv("ZK_PROVER_PATH", path3, 1);
#else
    _putenv_s("ZK_PROVER_PATH", path3);
#endif
    neuropet::Validator v(3, neuropet::ZkProofSystem::instance());
    std::vector<std::vector<int8_t>> t1{{2, 1, 0, 0, 3, 2}, {1, 0, 0, 0, 3, 1}};
    auto proof = v.generate_stark_proof(t1);
    auto t2 = t1;
    if (!t2.empty() && !t2[0].empty())
        t2[0][0] ^= 1;
    EXPECT_EQ(v.verify_stark_proof(t2, proof), false);
}

TEST(ZkProofSystemTest, RejectsTamperedProof) {
#if defined(__unix__)
    const char* path4 = "./libstark_prover_v2.so";
#elif defined(_WIN32)
    const char* path4 = "./stark_prover_v2.dll";
#endif
    if (!std::filesystem::exists(path4))
        return;
#if defined(__unix__)
    setenv("ZK_PROVER_PATH", path4, 1);
#else
    _putenv_s("ZK_PROVER_PATH", path4);
#endif
    neuropet::Validator v(3, neuropet::ZkProofSystem::instance());
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
