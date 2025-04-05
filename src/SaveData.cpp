#pragma once

#include "SaveData.h"

#include <SkyrimScripting/Logging.h>

#include "Config.h"

inline void WriteString(SKSE::SerializationInterface* intfc, const std::string& str) {
    std::size_t len = str.size();
    intfc->WriteRecordData(len);
    intfc->WriteRecordData(str.data(), len);
}

inline bool ReadString(SKSE::SerializationInterface* intfc, std::string& out) {
    std::size_t len;
    if (!intfc->ReadRecordData(len)) return false;
    out.resize(len);
    return intfc->ReadRecordData(out.data(), len);
}

void LocationEvent::Save(SKSE::SerializationInterface* intfc) const {
    intfc->WriteRecordData(formIdentifier);
    WriteString(intfc, locationName);
    intfc->WriteRecordData(static_cast<std::uint32_t>(eventType));
    intfc->WriteRecordData(eventTime);
    intfc->WriteRecordData(eventPosition);
    intfc->WriteRecordData(eventRotation);
    WriteString(intfc, eventCellName);
}

void LocationEvent::Load(SKSE::SerializationInterface* intfc) {
    intfc->ReadRecordData(formIdentifier);
    ReadString(intfc, locationName);
    std::uint32_t type;
    intfc->ReadRecordData(type);
    eventType = static_cast<LocationEventType>(type);
    intfc->ReadRecordData(eventTime);
    intfc->ReadRecordData(eventPosition);
    intfc->ReadRecordData(eventRotation);
    ReadString(intfc, eventCellName);
}

void SaveData::Save(SKSE::SerializationInterface* intfc) {
    // Save discoveryEvents map
    std::uint32_t mapCount = static_cast<std::uint32_t>(discoveryEvents.size());
    intfc->WriteRecordData(mapCount);

    // Iterate through the map and save each LocationEvent
    for (const auto& [formIdentifier, event] : discoveryEvents) {
        event.Save(intfc);
    }

    // Save recentlyDiscoveredMarkers vector
    std::uint32_t vectorCount = static_cast<std::uint32_t>(recentlyDiscoveredMarkers.size());
    intfc->WriteRecordData(vectorCount);

    // Iterate through the vector and save each FormIdentifier
    for (const auto& formIdentifier : recentlyDiscoveredMarkers) {
        intfc->WriteRecordData(formIdentifier);
    }
}

void SaveData::Load(SKSE::SerializationInterface* intfc) {
    // Load discoveryEvents map
    discoveryEvents.clear();
    std::uint32_t mapCount;
    intfc->ReadRecordData(mapCount);

    for (std::uint32_t i = 0; i < mapCount; ++i) {
        LocationEvent event;
        event.Load(intfc);
        discoveryEvents.emplace(event.formIdentifier, std::move(event));
    }

    // Load recentlyDiscoveredMarkers vector
    recentlyDiscoveredMarkers.clear();
    std::uint32_t vectorCount;
    intfc->ReadRecordData(vectorCount);

    // Iterate through the vector and load each FormIdentifier
    for (std::uint32_t i = 0; i < vectorCount; ++i) {
        FormIdentifier formIdentifier;
        intfc->ReadRecordData(formIdentifier);
        recentlyDiscoveredMarkers.push_back(formIdentifier);
    }
}

void SaveCallback(SKSE::SerializationInterface* a_intfc) {
    Log("[cosave] SaveCallback called");
    if (a_intfc->OpenRecord(Config::COSAVE_ID, Config::COSAVE_VERSION)) g_saveData.Save(a_intfc);
}

void LoadCallback(SKSE::SerializationInterface* a_intfc) {
    Log("[cosave] LoadCallback called");
    std::uint32_t type, version, length;
    while (a_intfc->GetNextRecordInfo(type, version, length)) {
        if (type == Config::COSAVE_ID) {
            g_saveData.Load(a_intfc);
            g_isSaveDataLoaded = true;
        }
    }
}

void RevertCallback(SKSE::SerializationInterface* a_intfc) {
    Log("[cosave] RevertCallback called");
    g_saveData         = {};
    g_isSaveDataLoaded = false;
}

void SetupSaveCallbacks() {
    const auto* serializationInterface = SKSE::GetSerializationInterface();
    serializationInterface->SetUniqueID(Config::COSAVE_ID);
    serializationInterface->SetRevertCallback(RevertCallback);
    serializationInterface->SetSaveCallback(SaveCallback);
    serializationInterface->SetLoadCallback(LoadCallback);
}

collections_map<FormIdentifier, LocationEvent>& SaveData::GetDiscoveryEvents() { return discoveryEvents; }

std::vector<FormIdentifier>& SaveData::GetRecentlyDiscoveredMapMarkerIDs() { return recentlyDiscoveredMarkers; }

void SaveData::SaveDiscoveryEvent(LocationEventType type, const RE::MapMarkerData* mapMarkerData) {
    // Log("[Save] [SaveDiscoveryEvent] {} - {}", LocationEventTypeToString(type), mapMarkerData->locationName);

    // auto* player         = RE::PlayerCharacter::GetSingleton();
    // auto  locationFormId = FormIdentifier::CreateIdentifier(location);

    // auto existing = discoveryEvents.find(locationFormId);
    // if (existing != discoveryEvents.end()) {
    //     // If it is, update the time...
    //     existing->second.eventTime = RE::Calendar::GetSingleton()->GetCurrentGameTime();
    //     // And if the new event type is "Cleared", update the event type to "Cleared" if it was previously "Discovered"
    //     if (existing->second.eventType == LocationEventType::Discovered && type == LocationEventType::Cleared) existing->second.eventType = LocationEventType::Cleared;
    //     Log("[Save] [UpdateLocationEvent] {} - {} - {}", existing->second.locationName, LocationEventTypeToString(existing->second.eventType), existing->second.eventCellName);
    //     return &existing->second;
    // }

    // auto* currentLocation = player->GetCurrentLocation();

    // auto emplaceResult = discoveryEvents.emplace(
    //     locationFormId,
    //     LocationEvent{
    //         locationFormId,
    //         std::string{location->GetFullName()},
    //         type,
    //         RE::Calendar::GetSingleton()->GetCurrentGameTime(),
    //         player->GetPosition(),
    //         player->GetAngle(),
    //         currentLocation ? currentLocation->GetFullName() : "<Unknown Location>",
    //     }
    // );

    // if (!emplaceResult.second) {
    //     Log("[Save] [Failed to add location event] {} - {} - {}", emplaceResult.first->second.locationName, LocationEventTypeToString(emplaceResult.first->second.eventType),
    //         emplaceResult.first->second.eventCellName);
    //     return nullptr;
    // }

    // // Add it to the vector that has the ordered list of discovered locations
    // // UNLESS this is a discovery from the player map, which means
    // // we don't know exactly WHEN it was actually discovered
    // if (type != LocationEventType::DiscoveredFromPlayerMap) recentlyDiscoveredMarkers.push_back(locationFormId);

    // // If the emplace was successful, we can log the event
    // auto& addedLocationEvent = emplaceResult.first->second;
    // Log("[Save] [AddLocationEvent] {} - {} - {}", addedLocationEvent.locationName, LocationEventTypeToString(addedLocationEvent.eventType), addedLocationEvent.eventCellName);
    // return &addedLocationEvent;
}

// void SaveData::DiscoveredMapMarker(const RE::BGSLocation* location) { SaveDiscoveryEvent(LocationEventType::Discovered, location); }
// // void SaveData::ClearedLocation(const RE::BGSLocation* location) { SaveDiscoveryEvent(LocationEventType::Cleared, location); }
// void SaveData::FoundPreviouslyDiscoveredLocationOnPlayersMap(const RE::BGSLocation* location) { SaveDiscoveryEvent(LocationEventType::DiscoveredFromPlayerMap, location); }

LocationEvent* SaveData::LookupMapMarker(const FormIdentifier& mapMarkerReferenceFormID) {
    auto found = discoveryEvents.find(mapMarkerReferenceFormID);
    if (found != discoveryEvents.end()) return &found->second;
    return nullptr;
}

LocationEvent* SaveData::LookupMapMarker(const RE::MapMarkerData* mapMarkerData) {
    // return LookupMapMarker(FormIdentifier::CreateIdentifier(location));
    return nullptr;
}

LocationEvent* SaveData::GetMostRecentlyDiscoveredLocation() {
    if (recentlyDiscoveredMarkers.empty()) return nullptr;
    auto lastDiscoveredLocation = recentlyDiscoveredMarkers.back();
    auto found                  = discoveryEvents.find(lastDiscoveredLocation);
    if (found != discoveryEvents.end()) return &found->second;
    return nullptr;
}

LocationEvent* SaveData::GetRecentlyDiscoveredLocation(std::uint32_t index) {
    if (index >= recentlyDiscoveredMarkers.size()) return nullptr;
    auto found = discoveryEvents.find(recentlyDiscoveredMarkers[index]);
    if (found != discoveryEvents.end()) return &found->second;
    return nullptr;
}

bool SaveData::IsMapMarkerDiscovered(const RE::MapMarkerData* mapMarkerData) const {
    //
    // return discoveryEvents.contains(FormIdentifier::CreateIdentifier(location));
    return false;
}

std::uint32_t SaveData::GetTotalDiscoveredMapMarkersCount() const { return discoveryEvents.size(); }

std::uint32_t SaveData::GetRecentlyDiscoveredMapMarkersCount() const { return recentlyDiscoveredMarkers.size(); }
