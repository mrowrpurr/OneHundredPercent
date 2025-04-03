#pragma once

#include "EventHandler.h"

#include <format>

#include "DiscoveredLocations.h"
#include "IniFile.h"
#include "JournalManager.h"
#include "SaveData.h"
#include "SillyMessages.h"

void EventHandler::UpdateJournalWithLatestStats() {
    auto displayedLocationStates = GetDiscoveredLocationStats();

    JournalManager::UpdateObjectiveText(
        0, std::format("{} discovered locations out of {}", displayedLocationStates.discoveredLocations, displayedLocationStates.totalLocations).c_str()
    );
    JournalManager::SetStatus(0, true, false);

    if (GetIni().percentage_based_message_in_journal) {
        auto percentageDiscovered = static_cast<float>(displayedLocationStates.discoveredLocations) / displayedLocationStates.totalLocations * 100.0f;
        auto integerPercentage    = static_cast<int>(std::floor(percentageDiscovered));
        auto percentageMessage    = SillyMessages::instance().GetRandomMessage_PercentageDiscovered(percentageDiscovered);
        if (!percentageMessage.empty()) {
            JournalManager::UpdateObjectiveText(1, percentageMessage.c_str());
            JournalManager::SetStatus(1, true, false);
        } else {
            JournalManager::SetStatus(1, false, false);
        }
    } else {
        JournalManager::SetStatus(1, false, false);
    }
}

void EventHandler::OnLocationDiscovered(const RE::MapMarkerData* mapMarkerData) {
    SaveLocationDiscoveredEvent(mapMarkerData->locationName.GetFullName());
    UpdateJournalWithLatestStats();

    if (GetIni().notification_on_location_discovered) {
        std::string locationName = mapMarkerData->locationName.GetFullName();
        std::string message;
        
        // First try to get a specific message for this location
        if (SillyMessages::instance().HasSpecificLocationMessage(locationName)) {
            message = SillyMessages::instance().GetRandomSpecificLocationMessage(locationName);
        } else {
            // Fall back to generic location discovered message if no specific one exists
            message = SillyMessages::instance().GetRandomMessage_LocationDiscovered(locationName);
        }
        
        if (!message.empty()) RE::DebugNotification(message.c_str());
    }
}

void EventHandler::OnLocationCleared(const BGSLocationEx* locationEx) {
    SaveLocationClearedEvent(locationEx->GetFullName());
    UpdateJournalWithLatestStats();

    if (GetIni().notification_on_location_cleared) {
        auto message = SillyMessages::instance().GetRandomMessage_LocationCleared(locationEx->GetFullName());
        if (!message.empty()) RE::DebugNotification(message.c_str());
    }
}

void EventHandler::OnOpenJournal() { UpdateJournalWithLatestStats(); }
