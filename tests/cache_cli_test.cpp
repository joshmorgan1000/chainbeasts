#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

TEST(CacheCli, ListAndClear) {
    namespace fs = std::filesystem;
    fs::path dir = "cache_cli_test_dir";
    fs::create_directories(dir);
    {
        std::ofstream(dir / "dummy.bin") << 'x';
    }

    std::string list_cmd =
        "HARMONICS_CACHE_DIR=" + dir.string() + " ./cache_cli list > listing.txt";
    ASSERT_EQ(std::system(list_cmd.c_str()), 0);

    std::ifstream in("listing.txt");
    std::string line;
    std::getline(in, line);
    EXPECT_EQ(line, (dir / "dummy.bin").string());

    std::string clear_cmd = "HARMONICS_CACHE_DIR=" + dir.string() + " ./cache_cli clear";
    ASSERT_EQ(std::system(clear_cmd.c_str()), 0);
    EXPECT_EQ(fs::exists(dir), false);

    fs::remove("listing.txt");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
