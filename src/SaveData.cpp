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

    for (const auto& e : locationEvents) {
        WriteString(intfc, e.locationName);
        intfc->WriteRecordData(static_cast<std::uint32_t>(e.eventType));
        intfc->WriteRecordData(e.eventTime);
        intfc->WriteRecordData(e.eventPosition);
        intfc->WriteRecordData(e.eventRotation);
        WriteString(intfc, e.eventCellName);
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
        locationEvents.push_back(std::move(e));
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

void SaveLocationEvent(LocationEventType type, std::string_view locationName) {
    auto* playerCharacter = RE::PlayerCharacter::GetSingleton();
    auto* currentLocation = playerCharacter->GetCurrentLocation();

    GetSaveData().locationEvents.emplace_back(
        std::string(locationName), type, RE::Calendar::GetSingleton()->GetCurrentGameTime(), playerCharacter->GetPosition(), playerCharacter->GetAngle(),
        currentLocation ? currentLocation->GetFullName() : "<Unknown Location>"
    );
}

void SaveLocationDiscoveredEvent(std::string_view locationName) { SaveLocationEvent(LocationEventType::Discovered, locationName); }
void SaveLocationClearedEvent(std::string_view locationName) { SaveLocationEvent(LocationEventType::Cleared, locationName); }