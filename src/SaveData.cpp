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
    std::uint32_t count = static_cast<std::uint32_t>(locationEvents.size());
    intfc->WriteRecordData(count);

    for (const auto& [formIdentifier, event] : locationEvents) {
        event.Save(intfc);
    }
}

void SaveData::Load(SKSE::SerializationInterface* intfc) {
    locationEvents.clear();
    std::uint32_t count;
    intfc->ReadRecordData(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        LocationEvent event;
        event.Load(intfc);
        locationEvents.emplace(event.formIdentifier, std::move(event));
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

LocationEvent* SaveData::SaveLocationEvent(LocationEventType type, const RE::BGSLocation* location) {
    auto* player         = RE::PlayerCharacter::GetSingleton();
    auto  locationFormId = FormIdentifier::CreateIdentifier(location);

    auto existing = locationEvents.find(locationFormId);
    if (existing != locationEvents.end()) {
        // If it is, update the time...
        existing->second.eventTime = RE::Calendar::GetSingleton()->GetCurrentGameTime();
        // And if the new event type is "Cleared", update the event type to "Cleared" if it was previously "Discovered"
        if (existing->second.eventType == LocationEventType::Discovered && type == LocationEventType::Cleared) existing->second.eventType = LocationEventType::Cleared;
        Log("[Save] [UpdateLocationEvent] {} - {} - {}", existing->second.locationName, LocationEventTypeToString(existing->second.eventType), existing->second.eventCellName);
        return &existing->second;
    }

    auto* currentLocation = player->GetCurrentLocation();

    auto emplaceResult = locationEvents.emplace(
        locationFormId,
        LocationEvent{
            locationFormId,
            std::string{location->GetFullName()},
            type,
            RE::Calendar::GetSingleton()->GetCurrentGameTime(),
            player->GetPosition(),
            player->GetAngle(),
            currentLocation ? currentLocation->GetFullName() : "<Unknown Location>",
        }
    );

    if (!emplaceResult.second) {
        Log("[Save] [Failed to add location event] {} - {} - {}", emplaceResult.first->second.locationName, LocationEventTypeToString(emplaceResult.first->second.eventType),
            emplaceResult.first->second.eventCellName);
        return nullptr;
    }

    // If the emplace was successful, we can log the event
    auto& addedLocationEvent = emplaceResult.first->second;
    Log("[Save] [AddLocationEvent] {} - {} - {}", addedLocationEvent.locationName, LocationEventTypeToString(addedLocationEvent.eventType), addedLocationEvent.eventCellName);
    return &addedLocationEvent;
}

void SaveData::DiscoveredLocation(const RE::BGSLocation* location) { SaveLocationEvent(LocationEventType::Discovered, location); }
void SaveData::ClearedLocation(const RE::BGSLocation* location) { SaveLocationEvent(LocationEventType::Cleared, location); }
void SaveData::FoundPreviouslyDiscoveredLocationOnPlayersMap(const RE::BGSLocation* location) { SaveLocationEvent(LocationEventType::DiscoveredFromMapMarker, location); }
