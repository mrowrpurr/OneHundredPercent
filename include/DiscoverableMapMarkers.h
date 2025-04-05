#pragma once

#include <RE/Skyrim.h>
#include <collections.h>

#include <cstdint>

class DiscoverableMapMarkers {
    std::uint32_t                                                       totalDiscoverableMapMarkersCount = 0;
    collections_map<const RE::MapMarkerData*, const RE::TESObjectREFR*> discoverableMapMarkersToReferences;
    collections_set<const RE::MapMarkerData*>                           discoverableMapMarkers;

public:
    std::uint32_t                                                        GetTotalDiscoverableMapMarkersCount() const;
    collections_map<const RE::MapMarkerData*, const RE::TESObjectREFR*>& GetDiscoverableMapMarkersToReferences();
    collections_set<const RE::MapMarkerData*>&                           GetDiscoverableMapMarkers();

    void                     AddDiscoverableMapMarker(const RE::MapMarkerData* mapMarkerData, const RE::TESObjectREFR* ref);
    const RE::TESObjectREFR* GetReferenceForMarker(const RE::MapMarkerData* mapMarkerData);
};

void                    ReloadDiscoverableMapMarkers();
DiscoverableMapMarkers* GetDiscoverableMapMarkers();
