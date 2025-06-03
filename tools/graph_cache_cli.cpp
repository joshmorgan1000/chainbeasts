#include <filesystem>
#include <iostream>
#include <string>

/**
 * @brief Inspect and clear compiled graph caches.
 *
 * The tool operates on the `kernels` subdirectory within the
 * `HARMONICS_CACHE_DIR` directory (default `.harmonics/cache`).
 * It accepts two commands:
 *  - `list`  : prints cached graph digests.
 *  - `clear` : removes all cached kernels.
 */
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <list|clear>" << std::endl;
        return 1;
    }
    std::string cmd = argv[1];
    const char* env = std::getenv("HARMONICS_CACHE_DIR");
    std::filesystem::path cache_dir = env ? env : ".harmonics/cache";
    cache_dir /= "kernels";

    if (cmd == "list") {
        if (!std::filesystem::exists(cache_dir)) {
            std::cout << "Cache directory not found: " << cache_dir << std::endl;
            return 0;
        }
        for (auto& entry : std::filesystem::directory_iterator(cache_dir)) {
            std::cout << entry.path().string() << std::endl;
        }
    } else if (cmd == "clear") {
        if (std::filesystem::exists(cache_dir)) {
            std::filesystem::remove_all(cache_dir);
            std::cout << "Removed cache directory: " << cache_dir << std::endl;
        } else {
            std::cout << "Cache directory not found: " << cache_dir << std::endl;
        }
    } else {
        std::cout << "Unknown command: " << cmd << std::endl;
        return 1;
    }
    return 0;
}
