#pragma once

#include <RE/Skyrim.h>
#include <collections.h>

#include <cstdint>

struct DiscoverableLocationInfo {
    std::uint32_t                                         totalDiscoverableLocationCount = 0;
    collections_map<RE::MapMarkerData*, RE::BGSLocation*> discoverableMapMarkersToLocations;
};

void                      ReloadDiscoverableLocationInfo();
DiscoverableLocationInfo* GetDiscoverableLocationInfo();
