#pragma once

#define HARMONICS_PLUGIN_IMPL
#include "harmonics/plugin.hpp"
#include "harmonics/version.hpp"
#undef HARMONICS_PLUGIN_IMPL
#include "neuropet/http_client.hpp"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace neuropet {

using PluginHandle = harmonics::PluginHandle;

struct MarketplacePlugin {
    std::string name;
    std::string version;
    std::string url;
};

inline bool parse_manifest(const std::filesystem::path& file, int* harmonics_ver) {
    if (!std::filesystem::exists(file))
        return true;
    std::ifstream in(file);
    if (!in)
        return true;
    std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    auto pos = data.find("\"harmonics_version\"");
    if (pos == std::string::npos)
        return true;
    pos = data.find(':', pos);
    if (pos == std::string::npos)
        return true;
    ++pos;
    while (pos < data.size() && std::isspace(static_cast<unsigned char>(data[pos])))
        ++pos;
    if (pos >= data.size())
        return true;
    if (data[pos] == '"' || data[pos] == '\'')
        ++pos;
    std::string num;
    while (pos < data.size() && std::isdigit(static_cast<unsigned char>(data[pos])))
        num += data[pos++];
    if (!num.empty() && harmonics_ver)
        *harmonics_ver = std::atoi(num.c_str());
    return true;
}

inline bool manifest_version_ok(const std::filesystem::path& lib) {
    auto manifest = lib.parent_path() / "plugin.json";
    int hv = 0;
    if (!parse_manifest(manifest, &hv))
        return false;
    return hv == 0 || hv == harmonics::version();
}

inline std::vector<MarketplacePlugin>
fetch_available_plugins(const std::string& host, unsigned short port,
                        const std::string& path = "/api/plugins") {
    std::string resp;
    if (!http_get(host, port, path, &resp))
        return {};
    std::vector<MarketplacePlugin> out;
    std::regex item(
        R"PLUG("name"\s*:\s*"([^"]+)"[^{}]*"version"\s*:\s*"([^"]+)"[^{}]*"url"\s*:\s*"([^"]+)")PLUG");
    for (std::sregex_iterator it(resp.begin(), resp.end(), item), end; it != end; ++it) {
        MarketplacePlugin p{(*it)[1], (*it)[2], (*it)[3]};
        out.push_back(std::move(p));
    }
    return out;
}

inline std::vector<PluginHandle> load_kernel_plugins(const std::string& path = "",
                                                     const std::string& market_host = "",
                                                     unsigned short market_port = 0) {
    if (!market_host.empty())
        (void)fetch_available_plugins(market_host, market_port);
    std::vector<PluginHandle> handles;
    const char* env = std::getenv("HARMONICS_PLUGIN_PATH");
    std::string search = path.empty() && env ? env : path;
    if (search.empty())
        return handles;
    std::stringstream ss(search);
    std::string dir;
    namespace fs = std::filesystem;
    while (std::getline(ss, dir, ':')) {
        if (dir.empty())
            continue;
        fs::path p{dir};
        if (!fs::exists(p) || !fs::is_directory(p))
            continue;
        for (const auto& entry : fs::directory_iterator(p)) {
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                if (!manifest_version_ok(entry.path()))
                    continue;
                try {
                    handles.push_back(harmonics::load_plugin(entry.path().string()));
                } catch (...) {
                }
            }
        }
    }
    return handles;
}

inline std::future<std::vector<PluginHandle>>
load_kernel_plugins_async(const std::string& path = "", const std::string& market_host = "",
                          unsigned short market_port = 0) {
    return std::async(std::launch::async,
                      [=]() { return load_kernel_plugins(path, market_host, market_port); });
}

inline void unload_kernel_plugins(std::vector<PluginHandle>& handles) {
    harmonics::unload_plugins(handles);
}

inline std::future<void> unload_kernel_plugins_async(std::vector<PluginHandle>& handles) {
    return harmonics::unload_plugins_async(handles);
}

} // namespace neuropet
