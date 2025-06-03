#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "neuropet/hatching.hpp"     // for keccak256_bytes
#include "neuropet/proof_system.hpp" // to_hex utility

namespace neuropet {

constexpr unsigned MAX_SENSORS = 4;
constexpr unsigned MAX_APPENDAGES = 4;

/**\brief HSV colour encoded as 4-bit components. */
struct HSV {
    std::uint8_t h{0};
    std::uint8_t s{0};
    std::uint8_t v{0};
};

/**\brief Visual trait collection for a creature. */
struct VisualTraits {
    std::uint8_t body_shape{0};
    HSV body_colour{};
    std::array<std::uint8_t, MAX_SENSORS> sensor_shape{};
    std::array<HSV, MAX_SENSORS> sensor_colour{};
    std::array<std::uint8_t, MAX_APPENDAGES> appendage_shape{};
    std::array<HSV, MAX_APPENDAGES> appendage_colour{};
    std::uint8_t sensor_count{0};
    std::uint8_t appendage_count{0};
};

inline std::uint16_t pack_hsv(const HSV& c) {
    return static_cast<std::uint16_t>(((c.h & 0xF) << 8) | ((c.s & 0xF) << 4) | (c.v & 0xF));
}

inline HSV unpack_hsv(std::uint16_t p) {
    HSV c{};
    c.h = static_cast<std::uint8_t>((p >> 8) & 0xF);
    c.s = static_cast<std::uint8_t>((p >> 4) & 0xF);
    c.v = static_cast<std::uint8_t>(p & 0xF);
    return c;
}

inline unsigned __int128 pack_traits(const VisualTraits& t) {
    unsigned __int128 out = 0;
    unsigned bits = 0;
    auto push = [&](unsigned value, unsigned width) {
        out |= (static_cast<unsigned __int128>(value) &
                ((static_cast<unsigned __int128>(1) << width) - 1))
               << bits;
        bits += width;
    };

    push(t.body_shape, 4);
    push(pack_hsv(t.body_colour), 12);
    for (unsigned i = 0; i < t.sensor_count && i < MAX_SENSORS; ++i) {
        push(t.sensor_shape[i], 4);
        push(pack_hsv(t.sensor_colour[i]), 12);
    }
    for (unsigned i = 0; i < t.appendage_count && i < MAX_APPENDAGES; ++i) {
        push(t.appendage_shape[i], 4);
        push(pack_hsv(t.appendage_colour[i]), 12);
    }
    return out;
}

inline VisualTraits unpack_traits(unsigned __int128 packed, std::uint8_t sensors,
                                  std::uint8_t appendages) {
    VisualTraits t{};
    t.sensor_count = sensors;
    t.appendage_count = appendages;
    unsigned bits = 0;
    auto pop = [&](unsigned width) {
        unsigned __int128 mask = (static_cast<unsigned __int128>(1) << width) - 1;
        unsigned val = static_cast<unsigned>((packed >> bits) & mask);
        bits += width;
        return val;
    };

    t.body_shape = pop(4);
    t.body_colour = unpack_hsv(pop(12));
    for (unsigned i = 0; i < sensors && i < MAX_SENSORS; ++i) {
        t.sensor_shape[i] = pop(4);
        t.sensor_colour[i] = unpack_hsv(pop(12));
    }
    for (unsigned i = 0; i < appendages && i < MAX_APPENDAGES; ++i) {
        t.appendage_shape[i] = pop(4);
        t.appendage_colour[i] = unpack_hsv(pop(12));
    }
    return t;
}

inline std::array<std::uint8_t, 32> trait_name_hash(const std::string& name) {
    return keccak256_bytes(name.data(), name.size());
}

inline std::array<unsigned char, 16> traits_to_bytes(unsigned __int128 v) {
    std::array<unsigned char, 16> out{};
    for (int i = 0; i < 16; ++i)
        out[15 - i] = static_cast<unsigned char>(v >> (i * 8));
    return out;
}

inline std::string traits_hex(unsigned __int128 v) {
    std::array<unsigned char, 32> buf{};
    auto bytes16 = traits_to_bytes(v);
    std::copy(bytes16.begin(), bytes16.end(), buf.begin() + 16);
    return to_hex(buf.data(), buf.size());
}

} // namespace neuropet
