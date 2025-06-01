#include "neuropet/proof_system.hpp"
#include "neuropet/zk_proof_system.hpp"
#include <gtest/gtest.h>
#include <vector>

extern "C" {
void zk_generate_proof(const int8_t* data, std::size_t len, zk_proof_raw* out);
bool zk_verify_proof(const int8_t* data, std::size_t len, const zk_proof_raw* proof);
}

TEST(StarkProverTest, GenerateAndVerify) {
    std::vector<int8_t> data{1, 2, 3, 4, 5};
    zk_proof_raw proof{};
    zk_generate_proof(data.data(), data.size(), &proof);

    constexpr uint32_t PRIME = 0x7fffffff;
    constexpr uint32_t X = 17;
    uint64_t val = 0;
    uint64_t pow = 1;
    for (size_t i = 0; i < data.size(); ++i) {
        int64_t c = static_cast<int64_t>(data[i]);
        if (c < 0)
            c += PRIME;
        val = (val + static_cast<uint64_t>(c) * pow) % PRIME;
        pow = (pow * X) % PRIME;
    }
    uint32_t res = static_cast<uint32_t>(val);
    std::string expected_proof = neuropet::to_hex(reinterpret_cast<uint8_t*>(&res), sizeof(res));
    std::string expected_root =
        neuropet::keccak256_digest(reinterpret_cast<uint8_t*>(&res), sizeof(res));

    EXPECT_EQ(std::string(proof.proof), expected_proof);
    EXPECT_EQ(std::string(proof.root), expected_root);
    EXPECT_EQ(zk_verify_proof(data.data(), data.size(), &proof), true);
}

TEST(StarkProverTest, RejectsTamperedProof) {
    std::vector<int8_t> data{1, 2, 3};
    zk_proof_raw proof{};
    zk_generate_proof(data.data(), data.size(), &proof);
    zk_proof_raw bad = proof;
    bad.proof[0] ^= 1;
    EXPECT_EQ(zk_verify_proof(data.data(), data.size(), &bad), false);

    bad = proof;
    bad.root[0] ^= 1;
    EXPECT_EQ(zk_verify_proof(data.data(), data.size(), &bad), false);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
