#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace neuropet {

inline std::string strip_0x(const std::string& hex) {
    if (hex.rfind("0x", 0) == 0 || hex.rfind("0X", 0) == 0)
        return hex.substr(2);
    return hex;
}

inline std::string encode_uint32(std::uint32_t v) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::nouppercase << std::setw(64) << v;
    return ss.str();
}

inline std::string encode_int32(std::int32_t v) {
    std::uint32_t u = static_cast<std::uint32_t>(v);
    char buf[9];
    std::snprintf(buf, sizeof(buf), "%08x", u);
    return std::string(56, v < 0 ? 'f' : '0') + buf;
}

inline std::string encode_bytes32(const std::string& hex) {
    std::string h = strip_0x(hex);
    if (h.size() > 64)
        h = h.substr(h.size() - 64);
    return std::string(64 - h.size(), '0') + h;
}

inline std::string encode_uint256(std::uint64_t v) { return encode_uint32(v); }

inline std::string encode_bool(bool v) { return std::string(63, '0') + (v ? '1' : '0'); }

inline std::string encode_bytes(const std::string& hex) {
    std::string h = strip_0x(hex);
    std::ostringstream out;
    out << encode_uint256(h.size() / 2);
    std::string data = h;
    std::size_t rem = data.size() % 64;
    if (rem != 0)
        data += std::string(64 - rem, '0');
    out << data;
    return out.str();
}

} // namespace neuropet
