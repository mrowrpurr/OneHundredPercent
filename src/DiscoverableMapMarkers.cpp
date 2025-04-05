#include "DiscoverableMapMarkers.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>
#include <collections.h>

#include <atomic>
#include <chrono>

#include "JsonFiles.h"

std::unique_ptr<DiscoverableMapMarkers> g_DiscoverableMapMarkers = nullptr;
std::atomic<bool>                       g_hasLoggedFullListOfDiscoverableMapMarkers{false};
std::atomic<bool>                       g_DiscoverableMapMarkersReloading{false};
DiscoverableMapMarkers*                 GetDiscoverableMapMarkers() { return g_DiscoverableMapMarkers.get(); }

void ReloadDiscoverableMapMarkers() {
    if (g_DiscoverableMapMarkersReloading.exchange(true)) {
        Log("Reloading of discoverable map markers already in progress, skipping...");
        return;
    }

    Log("Reloading discoverable map markers ...");
    auto now = std::chrono::steady_clock::now();

    auto                                         locationInfo = std::make_unique<DiscoverableMapMarkers>();
    collections_map<RE::TESFile*, std::uint32_t> countOfDiscoverableMapMarkersPerFile;

    auto discoverableCount = 0;
    auto worldspaces       = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESWorldSpace>();
    for (auto* worldspace : worldspaces) {
        if (!worldspace) continue;

        auto* persistent = worldspace->persistentCell;
        if (persistent) {
            for (auto& refHandle : persistent->references) {
                auto* ref = refHandle.get();
                if (!ref) continue;

                if (auto* extraMapMarker = ref->extraList.GetByType<RE::ExtraMapMarker>()) {
                    if (auto* mapData = extraMapMarker->mapData) {
                        // if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                        //     Log("DISCOVERABLE MAP MARKER: {}", mapData->locationName.GetFullName());
                        //     discoverableCount++;
                        // }
                        if (!mapData->locationName.fullName.empty()) {
                            Log("[Debug] [Discoverable Map Marker]: {} - {}", mapData->locationName.GetFullName(), ref->GetFormID());
                            // locationInfo->DiscoverableMapMarkers.insert(mapData);
                            discoverableCount++;
                        }
                    }
                }
            }
        }
    }

    Log("-> Discoverable Count: {}", discoverableCount);

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
    // Log("Reloading discovered locations took {} ms", durationMs);
    // Log("Total discoverable locations: {} (location count: {}) (marker count: {})", locationInfo->totalDiscoverableMapMarkersCount, locationInfo->DiscoverableMapMarkers.size(),
    //     locationInfo->DiscoverableMapMarkersToReferences.size());
    // Log("Count of discoverable locations per file:");
    // for (const auto& [file, count] : countOfDiscoverableMapMarkersPerFile) Log("File: {} - {} discoverable locations", file->GetFilename(), count);

    g_DiscoverableMapMarkers                    = std::move(locationInfo);
    g_DiscoverableMapMarkersReloading           = false;
    g_hasLoggedFullListOfDiscoverableMapMarkers = true;
}
