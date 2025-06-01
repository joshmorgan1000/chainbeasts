#pragma once

#include <algorithm>
#include <blake3.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string>
#include <vector>

namespace neuropet {

inline std::string to_hex(const unsigned char* data, std::size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out(2 * len, '0');
    for (std::size_t i = 0; i < len; ++i) {
        out[2 * i] = hex[data[i] >> 4];
        out[2 * i + 1] = hex[data[i] & 0xf];
    }
    return out;
}

inline std::string blake3_digest(const void* data, std::size_t size) {
    uint8_t out[BLAKE3_OUT_LEN];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, data, size);
    blake3_hasher_finalize(&hasher, out, BLAKE3_OUT_LEN);
    return to_hex(out, BLAKE3_OUT_LEN);
}

inline std::string sha256_digest(const void* data, std::size_t size) {
    unsigned char out[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, size);
    SHA256_Final(out, &ctx);
    return to_hex(out, SHA256_DIGEST_LENGTH);
}

inline std::string keccak256_digest(const void* data, std::size_t size) {
    unsigned char out[32];
    unsigned int outlen = sizeof(out);
    EVP_Digest(data, size, out, &outlen, EVP_sha3_256(), nullptr);
    return to_hex(out, outlen);
}

/**
 * @brief Compute a simple loss metric from a collection of tensors.
 *
 * The metric is the root mean square of all tensor elements, providing a
 * deterministic scalar value for the proof system without relying on any
 * training logic.
 */
inline float compute_loss(const std::vector<std::vector<int8_t>>& tensors) {
    if (tensors.size() < 2)
        return 0.0f;

    const auto& pred = tensors[0];
    const auto& target = tensors[1];
    if (pred.size() < 6 || target.size() < 6)
        return 0.0f;

    // Cross-entropy over move logits (first 4 values).
    float max_logit = static_cast<float>(*std::max_element(pred.begin(), pred.begin() + 4));
    float exp_sum = 0.0f;
    float probs[4];
    for (int i = 0; i < 4; ++i) {
        probs[i] = std::exp(static_cast<float>(pred[i]) - max_logit);
        exp_sum += probs[i];
    }
    for (int i = 0; i < 4; ++i)
        probs[i] /= exp_sum;

    float ce = 0.0f;
    for (int i = 0; i < 4; ++i) {
        float t = static_cast<float>(target[i]);
        float p = std::max(probs[i], 1e-7f);
        ce += -t * std::log(p);
    }

    // MSE for attack power (index 4).
    float p4 = static_cast<float>(pred[4]);
    float t4 = static_cast<float>(target[4]);
    float mse = (p4 - t4) * (p4 - t4);

    // Binary cross-entropy for block flag (index 5).
    float logit = static_cast<float>(pred[5]);
    float t5 = static_cast<float>(target[5]);
    float prob = 1.0f / (1.0f + std::exp(-logit));
    prob = std::clamp(prob, 1e-7f, 1.0f - 1e-7f);
    float bce = -(t5 * std::log(prob) + (1.0f - t5) * std::log(1.0f - prob));

    return ce + mse + bce;
}

struct StarkProof {
    std::string root;
    std::string proof;
    float loss{0.0f};
};

class IProofSystem {
  public:
    virtual ~IProofSystem() = default;
    virtual StarkProof generate_proof(const std::vector<std::vector<int8_t>>& tensors) const = 0;
    virtual bool verify_proof(const std::vector<std::vector<int8_t>>& tensors,
                              const StarkProof& proof) const = 0;
};

class Blake3ProofSystem : public IProofSystem {
  public:
    static const Blake3ProofSystem& instance() {
        static Blake3ProofSystem inst;
        return inst;
    }

    /**
     * @brief Generate a proof by hashing all tensors with BLAKE3.
     *
     * Each tensor is hashed in sequence so the proof depends on their order as
     * well as their contents. The resulting root is hashed again to derive the
     * proof field. A simple loss metric is attached using ``compute_loss``.
     */
    StarkProof generate_proof(const std::vector<std::vector<int8_t>>& tensors) const override {
        blake3_hasher hasher;
        blake3_hasher_init(&hasher);
        for (const auto& t : tensors) {
            if (!t.empty())
                blake3_hasher_update(&hasher, t.data(), t.size());
        }
        uint8_t out[BLAKE3_OUT_LEN];
        blake3_hasher_finalize(&hasher, out, BLAKE3_OUT_LEN);
        StarkProof p{};
        p.root = to_hex(out, BLAKE3_OUT_LEN);
        p.proof = blake3_digest(p.root.data(), p.root.size());
        p.loss = compute_loss(tensors);
        return p;
    }

    /**
     * @brief Recompute the proof and compare it against the provided one.
     *
     * The BLAKE3 digest is derived from the tensors exactly as in
     * ``generate_proof``. Both the root hash and the embedded loss value must
     * match in order for verification to succeed.
     */
    bool verify_proof(const std::vector<std::vector<int8_t>>& tensors,
                      const StarkProof& proof) const override {
        auto expected = generate_proof(tensors);
        if (expected.root != proof.root)
            return false;
        if (expected.loss != proof.loss)
            return false;
        return blake3_digest(proof.root.data(), proof.root.size()) == proof.proof;
    }

  private:
    Blake3ProofSystem() = default;
};

} // namespace neuropet
