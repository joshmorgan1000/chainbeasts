#include "neuropet/validator.hpp"
#include <gtest/gtest.h>

TEST(ValidatorTest, QuorumTracking) {
    neuropet::Validator v(2, neuropet::Blake3ProofSystem::instance());
    std::string r1 = "deadbeef";
    std::string r2 = "abba";

    EXPECT_EQ(v.has_quorum(r1), false);
    v.attest(r1);
    EXPECT_EQ(v.has_quorum(r1), false);
    v.attest(r1);
    EXPECT_EQ(v.has_quorum(r1), true);

    EXPECT_EQ(v.has_quorum(r2), false);
    v.attest(r2);
    v.attest(r2);
    EXPECT_EQ(v.has_quorum(r2), true);

    // Additional attestations should keep quorum satisfied
    v.attest(r1);
    EXPECT_EQ(v.has_quorum(r1), true);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
