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

    DiscoveredLocationStats statsFromPlayer;
    DiscoveredLocationStats statsFromReferences;

    std::unordered_map<RE::TESFile*, std::uint32_t> markersPerFile;

    auto* player           = RE::PlayerCharacter::GetSingleton();
    auto* clearableKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypeClearable");

    collections_set<RE::MapMarkerData*> discoveredMapMarkers;

    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    statsFromPlayer.totalLocations++;
                    markersPerFile[marker->GetFile(0)]++;
                    if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                        discoveredMapMarkers.insert(mapData);
                    }
                    // if (marker->HasKeyword(clearableKeyword)) statsFromPlayer.clearedLocations++;
                }
            }
        }
    }

    collections_set<RE::MapMarkerData*> discoveredByPlayerAndIsInTheBigList;

    auto workspaceBasedCountOfMarkers = 0;
    auto worldspaces                  = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESWorldSpace>();
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

                        if (discoveredMapMarkers.contains(mapData)) {
                            discoveredByPlayerAndIsInTheBigList.insert(mapData);
                        }

                        workspaceBasedCountOfMarkers++;

                        statsFromReferences.totalLocations++;
                        markersPerFile[ref->GetFile(0)]++;
                        if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo))
                            statsFromReferences.discoveredLocations++;
                        if (ref->HasKeyword(clearableKeyword)) statsFromReferences.clearedLocations++;
                    }
                }
            }
        }
    }

    Log("Ok, the player has {} total discovered locations, {} of which are in the big list", statsFromPlayer.totalLocations, discoveredByPlayerAndIsInTheBigList.size());

    // Print out the stats from the player
    Log("Player discovered locations: {} / {} (cleared: {})", statsFromPlayer.discoveredLocations, statsFromPlayer.totalLocations, statsFromPlayer.clearedLocations);

    // Print out the stats from the references
    Log("References discovered locations: {} / {} (cleared: {})", statsFromReferences.discoveredLocations, statsFromReferences.totalLocations,
        statsFromReferences.clearedLocations);

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    Log("Recalculation took {} ms", durationMs);

    return statsFromReferences;
}
