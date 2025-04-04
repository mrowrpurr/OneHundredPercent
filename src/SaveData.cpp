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

void SaveData::Save(SKSE::SerializationInterface* intfc) {
    std::uint32_t count = static_cast<std::uint32_t>(locationEvents.size());
    intfc->WriteRecordData(count);

    for (const auto& [key, e] : locationEvents) {
        WriteString(intfc, e.locationName);
        intfc->WriteRecordData(static_cast<std::uint32_t>(e.eventType));
        intfc->WriteRecordData(e.eventTime);
        intfc->WriteRecordData(e.eventPosition);
        intfc->WriteRecordData(e.eventRotation);
        WriteString(intfc, e.eventCellName);
        intfc->WriteRecordData(e.locationFormID);
        WriteString(intfc, e.locationPluginName);
    }
}

void SaveData::Load(SKSE::SerializationInterface* intfc) {
    locationEvents.clear();
    std::uint32_t count;
    intfc->ReadRecordData(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        LocationEvent e;
        ReadString(intfc, e.locationName);
        std::uint32_t type;
        intfc->ReadRecordData(type);
        e.eventType = static_cast<LocationEventType>(type);
        intfc->ReadRecordData(e.eventTime);
        intfc->ReadRecordData(e.eventPosition);
        intfc->ReadRecordData(e.eventRotation);
        ReadString(intfc, e.eventCellName);
        intfc->ReadRecordData(e.locationFormID);
        ReadString(intfc, e.locationPluginName);
        locationEvents[e.locationName] = std::move(e);
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

/*
[[nodiscard]] FormID GetLocalFormID() {
    auto file = GetFile(0);

    RE::FormID fileIndex = file->compileIndex << (3 * 8);
    fileIndex += file->smallFileCompileIndex << ((1 * 8) + 4);

    return formID & ~fileIndex;
}
*/

const RE::FormID GetLocalFormID(const RE::BGSLocation* location) {
    if (!location) return 0;

    auto file = location->GetFile(0);
    if (!file) return 0;

    RE::FormID fileIndex = file->compileIndex << (3 * 8);
    fileIndex += file->smallFileCompileIndex << ((1 * 8) + 4);

    return location->GetFormID() & ~fileIndex;
}

void SaveLocationEvent(LocationEventType type, const RE::BGSLocation* location) {
    auto* playerCharacter = RE::PlayerCharacter::GetSingleton();
    auto* currentLocation = playerCharacter->GetCurrentLocation();
    auto  locationName    = std::string{location->GetFullName()};
    auto& saveData        = GetSaveData();

    auto existing = saveData.locationEvents.find(locationName);
    if (existing != saveData.locationEvents.end()) {
        // If it is, update the time...
        existing->second.eventTime = RE::Calendar::GetSingleton()->GetCurrentGameTime();
        // And if the new event type is "Cleared", update the event type to "Cleared" if it was previously "Discovered"
        if (existing->second.eventType == LocationEventType::Discovered && type == LocationEventType::Cleared) existing->second.eventType = LocationEventType::Cleared;
        Log("[Save] [UpdateLocationEvent] {} - {} - {}", existing->second.locationName, LocationEventTypeToString(existing->second.eventType), existing->second.eventCellName);
        return;
    }

    auto& addedLocationEvent = saveData.locationEvents[locationName] = LocationEvent{
        locationName,
        type,
        RE::Calendar::GetSingleton()->GetCurrentGameTime(),
        playerCharacter->GetPosition(),
        playerCharacter->GetAngle(),
        currentLocation ? currentLocation->GetFullName() : "<Unknown Location>",
        location ? GetLocalFormID(location) : 0,
        location ? location->GetFile(0)->GetFilename().data() : "",
    };
    Log("[Save] [AddLocationEvent] {} - {} - {}", addedLocationEvent.locationName, LocationEventTypeToString(addedLocationEvent.eventType), addedLocationEvent.eventCellName);
}

void SaveLocationDiscoveredEvent(const RE::BGSLocation* location) { SaveLocationEvent(LocationEventType::Discovered, location); }
void SaveLocationClearedEvent(const RE::BGSLocation* location) { SaveLocationEvent(LocationEventType::Cleared, location); }
