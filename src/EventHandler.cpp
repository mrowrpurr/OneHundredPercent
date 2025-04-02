#pragma once

#include "EventHandler.h"

#include <format>

#include "DiscoveredLocations.h"
#include "JournalManager.h"

void EventHandler::UpdateJournalWithLatestStats() {
    auto displayedLocationStates = GetDiscoveredLocationStats();
    JournalManager::UpdateObjectiveText(
        0, std::format("{} discovered locations out of {}", displayedLocationStates.discoveredLocations, displayedLocationStates.totalLocations).c_str()
    );
    JournalManager::SetStatus(0, true, false);
}

void EventHandler::OnLocationDiscovered(const RE::MapMarkerData* mapMarkerData) { UpdateJournalWithLatestStats(); }
void EventHandler::OnLocationCleared(const BGSLocationEx* locationEx) { UpdateJournalWithLatestStats(); }
void EventHandler::OnOpenJournal() { UpdateJournalWithLatestStats(); }
