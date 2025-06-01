#pragma once

#include "neuropet/proof_system.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

extern "C" {
struct zk_proof_raw {
    char root[65];
    char proof[65];
};
}

namespace neuropet {

/** Proof system backed by external STARK/SNARK circuits. */
class ZkProofSystem : public IProofSystem {
  public:
    static const ZkProofSystem& instance();

    StarkProof generate_proof(const std::vector<std::vector<int8_t>>& tensors) const override;
    bool verify_proof(const std::vector<std::vector<int8_t>>& tensors,
                      const StarkProof& proof) const override;

  private:
    ZkProofSystem();
    ~ZkProofSystem();
    using GenerateFn = void (*)(const int8_t*, std::size_t, zk_proof_raw*);
    using VerifyFn = bool (*)(const int8_t*, std::size_t, const zk_proof_raw*);
    void* handle_{nullptr};
    GenerateFn gen_{nullptr};
    VerifyFn verify_{nullptr};
};

} // namespace neuropet
