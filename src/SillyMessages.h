#pragma once

#include <collections.h>

#include <string>

using MessageCollection = collections_map<std::string, std::vector<std::string>>;

struct SillyMessages {
    MessageCollection PercentageDiscoveredMessages;
    MessageCollection OnSpecificLocationDiscovered;
    MessageCollection OnMatchingLocationDiscovered;
    MessageCollection OnSpecificLocationCleared;
    MessageCollection OnMatchingLocationCleared;

    static SillyMessages& instance();
};
