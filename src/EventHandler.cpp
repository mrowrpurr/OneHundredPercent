#pragma once

#include "EventHandler.h"

#include <SkyrimScripting/Logging.h>

#include <algorithm>
#include <format>

#include "DiscoverableMapMarkers.h"
#include "JournalManager.h"
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
    if (GetConfig().on_location_discovered_notification) {
        if (SillyMessages::instance().HasSpecificLocationMessage(locationName.data())) sillyMessage = SillyMessages::instance().GetRandomSpecificLocationMessage(locationName);
        else sillyMessage = SillyMessages::instance().GetRandomMessage_LocationDiscovered(locationName);
        if (!sillyMessage.empty()) Log("[Message] Location discovered: {} - {}", locationName, sillyMessage);
        else Log("No message found for discovered location: {}", locationName);
    }
    return sillyMessage;
}

std::string GetSillyMessage_OnLocationCleared(std::string_view locationName) {
    std::string sillyMessage;
    if (GetConfig().on_location_cleared_notification) {
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

void EventHandler::UpdateJournalWithLatestStats() {
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

    auto& saveData = GetSaveData();

    // Get percentage discovered
    auto totalDiscoverableMapMarkers = GetDiscoverableMapMarkers()->GetTotalDiscoverableMapMarkersCount();
    auto recentlyDiscoveredMarkers   = saveData.GetTotalDiscoveredMapMarkersCount();
    auto percentageDiscovered        = static_cast<float>(recentlyDiscoveredMarkers) / totalDiscoverableMapMarkers * 100.0f;
    // auto        integerPercentage              = static_cast<int>(std::floor(percentageDiscovered));

    std::vector<JournalEntry> journalEntries;

    // For now, the order of these tyes of messages is fixed.

    // 1. Percentage discovered
    if (GetConfig().show_percentage_in_journal) {
        auto percentageMessage = std::format("{} discovered locations out of {} ({:.1f}%)", recentlyDiscoveredMarkers, totalDiscoverableMapMarkers, percentageDiscovered);
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

    // 3. Silly message (for the most recent location)
    if (GetConfig().show_message_for_most_recent_location_in_journal) {
        if (auto* mostRecentLocationEvent = saveData.GetMostRecentlyDiscoveredLocation()) {
            Log("[Journal] Most recent location: Discovered location: {}", mostRecentLocationEvent->locationName);
            std::string sillyRecentLocationMessage;
            switch (mostRecentLocationEvent->eventType) {
                case LocationEventType::Cleared:
                    sillyRecentLocationMessage = SillyMessages::instance().GetRandomMessage_LocationCleared(mostRecentLocationEvent->locationName);
                    break;
                default:
                    sillyRecentLocationMessage = SillyMessages::instance().GetRandomMessage_LocationDiscovered(mostRecentLocationEvent->locationName);
                    break;
            }
            if (!sillyRecentLocationMessage.empty()) {
                journalEntries.push_back({JournalEntryType::MostRecentLocationSillyMessage, sillyRecentLocationMessage});
                Log("[Journal] [Message] Most recent location: Discovered location: {} - {}", mostRecentLocationEvent->locationName, sillyRecentLocationMessage);
            } else {
                Log("[Journal] No message found for most recent location: {}", mostRecentLocationEvent->locationName);
            }
        }
    }

    // 4. Discovered locations
    if (GetConfig().show_recent_locations_in_journal && GetConfig().max_recent_locations_in_journal > 0) {
        auto recentLocationCount               = 0;
        auto maxLocations                      = std::min(saveData.GetRecentlyDiscoveredMapMarkersCount(), GetConfig().max_recent_locations_in_journal);
        auto numberOfrecentlyDiscoveredMarkers = saveData.GetRecentlyDiscoveredMapMarkersCount();
        recentLocationCount                    = maxLocations;
        for (auto i = numberOfrecentlyDiscoveredMarkers; i > numberOfrecentlyDiscoveredMarkers - maxLocations; --i) {
            if (auto* locationEvent = saveData.GetRecentlyDiscoveredLocation(i - 1)) {
                Log("Journal (LocationEvent) {:x} @ {} - {}", locationEvent->formIdentifier.localFormID, locationEvent->formIdentifier.pluginName, locationEvent->locationName);

                std::string locationMessage;
                switch (locationEvent->eventType) {
                    case LocationEventType::Cleared:
                        locationMessage = std::format("Cleared location: {}", locationEvent->locationName);
                        break;
                    default:
                        locationMessage = std::format("Discovered location: {}", locationEvent->locationName);
                        break;
                }
                journalEntries.push_back({JournalEntryType::RecentLocation, locationMessage});
                Log("[Journal] {}", locationMessage);
            } else {
                SKSE::log::error("[Journal] [Error] Failed to get recently discovered location: {}", i);
                break;
            }
        }
    }

    Log("> Here are the journal entries to be added:");
    for (const auto& message : journalEntries) Log("> [Journal] {}", message.text);

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

    // Now keep going up and find journal entries and check if they are empty text or not
    // and if they're not empty text, empty the text and mark as inactive/not visible:
    auto objectiveId = journalEntries.size();
    while (journalEntries.size() < GetConfig().max_recent_locations_in_journal) {
        Log("Checking for objectiveId {} ...", objectiveId);
        if (JournalManager::IsObjectiveVisible(objectiveId)) {
            JournalManager::ClearObjective(objectiveId);
            objectiveId++;
        } else {
            break;
        }
    }

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastJournalUpdate).count();
    Log("Journal update took {} ms", durationMs);
}

void EventHandler::OnMapMarkerDiscovered(const RE::MapMarkerData* mapMarkerData) {
    GetSaveData().SaveDiscoveryEvent(LocationEventType::Discovered, mapMarkerData);

    if (GetConfig().enable_on_screen_messages) {
        auto sillyMessage = GetSillyMessage_OnLocationDiscovered(mapMarkerData->locationName.GetFullName());
        SendFormattedDebugNotificationMessage(sillyMessage, GetConfig().on_location_discovered_notification_color);
    }

    if (GetConfig().enable_journal) UpdateJournalWithLatestStats();
}

void EventHandler::OnMapMarkerCleared(const RE::MapMarkerData* mapMarkerData) {
    GetSaveData().SaveDiscoveryEvent(LocationEventType::Cleared, mapMarkerData);

    if (GetConfig().enable_on_screen_messages) {
        auto sillyMessage = GetSillyMessage_OnLocationCleared(mapMarkerData->locationName.GetFullName());
        SendFormattedDebugNotificationMessage(sillyMessage, GetConfig().on_location_cleared_notification_color);
    }

    if (GetConfig().enable_journal) UpdateJournalWithLatestStats();
}
