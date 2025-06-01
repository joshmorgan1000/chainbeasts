#pragma once

#define HARMONICS_PLUGIN_IMPL
#include "harmonics/plugin.hpp"
#undef HARMONICS_PLUGIN_IMPL

namespace neuropet {

using PluginHandle = harmonics::PluginHandle;

inline std::vector<PluginHandle> load_kernel_plugins(const std::string& path = "") {
    return harmonics::load_plugins_from_path(path);
}

inline std::future<std::vector<PluginHandle>>
load_kernel_plugins_async(const std::string& path = "") {
    return harmonics::load_plugins_from_path_async(path);
}

inline void unload_kernel_plugins(std::vector<PluginHandle>& handles) {
    harmonics::unload_plugins(handles);
}

inline std::future<void> unload_kernel_plugins_async(std::vector<PluginHandle>& handles) {
    return harmonics::unload_plugins_async(handles);
}

} // namespace neuropet
