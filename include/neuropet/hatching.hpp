#pragma once

#include <array>
#include <cstdint>
#include <openssl/evp.h>
#include <vector>

#include "neuropet/int8_spec.hpp"

namespace neuropet {

struct HatchTopology {
    std::uint8_t sensor_count{1};
    std::uint8_t append_count{1};
    std::array<std::uint8_t, 4> sensor_size{0};
    std::array<std::uint8_t, 4> append_size{0};
};

inline std::array<std::uint8_t, 32> keccak256_bytes(const void* data, std::size_t size) {
    std::array<std::uint8_t, 32> out{};
    unsigned int outlen = out.size();
    EVP_Digest(data, size, out.data(), &outlen, EVP_sha3_256(), nullptr);
    return out;
}

inline HatchTopology derive_topology(const std::array<std::uint8_t, 32>& seed) {
    auto bits = keccak256_bytes(seed.data(), seed.size());
    static constexpr std::array<std::uint8_t, 4> SIZE_TABLE{32, 48, 64, 96};
    HatchTopology t{};
    std::uint16_t sc =
        static_cast<std::uint16_t>(bits[0]) | (static_cast<std::uint16_t>(bits[1]) << 8);
    std::uint16_t ac =
        static_cast<std::uint16_t>(bits[2]) | (static_cast<std::uint16_t>(bits[3]) << 8);
    t.sensor_count = static_cast<std::uint8_t>(1 + sc % 4);
    t.append_count = static_cast<std::uint8_t>(1 + ac % 4);
    for (int i = 0; i < 4; ++i) {
        t.sensor_size[i] = SIZE_TABLE[bits[4 + 2 * i] & 3];
        t.append_size[i] = SIZE_TABLE[bits[20 + 2 * i] & 3];
    }
    return t;
}

inline std::uint64_t splitmix64(std::uint64_t& state) {
    std::uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

inline int8_t next_weight(std::uint64_t& state) {
    std::uint64_t r = splitmix64(state);
    return static_cast<int8_t>((r >> 2) % 17 - 8);
}

inline std::vector<int8_t> generate_genesis_weights(std::size_t count, std::uint64_t seed) {
    std::uint64_t state = seed;
    std::vector<int8_t> out(count);
    for (std::size_t i = 0; i < count; ++i)
        out[i] = next_weight(state);
    return out;
}

} // namespace neuropet
