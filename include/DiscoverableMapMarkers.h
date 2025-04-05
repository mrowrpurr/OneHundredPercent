#pragma once

#include <RE/Skyrim.h>
#include <collections.h>

#include <cstdint>

struct DiscoverableMapMarkers {
    std::uint32_t                                           totalDiscoverableMapMarkersCount = 0;
    collections_map<RE::MapMarkerData*, RE::TESObjectREFR*> DiscoverableMapMarkersToReferences;
    collections_set<RE::BGSLocation*>                       DiscoverableMapMarkers;
};

void                    ReloadDiscoverableMapMarkers();
DiscoverableMapMarkers* GetDiscoverableMapMarkers();
