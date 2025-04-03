#pragma once

#include "EventHandler.h"

#include <SkyrimScripting/Logging.h>

#include <format>

#include "DiscoveredLocations.h"
#include "IniFile.h"
#include "JournalManager.h"
#include "SaveData.h"
#include "SillyMessages.h"

std::chrono::steady_clock::time_point lastJournalUpdate = std::chrono::steady_clock::now();

auto dontUpdateTheJournalMultipleTimesWithinMs = 100;

void EventHandler::UpdateJournalWithLatestStats(bool showSillyMessage) {
    auto now = std::chrono::steady_clock::now();
    if (now - lastJournalUpdate < std::chrono::milliseconds(dontUpdateTheJournalMultipleTimesWithinMs)) {
        Log("Skipping journal update to avoid multiple updates within {} ms", dontUpdateTheJournalMultipleTimesWithinMs);
        return;
    }
    lastJournalUpdate = now;

    Log("Updating journal with latest stats...");

    auto displayedLocationStates = GetDiscoveredLocationStats();

    auto& saveData = GetSaveData();

    auto objectiveId = saveData.locationEvents.size();

    auto discoveredCount = displayedLocationStates.discoveredLocations;
    if (saveData.locationEvents.size() > discoveredCount) discoveredCount = saveData.locationEvents.size();

    Log("Adding the 'discovered locations' objective...");
    JournalManager::UpdateObjectiveText(objectiveId, std::format("{} discovered locations out of {}", discoveredCount, displayedLocationStates.totalLocations).c_str());
    JournalManager::UpdateObjectiveText(objectiveId, std::format("{} discovered locations out of {}", discoveredCount, displayedLocationStates.totalLocations).c_str());
    JournalManager::SetStatus(objectiveId, true, false, true);

    for (auto& locationEvent : saveData.locationEvents) {
        objectiveId--;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        switch (locationEvent.eventType) {
            case LocationEventType::Cleared:
                Log("Cleared location: {}", locationEvent.locationName);
                JournalManager::UpdateObjectiveText(objectiveId, std::format("Cleared location: {}", locationEvent.locationName).c_str());
                JournalManager::SetStatus(objectiveId, true, true, false);
                // JournalManager::SetStatus(objectiveId, true, true, false);
                break;
            default:
                Log("Discovered location: {}", locationEvent.locationName);
                JournalManager::UpdateObjectiveText(objectiveId, std::format("Discovered location: {}", locationEvent.locationName).c_str());
                // JournalManager::SetStatus(objectiveId, true, true, false);
                JournalManager::SetStatus(objectiveId, true, true, false);
                break;
        }
    }

    if (!showSillyMessage) {
        Log("Skipping silly message display as requested.");
        return;
    }

    Log("Checking for silly messages...");
    if (GetIni().percentage_based_message_in_journal) {
        auto percentageDiscovered = static_cast<float>(displayedLocationStates.discoveredLocations) / displayedLocationStates.totalLocations * 100.0f;
        auto integerPercentage    = static_cast<int>(std::floor(percentageDiscovered));
        auto percentageMessage    = SillyMessages::instance().GetRandomMessage_PercentageDiscovered(percentageDiscovered);
        if (!percentageMessage.empty()) {
            Log("Percentage discovered: {}", percentageDiscovered);
            Log("Message: {}", percentageMessage);
            JournalManager::UpdateObjectiveText(1, percentageMessage.c_str());
            JournalManager::SetStatus(1, true, false, true);
            // JournalManager::SetStatus(1, true, false);
        } else {
            Log("No message found for percentage discovered: {}", percentageDiscovered);
            JournalManager::SetStatus(1, false, false);
        }
    }
    Log("Journal updated with latest stats.");
}

void EventHandler::OnLocationDiscovered(const RE::MapMarkerData* mapMarkerData) {
    SaveLocationDiscoveredEvent(mapMarkerData->locationName.GetFullName());
    UpdateJournalWithLatestStats(true);

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

        if (!message.empty()) {
            Log("Silly Message: {} - {}", locationName, message);
            RE::DebugNotification(message.c_str());
        } else {
            Log("No message found for discovered location: {}", locationName);
        }
    }
}

void EventHandler::OnLocationCleared(const BGSLocationEx* locationEx) {
    SaveLocationClearedEvent(locationEx->GetFullName());
    UpdateJournalWithLatestStats(true);

    if (GetIni().notification_on_location_cleared) {
        auto message = SillyMessages::instance().GetRandomMessage_LocationCleared(locationEx->GetFullName());
        if (!message.empty()) {
            Log("Silly Message, Location cleared: {} - {}", locationEx->GetFullName(), message);
            RE::DebugNotification(message.c_str());
        } else {
            Log("No message found for cleared location: {}", locationEx->GetFullName());
        }
    }
}
