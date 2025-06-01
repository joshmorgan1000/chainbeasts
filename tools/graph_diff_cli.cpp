#include <fstream>
#include <iostream>
#include <string>

#include <harmonics/graph_diff.hpp>
#include <harmonics/serialization.hpp>
#include <cstdint>

using namespace harmonics;

static void write_flow(std::ostream& out, const GraphDiff::Flow& f) {
    write_string(out, f.src);
    write_string(out, f.dst);
    std::uint8_t back = f.backward ? 1 : 0;
    out.write(reinterpret_cast<const char*>(&back), sizeof(back));
    std::uint8_t has = f.func ? 1 : 0;
    out.write(reinterpret_cast<const char*>(&has), sizeof(has));
    if (has)
        write_string(out, *f.func);
}

static GraphDiff::Flow read_flow(std::istream& in) {
    GraphDiff::Flow f{};
    f.src = read_string(in);
    f.dst = read_string(in);
    std::uint8_t back;
    in.read(reinterpret_cast<char*>(&back), sizeof(back));
    f.backward = back != 0;
    std::uint8_t has;
    in.read(reinterpret_cast<char*>(&has), sizeof(has));
    if (has)
        f.func = read_string(in);
    return f;
}

static void save_graph_diff(const GraphDiff& diff, std::ostream& out) {
    out.write("HGRD", 4);
    std::uint32_t count;
    count = diff.added_layers.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& l : diff.added_layers)
        write_string(out, l.name);
    count = diff.removed_layers.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& s : diff.removed_layers)
        write_string(out, s);
    count = diff.added_flows.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& f : diff.added_flows)
        write_flow(out, f);
    count = diff.removed_flows.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& f : diff.removed_flows)
        write_flow(out, f);
}

static GraphDiff load_graph_diff(std::istream& in) {
    char magic[4];
    in.read(magic, 4);
    if (std::string(magic, 4) != "HGRD")
        throw std::runtime_error("invalid diff file");
    GraphDiff diff;
    std::uint32_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    diff.added_layers.resize(count);
    for (auto& l : diff.added_layers)
        l.name = read_string(in);
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    diff.removed_layers.resize(count);
    for (auto& s : diff.removed_layers)
        s = read_string(in);
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    diff.added_flows.resize(count);
    for (auto& f : diff.added_flows)
        f = read_flow(in);
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    diff.removed_flows.resize(count);
    for (auto& f : diff.removed_flows)
        f = read_flow(in);
    return diff;
}

static void usage(const char* arg0) {
    std::cout << "Usage: " << arg0
              << " <diff|apply|merge> <files> [-o out]" << std::endl;
}

static void print_summary(const GraphDiff& diff) {
    std::cout << diff.added_layers.size() << " layers added, "
              << diff.removed_layers.size() << " layers removed, "
              << diff.added_flows.size() << " flows added, "
              << diff.removed_flows.size() << " flows removed" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    std::string cmd = argv[1];
    if (cmd == "diff") {
        if (argc < 4) {
            usage(argv[0]);
            return 1;
        }
        std::string base_path = argv[2];
        std::string upd_path = argv[3];
        std::string out_path;
        for (int i = 4; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc)
                out_path = argv[++i];
        }
        std::ifstream in1(base_path, std::ios::binary);
        std::ifstream in2(upd_path, std::ios::binary);
        if (!in1 || !in2) {
            std::cerr << "failed to open input files" << std::endl;
            return 1;
        }
        auto g1 = load_graph(in1);
        auto g2 = load_graph(in2);
        auto diff = diff_graphs(g1, g2);
        if (!out_path.empty()) {
            std::ofstream out(out_path, std::ios::binary);
            if (!out) {
                std::cerr << "failed to open output file" << std::endl;
                return 1;
            }
            save_graph_diff(diff, out);
        } else {
            print_summary(diff);
        }
        return 0;
    } else if (cmd == "apply") {
        if (argc < 4) {
            usage(argv[0]);
            return 1;
        }
        std::string graph_path = argv[2];
        std::string diff_path = argv[3];
        std::string out_path = graph_path;
        for (int i = 4; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc)
                out_path = argv[++i];
        }
        std::ifstream gin(graph_path, std::ios::binary);
        std::ifstream din(diff_path, std::ios::binary);
        if (!gin || !din) {
            std::cerr << "failed to open input files" << std::endl;
            return 1;
        }
        auto g = load_graph(gin);
        auto diff = load_graph_diff(din);
        apply_diff(g, diff);
        std::ofstream out(out_path, std::ios::binary);
        if (!out) {
            std::cerr << "failed to open output file" << std::endl;
            return 1;
        }
        save_graph(g, out);
        return 0;
    } else if (cmd == "merge") {
        if (argc < 4) {
            usage(argv[0]);
            return 1;
        }
        std::string base_path = argv[2];
        std::string patch_path = argv[3];
        std::string out_path = base_path;
        for (int i = 4; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc)
                out_path = argv[++i];
        }
        std::ifstream in1(base_path, std::ios::binary);
        std::ifstream in2(patch_path, std::ios::binary);
        if (!in1 || !in2) {
            std::cerr << "failed to open input files" << std::endl;
            return 1;
        }
        auto g1 = load_graph(in1);
        auto g2 = load_graph(in2);
        auto merged = merge_graphs(g1, g2);
        std::ofstream out(out_path, std::ios::binary);
        if (!out) {
            std::cerr << "failed to open output file" << std::endl;
            return 1;
        }
        save_graph(merged, out);
        return 0;
    } else {
        usage(argv[0]);
        return 1;
    }
}
