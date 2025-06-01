#pragma once

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace neuropet {

/** Type of an INT8 network layer. */
enum class Int8Op : std::uint8_t { Dense = 0, ReLU = 1 };

/** Description of a single network layer. */
struct Int8Layer {
    Int8Op op{Int8Op::Dense};
    std::size_t input{0};
    std::size_t output{0};
    std::vector<int8_t> weights; // size = input * output for Dense
    std::vector<int8_t> bias;    // size = output
};

/** Simple sequential INT8 network specification. */
struct Int8Network {
    std::vector<Int8Layer> layers{};
};

inline void write_u32(std::ostream& out, std::uint32_t v) {
    out.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

inline std::uint32_t read_u32(std::istream& in) {
    std::uint32_t v;
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    if (!in)
        throw std::runtime_error("failed to read u32");
    return v;
}

/** Save a network to a binary file. */
inline void save_network(const Int8Network& net, const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    out.write("N8NW", 4);
    write_u32(out, 1); // version
    write_u32(out, static_cast<std::uint32_t>(net.layers.size()));
    for (const auto& l : net.layers) {
        std::uint8_t op = static_cast<std::uint8_t>(l.op);
        out.write(reinterpret_cast<const char*>(&op), sizeof(op));
        write_u32(out, static_cast<std::uint32_t>(l.input));
        write_u32(out, static_cast<std::uint32_t>(l.output));
        write_u32(out, static_cast<std::uint32_t>(l.weights.size()));
        out.write(reinterpret_cast<const char*>(l.weights.data()), l.weights.size());
        write_u32(out, static_cast<std::uint32_t>(l.bias.size()));
        out.write(reinterpret_cast<const char*>(l.bias.data()), l.bias.size());
    }
    if (!out)
        throw std::runtime_error("failed to write network");
}

/** Load a network from a binary file. */
inline Int8Network load_network(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    char magic[4];
    in.read(magic, 4);
    if (std::string(magic, 4) != "N8NW")
        throw std::runtime_error("invalid network file");
    (void)read_u32(in); // version
    Int8Network net;
    std::uint32_t count = read_u32(in);
    net.layers.resize(count);
    for (auto& l : net.layers) {
        std::uint8_t op;
        in.read(reinterpret_cast<char*>(&op), sizeof(op));
        l.op = static_cast<Int8Op>(op);
        l.input = read_u32(in);
        l.output = read_u32(in);
        std::uint32_t w = read_u32(in);
        l.weights.resize(w);
        in.read(reinterpret_cast<char*>(l.weights.data()), w);
        std::uint32_t b = read_u32(in);
        l.bias.resize(b);
        in.read(reinterpret_cast<char*>(l.bias.data()), b);
    }
    if (!in)
        throw std::runtime_error("failed to read network");
    return net;
}

} // namespace neuropet
