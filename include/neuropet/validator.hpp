#pragma once

#include "neuropet/proof_aggregator.hpp"
#include "neuropet/proof_system.hpp"
#include "neuropet/zk_proof_system.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace neuropet {

/** Off-chain checkpoint validator with quorum replay and proof generation. */
class Validator {
  public:
    explicit Validator(std::size_t quorum = 3,
                       const IProofSystem& system = ZkProofSystem::instance(),
                       ProofAggregatorServer* p2p = nullptr)
        : quorum_(quorum), proof_system_(system), p2p_(p2p) {
        if (p2p_)
            p2p_->set_callback([this](const StarkProof& p) { attest(p.root); });
    }

    /** Generate a proof digest from INT8 tensors. */
    std::string generate_proof(const std::vector<std::vector<int8_t>>& tensors) const {
        return proof_system_.generate_proof(tensors).root;
    }

    /** Generate a STARK proof for the tensors and broadcast it. */
    StarkProof generate_stark_proof(const std::vector<std::vector<int8_t>>& tensors) const {
        StarkProof proof = proof_system_.generate_proof(tensors);
        if (p2p_)
            p2p_->submit(proof);
        return proof;
    }

    /** Verify a STARK proof against tensors. */
    bool verify_stark_proof(const std::vector<std::vector<int8_t>>& tensors,
                            const StarkProof& proof) const {
        if (!proof_system_.verify_proof(tensors, proof))
            return false; // The proof itself was invalid.

        // Only accept a proof when the reported loss is not worse than
        // the previous accepted value. This enforces monotonic improvement
        // during a training session.
        float loss = compute_loss(tensors);
        if (last_loss_ < loss)
            return false;
        last_loss_ = loss;
        return true;
    }

    /**
     * Generate and verify a STARK proof, then submit it to an on-chain verifier.
     * The proof is only submitted when verification succeeds.
     */
    template <typename Verifier>
    bool prove_and_submit(const std::vector<std::vector<int8_t>>& tensors,
                          Verifier& onchain) const {
        StarkProof proof = proof_system_.generate_proof(tensors);
        if (!proof_system_.verify_proof(tensors, proof))
            return false;
        if (last_loss_ < proof.loss)
            return false;
        last_loss_ = proof.loss;
        if (!onchain.submit_proof(proof))
            return false;
        if (p2p_)
            p2p_->submit(proof);
        return true;
    }

    /** Replay tensors and verify the resulting proof. */
    bool replay_and_verify(const std::vector<std::vector<int8_t>>& tensors,
                           const std::string& expected_root) const {
        return proof_system_.generate_proof(tensors).root == expected_root;
    }

    /** Record an attestation for the given root hash. */
    void attest(const std::string& root) { ++counts_[root]; }

    /** Check if a root hash has reached quorum. */
    bool has_quorum(const std::string& root) const {
        auto it = counts_.find(root);
        return it != counts_.end() && it->second >= quorum_;
    }

  private:
    std::size_t quorum_;
    const IProofSystem& proof_system_;
    ProofAggregatorServer* p2p_{nullptr};
    std::unordered_map<std::string, std::size_t> counts_{};
    mutable float last_loss_{std::numeric_limits<float>::infinity()};
};

} // namespace neuropet
