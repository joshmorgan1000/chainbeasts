#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "harmonics/cycle.hpp"
#include "harmonics/serialization.hpp"
#include "harmonics/version.hpp"

namespace harmonics {

inline std::filesystem::path kernels_dir() {
    const char* env = std::getenv("HARMONICS_CACHE_DIR");
    std::filesystem::path dir = env ? env : ".harmonics/cache";
    dir /= "kernels";
    std::filesystem::create_directories(dir);
    return dir;
}

inline void store_gpu_kernels(const GpuCycleKernels& kernels, const std::string& key) {
    std::filesystem::path path = kernels_dir() / (key + ".gpu");
    std::ofstream out(path, std::ios::binary);
    if (!out)
        return;
    out.write("HGKG", 4);
    std::uint32_t ver = static_cast<std::uint32_t>(version());
    out.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
    out.write(reinterpret_cast<const char*>(&kernels.backend), sizeof(kernels.backend));
    out.write(reinterpret_cast<const char*>(&kernels.compiled), sizeof(kernels.compiled));
    out.write(reinterpret_cast<const char*>(&kernels.device_index), sizeof(kernels.device_index));
    std::uint32_t count = static_cast<std::uint32_t>(kernels.ops.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& op : kernels.ops) {
        out.write(reinterpret_cast<const char*>(&op.source.kind), sizeof(op.source.kind));
        out.write(reinterpret_cast<const char*>(&op.source.index), sizeof(op.source.index));
        out.write(reinterpret_cast<const char*>(&op.target.kind), sizeof(op.target.kind));
        out.write(reinterpret_cast<const char*>(&op.target.index), sizeof(op.target.index));
        std::uint8_t back = op.backward ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&back), sizeof(back));
        std::uint8_t has_fn = op.func ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&has_fn), sizeof(has_fn));
        if (op.func)
            write_string(out, *op.func);
        write_string(out, op.shader);
        std::uint32_t words = static_cast<std::uint32_t>(op.spirv.size());
        out.write(reinterpret_cast<const char*>(&words), sizeof(words));
        if (words)
            out.write(reinterpret_cast<const char*>(op.spirv.data()), words * sizeof(uint32_t));
    }
}

inline std::optional<GpuCycleKernels> load_gpu_kernels(const std::string& key) {
    std::filesystem::path path = kernels_dir() / (key + ".gpu");
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::nullopt;
    char magic[4];
    in.read(magic, 4);
    if (std::string(magic, 4) != "HGKG")
        return std::nullopt;
    std::uint32_t ver;
    in.read(reinterpret_cast<char*>(&ver), sizeof(ver));
    (void)ver;
    GpuCycleKernels kernels{};
    in.read(reinterpret_cast<char*>(&kernels.backend), sizeof(kernels.backend));
    in.read(reinterpret_cast<char*>(&kernels.compiled), sizeof(kernels.compiled));
    in.read(reinterpret_cast<char*>(&kernels.device_index), sizeof(kernels.device_index));
    std::uint32_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    kernels.ops.resize(count);
    for (auto& op : kernels.ops) {
        in.read(reinterpret_cast<char*>(&op.source.kind), sizeof(op.source.kind));
        in.read(reinterpret_cast<char*>(&op.source.index), sizeof(op.source.index));
        in.read(reinterpret_cast<char*>(&op.target.kind), sizeof(op.target.kind));
        in.read(reinterpret_cast<char*>(&op.target.index), sizeof(op.target.index));
        std::uint8_t back{};
        in.read(reinterpret_cast<char*>(&back), sizeof(back));
        op.backward = back != 0;
        std::uint8_t has_fn{};
        in.read(reinterpret_cast<char*>(&has_fn), sizeof(has_fn));
        if (has_fn)
            op.func = read_string(in);
        op.shader = read_string(in);
        std::uint32_t words;
        in.read(reinterpret_cast<char*>(&words), sizeof(words));
        op.spirv.resize(words);
        if (words)
            in.read(reinterpret_cast<char*>(op.spirv.data()), words * sizeof(uint32_t));
    }
    return kernels;
}

inline void store_fpga_kernels(const FpgaCycleKernels& kernels, const std::string& key) {
    std::filesystem::path path = kernels_dir() / (key + ".fpga");
    std::ofstream out(path, std::ios::binary);
    if (!out)
        return;
    out.write("HGKF", 4);
    std::uint32_t ver = static_cast<std::uint32_t>(version());
    out.write(reinterpret_cast<const char*>(&ver), sizeof(ver));
    out.write(reinterpret_cast<const char*>(&kernels.backend), sizeof(kernels.backend));
    out.write(reinterpret_cast<const char*>(&kernels.compiled), sizeof(kernels.compiled));
    out.write(reinterpret_cast<const char*>(&kernels.device_index), sizeof(kernels.device_index));
    std::uint32_t count = static_cast<std::uint32_t>(kernels.ops.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& op : kernels.ops) {
        out.write(reinterpret_cast<const char*>(&op.source.kind), sizeof(op.source.kind));
        out.write(reinterpret_cast<const char*>(&op.source.index), sizeof(op.source.index));
        out.write(reinterpret_cast<const char*>(&op.target.kind), sizeof(op.target.kind));
        out.write(reinterpret_cast<const char*>(&op.target.index), sizeof(op.target.index));
        std::uint8_t back = op.backward ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&back), sizeof(back));
        std::uint8_t has_fn = op.func ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&has_fn), sizeof(has_fn));
        if (op.func)
            write_string(out, *op.func);
    }
}

inline std::optional<FpgaCycleKernels> load_fpga_kernels(const std::string& key) {
    std::filesystem::path path = kernels_dir() / (key + ".fpga");
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::nullopt;
    char magic[4];
    in.read(magic, 4);
    if (std::string(magic, 4) != "HGKF")
        return std::nullopt;
    std::uint32_t ver;
    in.read(reinterpret_cast<char*>(&ver), sizeof(ver));
    (void)ver;
    FpgaCycleKernels kernels{};
    in.read(reinterpret_cast<char*>(&kernels.backend), sizeof(kernels.backend));
    in.read(reinterpret_cast<char*>(&kernels.compiled), sizeof(kernels.compiled));
    in.read(reinterpret_cast<char*>(&kernels.device_index), sizeof(kernels.device_index));
    std::uint32_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    kernels.ops.resize(count);
    for (auto& op : kernels.ops) {
        in.read(reinterpret_cast<char*>(&op.source.kind), sizeof(op.source.kind));
        in.read(reinterpret_cast<char*>(&op.source.index), sizeof(op.source.index));
        in.read(reinterpret_cast<char*>(&op.target.kind), sizeof(op.target.kind));
        in.read(reinterpret_cast<char*>(&op.target.index), sizeof(op.target.index));
        std::uint8_t back{};
        in.read(reinterpret_cast<char*>(&back), sizeof(back));
        op.backward = back != 0;
        std::uint8_t has_fn{};
        in.read(reinterpret_cast<char*>(&has_fn), sizeof(has_fn));
        if (has_fn)
            op.func = read_string(in);
    }
    return kernels;
}

} // namespace harmonics
