#include "DiscoveredLocations.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include <chrono>

DiscoveredLocationStats GetDiscoveredLocationStats() {
    Log("Recalculating total number of discovered locations with map markers...");
    auto now = std::chrono::steady_clock::now();

    DiscoveredLocationStats stats;

    auto* player           = RE::PlayerCharacter::GetSingleton();
    auto* clearableKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypeClearable");

    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    stats.totalLocations++;
                    Log("{}", mapData->locationName.GetFullName());
                    if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                        stats.discoveredLocations++;
                    }
                    if (marker->HasKeyword(clearableKeyword)) {
                        stats.clearedLocations++;
                        Log("CLEARED location: {}", mapData->locationName.GetFullName());
                    }
                }
            }
        }
    }

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    Log("Recalculation took {} ms", durationMs);

    return stats;
}
