#include "DiscoveredLocations.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>
#include <collections.h>

#include <chrono>

#include "JsonFiles.h"

DiscoveredLocationStats GetDiscoveredLocationStats() {
    Log("Recalculating total number of discovered locations with map markers...");
    auto now = std::chrono::steady_clock::now();

    DiscoveredLocationStats discoveredLocations;

    auto* player = RE::PlayerCharacter::GetSingleton();

    collections_set<RE::MapMarkerData*> discoveredFromPlayerMap;

    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                        if (mapData->locationName.fullName == "Solitude Military Camp") continue;  // Not sure how to skip these ones quite yet
                        discoveredFromPlayerMap.insert(mapData);
                    }
                }
            }
        }
    }

    collections_map<RE::MapMarkerData*, RE::BGSLocation*> discoveredFromWorldSpaceToLocations;

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
                if (!location || location->fullName.empty()) continue;

                if (auto* marker = ref->extraList.GetByType<RE::ExtraMapMarker>()) {
                    auto* mapData = marker->mapData;
                    if (mapData) {
                        discoveredFromWorldSpaceToLocations.insert_or_assign(mapData, location);

                        if (IgnoredLocationIDs.contains(location->GetFormID())) {
                            discoveredFromPlayerMap.erase(mapData);
                            continue;
                        }

                        // 1. Must have a name
                        if (mapData->locationName.fullName.empty()) continue;

                        auto mapDataAddr = reinterpret_cast<uintptr_t>(mapData);

                        // 2. Should NOT be visible by default
                        // (If visible at game start, it's probably a quest point, not a discoverable)
                        if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible)) {
                            // Log("(visible) [Worldspace Marker] w/ MapData: {} @ {:x} -- {:x} @ {}", mapData->locationName.GetFullName(), mapDataAddr, location->GetLocalFormID(),
                            //     location->GetFile(0)->GetFilename());
                            continue;
                        }

                        discoveredLocations.totalLocations++;

                        if (discoveredFromPlayerMap.contains(mapData)) {
                            Log("[Is in worldspace + Player discovered] DISCOVERED: {}", mapData->locationName.GetFullName());
                            discoveredLocations.discoveredLocations++;
                        } else {
                            // Is it discovered?
                            if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                                Log("[Just worldspace discovered]: {}", mapData->locationName.GetFullName());
                                discoveredLocations.discoveredLocations++;
                            }
                        }
                    }
                }
            }
        }
    }

    // Print out the player map ones:
    for (auto& markerPtr : discoveredFromPlayerMap) {
        auto foundFromWorldSpace = discoveredFromWorldSpaceToLocations.find(markerPtr);
        if (foundFromWorldSpace == discoveredFromWorldSpaceToLocations.end()) discoveredFromPlayerMap.erase(markerPtr);
        else Log("[Player discovered] DISCOVERED: {}", markerPtr->locationName.GetFullName());
    }

    // Print out the discoveredLocations stats:
    Log("Total locations: {}", discoveredLocations.totalLocations);
    Log("Discovered locations: {}", discoveredLocations.discoveredLocations);
    Log("Discovered locations from player map: {}", discoveredFromPlayerMap.size());

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    Log("Recalculation took {} ms", durationMs);

    discoveredLocations.clearedLocations    = 0;
    discoveredLocations.discoveredLocations = discoveredFromPlayerMap.size();  // + ?

    return discoveredLocations;
}
