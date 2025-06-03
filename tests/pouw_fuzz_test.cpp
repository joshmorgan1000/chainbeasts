#ifdef __unix__
#include "neuropet/pouw_chain.hpp"
#include "neuropet/proof_aggregator.hpp"
#include "neuropet/validator.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <random>
#include <thread>

TEST(PoUWFuzzTest, RandomizedFinalization) {
    neuropet::ProofAggregatorServer p2p(0);
    p2p.start();

    neuropet::PoUWChain chain(1, neuropet::Blake3ProofSystem::instance());
    p2p.set_callback([&chain](const neuropet::StarkProof& p) { chain.attest(p.root); });

    neuropet::Validator v(1, neuropet::Blake3ProofSystem::instance(), &p2p);
    std::mt19937 rng(0);
    std::uniform_int_distribution<int> count_dist(1, 4);
    std::uniform_int_distribution<int> val_dist(-3, 3);
    constexpr int ITER = 16;
    float last_loss = 1.0f;
    for (int epoch = 0; epoch < ITER; ++epoch) {
        std::vector<std::vector<int8_t>> tensors;
        int tcount = count_dist(rng);
        for (int i = 0; i < tcount; ++i) {
            int tlen = count_dist(rng);
            std::vector<int8_t> t(tlen);
            for (int j = 0; j < tlen; ++j)
                t[j] = static_cast<int8_t>(val_dist(rng));
            tensors.push_back(t);
        }
        auto proof = v.generate_stark_proof(tensors);
        proof.loss = std::max(0.0f, last_loss - 0.01f);
        last_loss = proof.loss;
        chain.submit_checkpoint(1, static_cast<uint32_t>(epoch), proof.root, "miner", proof.loss,
                                1);
        chain.submit_proof(proof.root);
        bool done = false;
        for (int j = 0; j < 20 && !done; ++j) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            done = chain.finalize_checkpoint(proof.root);
        }
        EXPECT_EQ(done, true);
    }
    EXPECT_EQ(chain.chain().size(), static_cast<size_t>(ITER));
    p2p.stop();
}

TEST(ProofAggregatorFuzzTest, RandomBroadcast) {
    neuropet::ProofAggregatorServer a(0);
    neuropet::ProofAggregatorServer b(0);
    a.start();
    b.start();
    a.connect("127.0.0.1", b.port());
    b.connect("127.0.0.1", a.port());

    neuropet::Validator va(1, neuropet::Blake3ProofSystem::instance(), &a);
    neuropet::Validator vb(1, neuropet::Blake3ProofSystem::instance(), &b);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> count_dist(1, 3);
    std::uniform_int_distribution<int> val_dist(-2, 2);

    constexpr int COUNT = 10;
    std::vector<std::string> roots_a;
    std::vector<std::string> roots_b;
    for (int i = 0; i < COUNT; ++i) {
        std::vector<std::vector<int8_t>> ta(1, std::vector<int8_t>(count_dist(rng)));
        for (auto& v : ta[0])
            v = static_cast<int8_t>(val_dist(rng));
        roots_a.push_back(va.generate_stark_proof(ta).root);

        std::vector<std::vector<int8_t>> tb(1, std::vector<int8_t>(count_dist(rng)));
        for (auto& v : tb[0])
            v = static_cast<int8_t>(val_dist(rng));
        roots_b.push_back(vb.generate_stark_proof(tb).root);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (const auto& r : roots_a)
        EXPECT_EQ(vb.has_quorum(r), true);
    for (const auto& r : roots_b)
        EXPECT_EQ(va.has_quorum(r), true);
    a.stop();
    b.stop();
}
#endif // __unix__

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
