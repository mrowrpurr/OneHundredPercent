#pragma once

#include "EventHandler.h"

#include <SkyrimScripting/Logging.h>

#include <format>

#include "DiscoverableLocations.h"
#include "IniFile.h"
#include "JournalManager.h"
#include "SaveData.h"
#include "SillyMessages.h"

std::chrono::steady_clock::time_point lastJournalUpdate = std::chrono::steady_clock::now();

auto dontUpdateTheJournalMultipleTimesWithinMs = 100;

void EventHandler::UpdateJournalWithLatestStats(std::optional<std::string> sillyMessage) {
    // auto now = std::chrono::steady_clock::now();
    // if (now - lastJournalUpdate < std::chrono::milliseconds(dontUpdateTheJournalMultipleTimesWithinMs)) {
    //     Log("Skipping journal update to avoid multiple updates within {} ms", dontUpdateTheJournalMultipleTimesWithinMs);
    //     return;
    // }
    // lastJournalUpdate = now;

    // if (!GetIni().show_message_for_most_recent_location_in_journal) sillyMessage = std::nullopt;

    // Log("Updating journal with latest stats...");

    // auto& saveData                = GetSaveData();
    // auto  displayedLocationStates = GetDiscoverableLocationInfo();
    // auto  discoveredCount         = displayedLocationStates.DiscoverableLocations;

    // Log("Checking for silly messages...");
    // std::string percentageSillyMessage;
    // if (GetIni().show_silly_message_in_journal) {
    //     auto percentageDiscovered = static_cast<float>(displayedLocationStates.DiscoverableLocations) / displayedLocationStates.totalLocations * 100.0f;
    //     auto integerPercentage    = static_cast<int>(std::floor(percentageDiscovered));
    //     auto percentageMessage    = SillyMessages::instance().GetRandomMessage_PercentageDiscovered(percentageDiscovered);
    //     if (!percentageMessage.empty()) {
    //         percentageSillyMessage = percentageMessage;
    //         Log("Percentage discovered: {} - {}", percentageDiscovered, percentageMessage);
    //         // JournalManager::UpdateObjectiveText(1, percentageMessage.c_str());
    //         // JournalManager::SetStatus(1, true, false, true);
    //         // JournalManager::SetStatus(1, true, false);
    //     } else {
    //         Log("No message found for percentage discovered: {}", percentageDiscovered);
    //         // JournalManager::SetStatus(1, false, false);
    //     }
    // }

    // Log("Journal updated with latest stats.");
    // if (saveData.locationEvents.size() > discoveredCount) discoveredCount = saveData.locationEvents.size();

    // auto objectiveId = GetIni().show_recent_locations_in_journal ? saveData.locationEvents.size() : 0;
    // if (GetIni().show_message_for_most_recent_location_in_journal)
    //     if (sillyMessage && !sillyMessage.value().empty()) objectiveId++;
    // if (GetIni().show_recent_locations_in_journal)
    //     if (!percentageSillyMessage.empty()) objectiveId++;

    // Log("Adding the 'discovered locations' objective...");
    // auto percentageFloat              = static_cast<float>(discoveredCount) / displayedLocationStates.totalLocations * 100.0f;
    // auto percentageStringToOneDecimal = std::format("{:.1f}", percentageFloat);
    // JournalManager::UpdateObjectiveText(
    //     objectiveId, std::format("{} discovered locations out of {} ({}%)", discoveredCount, displayedLocationStates.totalLocations, percentageStringToOneDecimal).c_str()
    // );
    // JournalManager::SetStatus(objectiveId, true, false, true);  // <---- THIS NEEDS TO BE BASED ON CONFIG!

    // if (!percentageSillyMessage.empty()) {
    //     objectiveId--;
    //     Log("Adding PERCENTAGE silly message to journal: {}", percentageSillyMessage);
    //     JournalManager::UpdateObjectiveText(objectiveId, percentageSillyMessage.c_str());
    //     JournalManager::SetStatus(objectiveId, true, false, sillyMessage && sillyMessage.value().empty());
    // }

    // if (sillyMessage != std::nullopt) {
    //     objectiveId--;
    //     Log("Adding silly message to journal: {}", sillyMessage.value());
    //     JournalManager::UpdateObjectiveText(objectiveId, sillyMessage.value().c_str());
    //     JournalManager::SetStatus(objectiveId, true, false, true);
    // }

    // if (!GetIni().show_recent_locations_in_journal) {
    //     Log("Not adding recent locations to journal.");
    //     return;
    // }
    // for (auto& locationEvent : saveData.locationEvents) {
    //     objectiveId--;
    //     std::this_thread::sleep_for(std::chrono::milliseconds(20));
    //     switch (locationEvent.eventType) {
    //         case LocationEventType::Cleared:
    //             Log("Cleared location: {}", locationEvent.locationName);
    //             JournalManager::UpdateObjectiveText(objectiveId, std::format("Cleared location: {}", locationEvent.locationName).c_str());
    //             JournalManager::SetStatus(objectiveId, true, true, false);
    //             break;
    //         default:
    //             Log("Discovered location: {}", locationEvent.locationName);
    //             JournalManager::UpdateObjectiveText(objectiveId, std::format("Discovered location: {}", locationEvent.locationName).c_str());
    //             JournalManager::SetStatus(objectiveId, true, true, false);
    //             break;
    //     }
    // }
}

void EventHandler::OnLocationDiscovered(const RE::BGSLocation* location) {
    SaveLocationDiscoveredEvent(location);

    // TODO: EXTRACT THIS SILLY MESSAGE STUFF :)
    std::string sillyMessage;
    Log("... in silly message ...");
    if (GetIni().message_on_location_discovered) {
        auto locationName = std::string{location->GetName()};
        Log("...Discovered location: {}", locationName);

        // First try to get a specific message for this location
        if (SillyMessages::instance().HasSpecificLocationMessage(locationName)) {
            sillyMessage = SillyMessages::instance().GetRandomSpecificLocationMessage(locationName);
        } else {
            // Fall back to generic location discovered message if no specific one exists
            sillyMessage = SillyMessages::instance().GetRandomMessage_LocationDiscovered(locationName);
        }

        if (!sillyMessage.empty()) {
            Log("Silly Message: {} - {}", locationName, sillyMessage);
            RE::DebugNotification(sillyMessage.c_str());
        } else {
            Log("No message found for discovered location: {}", locationName);
        }
    }

    // TODO
    // UpdateJournalWithLatestStats(sillyMessage.empty() ? std::nullopt : std::make_optional(sillyMessage));
}

void EventHandler::OnLocationCleared(const RE::BGSLocation* location) {
    SaveLocationClearedEvent(location);

    // TODO: EXTRACT THIS SILLY MESSAGE STUFF :)
    std::string sillyMessage;
    if (GetIni().message_on_location_cleared) {
        const auto* locationName = location->GetName();

        sillyMessage = SillyMessages::instance().GetRandomMessage_LocationCleared(locationName);
        if (!sillyMessage.empty()) {
            Log("Silly Message, Location cleared: {} - {}", locationName, sillyMessage);
            RE::DebugNotification(sillyMessage.c_str());
        } else {
            Log("No message found for cleared location: {}", locationName);
        }
    }

    // TODO
    // UpdateJournalWithLatestStats(sillyMessage.empty() ? std::nullopt : std::make_optional(sillyMessage));
}
