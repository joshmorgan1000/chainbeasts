#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <harmonics/graph.hpp>
#include <harmonics/graph_diff.hpp>
#include <harmonics/parser.hpp>
#include <harmonics/serialization.hpp>

TEST(GraphDiffCli, DiffAndApply) {
    const char* base_src = "producer p; consumer c; cycle { p -> c; }";
    const char* upd_src = "producer p; consumer c; layer l; cycle { p -> l; l -> c; }";
    harmonics::Parser p1{base_src};
    auto g1 = harmonics::build_graph(p1.parse_declarations());
    harmonics::Parser p2{upd_src};
    auto g2 = harmonics::build_graph(p2.parse_declarations());

    {
        std::ofstream out("base.hgr", std::ios::binary);
        harmonics::save_graph(g1, out);
    }
    {
        std::ofstream out("upd.hgr", std::ios::binary);
        harmonics::save_graph(g2, out);
    }

    ASSERT_EQ(std::system("./graph_diff_cli diff base.hgr upd.hgr -o diff.hgrd"), 0);
    ASSERT_EQ(std::system("./graph_diff_cli apply base.hgr diff.hgrd -o new.hgr"), 0);

    std::ifstream in("new.hgr", std::ios::binary);
    auto merged = harmonics::load_graph(in);
    EXPECT_EQ(harmonics::graph_digest(merged), harmonics::graph_digest(g2));
}

TEST(GraphDiffCli, Merge) {
    const char* base_src = "producer p; consumer c; cycle { p -> c; }";
    const char* upd_src = "producer p; consumer c; layer l; cycle { p -> l; l -> c; }";
    harmonics::Parser p1{base_src};
    auto g1 = harmonics::build_graph(p1.parse_declarations());
    harmonics::Parser p2{upd_src};
    auto g2 = harmonics::build_graph(p2.parse_declarations());

    {
        std::ofstream out("base.hgr", std::ios::binary);
        harmonics::save_graph(g1, out);
    }
    {
        std::ofstream out("patch.hgr", std::ios::binary);
        harmonics::save_graph(g2, out);
    }

    ASSERT_EQ(std::system("./graph_diff_cli merge base.hgr patch.hgr -o merged.hgr"), 0);

    std::ifstream in("merged.hgr", std::ios::binary);
    auto merged = harmonics::load_graph(in);
    EXPECT_EQ(harmonics::graph_digest(merged), harmonics::graph_digest(g2));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
