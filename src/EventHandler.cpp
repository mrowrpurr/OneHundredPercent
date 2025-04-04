#pragma once

#include "EventHandler.h"

#include <SkyrimScripting/Logging.h>

#include <algorithm>
#include <format>

#include "DiscoverableLocations.h"
#include "JournalManager.h"
#include "PlayerDiscoveredLocations.h"
#include "SaveData.h"
#include "SillyMessages.h"
#include "TomlFile.h"

std::chrono::steady_clock::time_point lastJournalUpdate = std::chrono::steady_clock::now();

auto dontUpdateTheJournalMultipleTimesWithinMs = 100;

void SendFormattedDebugNotificationMessage(std::string_view text, std::string_view color) {
    if (GetConfig().enable_on_screen_messages == false) {
        Log("On-screen messages are disabled.");
        return;
    }
    if (color.empty()) {
        RE::DebugNotification(text.data());
    } else {
        if (color[0] == '#') color.remove_prefix(1);
        RE::DebugNotification(std::format("<font color='#{}'>{}</font>", color, text.data()).c_str());
    }
}

std::string GetSillyMessage_OnLocationDiscovered(std::string_view locationName) {
    std::string sillyMessage;
    if (GetConfig().message_on_location_discovered) {
        if (SillyMessages::instance().HasSpecificLocationMessage(locationName.data())) sillyMessage = SillyMessages::instance().GetRandomSpecificLocationMessage(locationName);
        else sillyMessage = SillyMessages::instance().GetRandomMessage_LocationDiscovered(locationName);
        if (!sillyMessage.empty()) Log("[Message] Location discovered: {} - {}", locationName, sillyMessage);
        else Log("No message found for discovered location: {}", locationName);
    }
    return sillyMessage;
}

std::string GetSillyMessage_OnLocationCleared(std::string_view locationName) {
    std::string sillyMessage;
    if (GetConfig().message_on_location_cleared) {
        sillyMessage = SillyMessages::instance().GetRandomMessage_LocationCleared(locationName);
        if (!sillyMessage.empty()) Log("[Message] Location cleared: {} - {}", locationName, sillyMessage);
        else Log("No message found for cleared location: {}", locationName);
    }
    return sillyMessage;
}

enum class JournalEntryType {
    PercentageDiscovered,
    PercentageDiscoveredSillyMessage,
    MostRecentLocationSillyMessage,
    RecentLocation,
};

struct JournalEntry {
    JournalEntryType type;
    std::string      text;
};

/*
    [Journal]
    enable_journal = true
    show_percentage_in_journal = true
    show_message_for_percentage_in_journal = true
    show_recent_locations_in_journal = true
    show_message_for_most_recent_location_in_journal = true
*/

void EventHandler::UpdateJournalWithLatestStats(std::string_view sillyMessage) {
    if (GetConfig().enable_journal == false) {
        Log("Journal updates are disabled.");
        return;
    }

    auto now = std::chrono::steady_clock::now();
    if (now - lastJournalUpdate < std::chrono::milliseconds(dontUpdateTheJournalMultipleTimesWithinMs)) {
        Log("Skipping journal update to avoid multiple updates within {} ms", dontUpdateTheJournalMultipleTimesWithinMs);
        return;
    }
    lastJournalUpdate = now;

    // Get percentage discovered
    auto totalDiscoverableLocations = GetDiscoverableLocationInfo()->totalDiscoverableLocationCount;
    auto discoveredLocations        = GetNumberOfPlayerDiscoveredLocations();
    auto percentageDiscovered       = static_cast<float>(discoveredLocations) / totalDiscoverableLocations * 100.0f;
    auto integerPercentage          = static_cast<int>(std::floor(percentageDiscovered));

    std::vector<JournalEntry> journalEntries;

    // For now, the order of these tyes of messages is fixed.

    // 1. Percentage discovered
    if (GetConfig().show_percentage_in_journal) {
        auto percentageMessage = std::format("{} discovered locations out of {} ({}%)", discoveredLocations, totalDiscoverableLocations, integerPercentage);
        journalEntries.push_back({JournalEntryType::PercentageDiscovered, percentageMessage});
        Log("[Journal] Percentage discovered: {} - {}", percentageDiscovered, percentageMessage);
    }

    // 2. Silly message (for the percentage discovered)
    if (GetConfig().show_message_for_percentage_in_journal) {
        auto percentageMessage = SillyMessages::instance().GetRandomMessage_PercentageDiscovered(percentageDiscovered);
        if (!percentageMessage.empty()) {
            journalEntries.push_back({JournalEntryType::PercentageDiscoveredSillyMessage, percentageMessage});
            Log("[Journal] [Message] Percentage discovered: {} - {}", percentageDiscovered, percentageMessage);
        } else {
            Log("[Journal] No message found for percentage discovered: {}", percentageDiscovered);
        }
    }

    auto& saveData = GetSaveData();

    // 3. Silly message (for the most recent location)
    if (GetConfig().show_message_for_most_recent_location_in_journal && !saveData.locationEvents.empty()) {
        auto& mostRecentLocationEvent = saveData.locationEvents.back();
        switch (mostRecentLocationEvent.eventType) {
            case LocationEventType::Cleared: {
                Log("[Journal] Most recent location: Cleared location: {}", mostRecentLocationEvent.locationName);
                auto sillyRecentLocationMessage = SillyMessages::instance().GetRandomMessage_LocationCleared(mostRecentLocationEvent.locationName);
                if (!sillyRecentLocationMessage.empty()) {
                    journalEntries.push_back({JournalEntryType::MostRecentLocationSillyMessage, sillyRecentLocationMessage});
                    Log("[Journal] [Message] Most recent location: Cleared location: {} - {}", mostRecentLocationEvent.locationName, sillyRecentLocationMessage);
                } else {
                    Log("[Journal] No message found for most recent location: {}", mostRecentLocationEvent.locationName);
                }
                break;
            }
            default: {
                Log("[Journal] Most recent location: Discovered location: {}", mostRecentLocationEvent.locationName);
                auto sillyRecentLocationMessage = SillyMessages::instance().GetRandomMessage_LocationDiscovered(mostRecentLocationEvent.locationName);
                if (!sillyRecentLocationMessage.empty()) {
                    journalEntries.push_back({JournalEntryType::MostRecentLocationSillyMessage, sillyRecentLocationMessage});
                    Log("[Journal] [Message] Most recent location: Discovered location: {} - {}", mostRecentLocationEvent.locationName, sillyRecentLocationMessage);
                } else {
                    Log("[Journal] No message found for most recent location: {}", mostRecentLocationEvent.locationName);
                }
                break;
            }
        }
    }

    // 4. Discovered locations
    if (GetConfig().show_recent_locations_in_journal && GetConfig().max_recent_locations_in_journal > 0) {
        auto recentLocationCount         = 0;
        auto maxLocations                = std::min(static_cast<std::uint32_t>(saveData.locationEvents.size()), GetConfig().max_recent_locations_in_journal);
        auto numberOfDiscoveredLocations = saveData.locationEvents.size();
        recentLocationCount              = maxLocations;
        for (auto i = numberOfDiscoveredLocations; i > numberOfDiscoveredLocations - maxLocations; --i) {
            auto&       locationEvent = saveData.locationEvents[i - 1];
            std::string locationMessage;
            switch (locationEvent.eventType) {
                case LocationEventType::Cleared:
                    locationMessage = std::format("Cleared location: {}", locationEvent.locationName);
                    break;
                default:
                    locationMessage = std::format("Discovered location: {}", locationEvent.locationName);
                    break;
            }
            journalEntries.push_back({JournalEntryType::RecentLocation, locationMessage});
            Log("[Journal] {}", locationMessage);
        }
    }

    // Ok, now we gotta show the things!
    for (auto i = 0; i < journalEntries.size(); ++i) {
        auto  objectiveId  = i;
        auto  journalIndex = journalEntries.size() - 1 - i;
        auto& entry        = journalEntries[journalIndex];
        JournalManager::UpdateObjectiveText(i, entry.text.c_str());
        switch (entry.type) {
            case JournalEntryType::PercentageDiscovered:
                JournalManager::SetStatus(objectiveId, true, false, true);
                break;
            case JournalEntryType::PercentageDiscoveredSillyMessage:
                JournalManager::SetStatus(objectiveId, true, false, false);
                break;
            case JournalEntryType::MostRecentLocationSillyMessage:
                JournalManager::SetStatus(objectiveId, true, false, false);
                break;
            case JournalEntryType::RecentLocation:
                JournalManager::SetStatus(objectiveId, true, true, false);
                break;
        }
    }

    Log("> Here are the journal entries to be added:");
    for (const auto& message : journalEntries) {
        Log("> [Journal] {}", message.text);
    }

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastJournalUpdate).count();
    Log("Journal update took {} ms", durationMs);
}

void EventHandler::OnLocationDiscovered(const RE::BGSLocation* location) {
    SaveLocationDiscoveredEvent(location);
    auto sillyMessage = GetSillyMessage_OnLocationDiscovered(location->GetName());
    SendFormattedDebugNotificationMessage(sillyMessage, GetConfig().color_on_location_discovered);
    UpdateJournalWithLatestStats(sillyMessage);
}

void EventHandler::OnLocationCleared(const RE::BGSLocation* location) {
    SaveLocationClearedEvent(location);
    auto sillyMessage = GetSillyMessage_OnLocationCleared(location->GetName());
    SendFormattedDebugNotificationMessage(sillyMessage, GetConfig().color_on_location_cleared);
    UpdateJournalWithLatestStats(sillyMessage);
}

// if (!GetConfig().show_message_for_most_recent_location_in_journal) sillyMessage = std::nullopt;

// Log("Updating journal with latest stats...");

// auto& saveData                = GetSaveData();
// auto  displayedLocationStates = GetDiscoverableLocationInfo();
// auto  discoveredCount         = displayedLocationStates.DiscoverableLocations;

// Log("Checking for silly messages...");
// std::string percentageSillyMessage;
// if (GetConfig().show_message_for_percentage_in_journal) {
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

// auto objectiveId = GetConfig().show_recent_locations_in_journal ? saveData.locationEvents.size() : 0;
// if (GetConfig().show_message_for_most_recent_location_in_journal)
//     if (sillyMessage && !sillyMessage.value().empty()) objectiveId++;
// if (GetConfig().show_recent_locations_in_journal)
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

// if (!GetConfig().show_recent_locations_in_journal) {
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