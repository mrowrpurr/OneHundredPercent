#include "DiscoverableLocations.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>
#include <collections.h>

#include <atomic>
#include <chrono>

#include "JsonFiles.h"

std::unique_ptr<DiscoverableLocationInfo> g_DiscoverableLocations = nullptr;

std::atomic<bool> g_DiscoverableLocationsReloading{false};

DiscoverableLocationInfo* GetDiscoverableLocationInfo() { return g_DiscoverableLocations.get(); }

void ReloadDiscoverableLocationInfo() {
    if (g_DiscoverableLocationsReloading.exchange(true)) {
        Log("Reloading discovered locations already in progress, skipping...");
        return;
    }

    Log("Reloading discovered locations...");
    auto now = std::chrono::steady_clock::now();

    auto locationInfo = std::make_unique<DiscoverableLocationInfo>();

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
                        if (IgnoredLocationIDs.contains(location->GetFormID())) continue;
                        if (mapData->locationName.fullName.empty()) continue;
                        if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible)) continue;

                        locationInfo->totalDiscoverableLocationCount++;
                        locationInfo->discoverableMapMarkersToLocations[mapData] = location;
                    }
                }
            }
        }
    }

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    Log("Reloading discovered locations took {} ms", durationMs);
    Log("Total discoverable locations: {} (marker count: {})", locationInfo->totalDiscoverableLocationCount, locationInfo->discoverableMapMarkersToLocations.size());

    g_DiscoverableLocations          = std::move(locationInfo);
    g_DiscoverableLocationsReloading = false;
}

// DiscoverableLocationInfo GetDiscoverableLocationInfo() {
//     Log("Recalculating total number of discovered locations with map markers...");
//     auto now = std::chrono::steady_clock::now();

//     DiscoverableLocationInfo DiscoverableLocations;

//     auto* player = RE::PlayerCharacter::GetSingleton();

//     collections_set<RE::MapMarkerData*> discoveredFromPlayerMap;

//     for (auto& markerPtr : player->currentMapMarkers) {
//         if (auto marker = markerPtr.get()) {
//             if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
//                 if (auto* mapData = extraMapMarker->mapData) {
//                     if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
//                         if (mapData->locationName.fullName == "Solitude Military Camp") continue;  // Not sure how to skip these ones quite yet
//                         discoveredFromPlayerMap.insert(mapData);
//                     }
//                 }
//             }
//         }
//     }

//     collections_map<RE::MapMarkerData*, RE::BGSLocation*> discoveredFromWorldSpaceToLocations;

//     // Print out the player map ones:
//     for (auto& markerPtr : discoveredFromPlayerMap) {
//         auto foundFromWorldSpace = discoveredFromWorldSpaceToLocations.find(markerPtr);
//         if (foundFromWorldSpace == discoveredFromWorldSpaceToLocations.end()) discoveredFromPlayerMap.erase(markerPtr);
//         else Log("[Player discovered] DISCOVERED: {}", markerPtr->locationName.GetFullName());
//     }

//     // Print out the DiscoverableLocations stats:
//     Log("Total locations: {}", DiscoverableLocations.totalLocations);
//     Log("Discovered locations: {}", DiscoverableLocations.DiscoverableLocations);
//     Log("Discovered locations from player map: {}", discoveredFromPlayerMap.size());

//     auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
//     Log("Recalculation took {} ms", durationMs);

//     DiscoverableLocations.clearedLocations    = 0;
//     DiscoverableLocations.DiscoverableLocations = discoveredFromPlayerMap.size();  // + ?

//     return DiscoverableLocations;
// }
