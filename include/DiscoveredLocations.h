#pragma once

#include <cstdint>

struct DiscoveredLocationStats {
    std::uint32_t totalLocations      = 0;
    std::uint32_t discoveredLocations = 0;
    std::uint32_t clearedLocations    = 0;
};

DiscoveredLocationStats GetDiscoveredLocationStats();
