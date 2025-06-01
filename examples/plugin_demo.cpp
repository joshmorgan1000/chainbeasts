#include "neuropet/plugin_loader.hpp"
#include <iostream>

int main(int argc, char** argv) {
    std::string path;
    if (argc > 1)
        path = argv[1];

    auto handles = neuropet::load_kernel_plugins(path);
    std::cout << "Loaded " << handles.size() << " plugin(s)" << std::endl;
    neuropet::unload_kernel_plugins(handles);
    return 0;
}
