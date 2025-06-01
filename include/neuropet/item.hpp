#pragma once

#include <cstdint>

namespace neuropet {

/** Item effect modifying creature stats. */
struct Item {
    std::uint32_t id{0};
    int power{0};
    int defense{0};
    int stamina{0};
};

} // namespace neuropet
