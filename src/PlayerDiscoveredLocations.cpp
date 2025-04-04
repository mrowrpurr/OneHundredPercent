#include "PlayerDiscoveredLocations.h"

#include <SkyrimScripting/Logging.h>

#include "DiscoverableLocations.h"
#include "FormUtils.h"
#include "JsonFiles.h"
#include "SaveData.h"

collections_set<RE::BGSLocation*> GetPlayerDiscoveredLocationList() {
    collections_set<RE::BGSLocation*> discoveredLocations;

    auto* discoverableLocations = GetDiscoverableLocationInfo();

    // 1. Get the discovered locations from the current map marker data
    //    this does NOT work when indoors and depends on the worldspace.
    //    It is NOT great but it gets us SOMETHING if you add this mod mid-playthru.
    auto* player = RE::PlayerCharacter::GetSingleton();

    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                        auto findLocation = discoverableLocations->discoverableMapMarkersToLocations.find(mapData);
                        if (findLocation != discoverableLocations->discoverableMapMarkersToLocations.end()) {
                            if (IgnoredLocationIDs.contains(findLocation->second->GetFormID())) continue;  // Skip any ignored location IDs
                            if (IgnoredMapMarkers.contains(ToLowerCase(std::format("{}::{}", findLocation->second->GetFullName(), mapData->locationName.GetFullName()))))
                                continue;  // Skip any ignored map markers
                            discoveredLocations.insert(findLocation->second);
                            Log("[Player Marker] Discovered location: {}::{} ... {:x} @ {}", findLocation->second->GetFullName(), mapData->locationName.GetFullName(),
                                findLocation->second->GetLocalFormID(), findLocation->second->GetFile(0)->GetFilename());
                        }
                    }
                }
            }
        }
    }

    Log("[Player Marker Total Locations] {}", discoveredLocations.size());

    // 2. Now take into consideration the ACTUAL discovered locations via save game.
    auto&                                            saveData = GetSaveData();
    collections_map<std::string, const RE::TESFile*> pluginNameToFileMap;

    for (auto& locationEvent : saveData.locationEvents) {
        if (!locationEvent.locationPluginName.empty()) {
            const RE::TESFile* plugin      = nullptr;
            auto               foundPlugin = pluginNameToFileMap.find(locationEvent.locationPluginName);
            if (foundPlugin == pluginNameToFileMap.end()) {
                plugin = RE::TESDataHandler::GetSingleton()->LookupModByName(locationEvent.locationPluginName.c_str());
                if (plugin) pluginNameToFileMap[locationEvent.locationPluginName] = plugin;
            } else {
                plugin = foundPlugin->second;
            }
            if (plugin) {
                auto localFormId   = locationEvent.locationFormID;
                auto formId        = GetFormID(plugin, localFormId);
                auto foundLocation = RE::TESForm::LookupByID<RE::BGSLocation>(formId);
                if (foundLocation) {
                    discoveredLocations.insert(foundLocation);
                } else {
                    Log("Failed to find location with form ID: {:x} in plugin: {}", localFormId, locationEvent.locationPluginName);
                }
            } else {
                Log("Plugin not found or invalid: {}", locationEvent.locationPluginName);
            }
        }
    }

    Log("[Save Game Total Locations] {}", saveData.locationEvents.size());
    Log("[Total Discovered Locations] {}", discoveredLocations.size());

    return discoveredLocations;
}

std::uint32_t GetNumberOfPlayerDiscoveredLocations() {
    // To start with, just get the full array and return the size :P
    auto discoveredLocations = GetPlayerDiscoveredLocationList();
    return static_cast<std::uint32_t>(discoveredLocations.size());
}
