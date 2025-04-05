#pragma once

#include <RE/Skyrim.h>
#include <collections.h>

#include <cstdint>

class DiscoverableMapMarkers {
    std::uint32_t                                           totalDiscoverableMapMarkersCount = 0;
    collections_map<RE::MapMarkerData*, RE::TESObjectREFR*> discoverableMapMarkersToReferences;
    collections_set<RE::MapMarkerData*>                     discoverableMapMarkers;

public:
    std::uint32_t                                            GetTotalDiscoverableMapMarkersCount() const { return totalDiscoverableMapMarkersCount; }
    collections_map<RE::MapMarkerData*, RE::TESObjectREFR*>& GetDiscoverableMapMarkersToReferences() { return discoverableMapMarkersToReferences; }
    collections_set<RE::MapMarkerData*>&                     GetDiscoverableMapMarkers() { return discoverableMapMarkers; }

    void AddDiscoverableMapMarker(RE::MapMarkerData* mapMarkerData, RE::TESObjectREFR* ref) {
        discoverableMapMarkersToReferences.emplace(mapMarkerData, ref);
        discoverableMapMarkers.emplace(mapMarkerData);
        totalDiscoverableMapMarkersCount++;
    }
};

void                    ReloadDiscoverableMapMarkers();
DiscoverableMapMarkers* GetDiscoverableMapMarkers();
