#include "DiscoveredLocations.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>
#include <collections.h>

#include <chrono>

std::string ToLowerCase(std::string_view text) {
    std::string result;
    std::ranges::transform(text, std::back_inserter(result), [](unsigned char c) { return std::tolower(c); });
    return result;
}

DiscoveredLocationStats GetDiscoveredLocationStats() {
    Log("Recalculating total number of discovered locations with map markers...");
    auto now = std::chrono::steady_clock::now();

    DiscoveredLocationStats discoveredLocations;

    auto* player = RE::PlayerCharacter::GetSingleton();
    // auto* clearableKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypeClearable");

    collections_set<RE::MapMarkerData*> discoveredLocationsMapMarkerData;

    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                        Log("> DISCOVERED: {}", mapData->locationName.GetFullName());
                        discoveredLocations.discoveredLocations++;
                        discoveredLocationsMapMarkerData.insert(mapData);
                    }
                    // No, this is CLEAR-ABLE, not CLEAR-ed ...
                    // if (marker->HasKeyword(clearableKeyword)) {
                    //     Log("CLEARED: {}", mapData->locationName.GetFullName());
                    //     discoveredLocations.clearedLocations++;
                    // }
                }
            }
        }
    }

    auto worldspaces = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESWorldSpace>();
    for (auto* worldspace : worldspaces) {
        if (!worldspace) continue;

        auto* persistent = worldspace->persistentCell;
        if (persistent) {
            for (auto& refHandle : persistent->references) {
                auto* ref = refHandle.get();
                if (!ref) continue;

                // 3. Location must exist and have a name (i.e., not a dummy marker)
                auto* location = ref->GetCurrentLocation();
                if (!location || location->fullName.empty()) {
                    continue;
                }

                if (auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>()) {
                    auto* mapData = marker->mapData;
                    if (mapData) {
                        // 1. Must have a name
                        if (mapData->locationName.fullName.empty()) {
                            continue;
                        }

                        // 2. Should NOT be visible by default
                        // (If visible at game start, it's probably a quest point, not a discoverable)
                        if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible)) continue;

                        discoveredLocations.totalLocations++;

                        if (!discoveredLocationsMapMarkerData.contains(mapData)) {
                            // Is it discovered?
                            if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                                Log(">>> DISCOVERED: {}", mapData->locationName.GetFullName());
                                discoveredLocations.discoveredLocations++;
                                discoveredLocationsMapMarkerData.insert(mapData);
                            }

                            // Is it cleared
                            // if (ref->HasKeyword(clearableKeyword)) {
                            //     Log("CLEARED: {}", mapData->locationName.GetFullName());
                            //     discoveredLocations.clearedLocations++;
                            // } else {
                            //     Log("NOT CLEARED: {}", mapData->locationName.GetFullName());
                            // }
                        }
                    }
                }
            }
        }
    }

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    Log("Recalculation took {} ms", durationMs);

    return discoveredLocations;
}
