#include "neuropet/proof_aggregator.hpp"
#include "neuropet/validator.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#ifdef __unix__
TEST(ValidatorTest, BroadcastAndIngest) {
    neuropet::ProofAggregatorServer a(0);
    neuropet::ProofAggregatorServer b(0);
    a.start();
    b.start();
    a.connect("127.0.0.1", b.port());
    b.connect("127.0.0.1", a.port());

    neuropet::Validator va(1, neuropet::ZkProofSystem::instance(), &a);
    neuropet::Validator vb(1, neuropet::ZkProofSystem::instance(), &b);

    std::vector<std::vector<int8_t>> tensors{{1, 2, 3}};
    auto proof = va.generate_stark_proof(tensors);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(vb.has_quorum(proof.root), true);
    a.stop();
    b.stop();
}
#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
