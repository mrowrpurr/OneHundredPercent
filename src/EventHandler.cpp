#pragma once

#include "EventHandler.h"

#include <format>

#include "DiscoveredLocations.h"
#include "JournalManager.h"

void EventHandler::OnLocationDiscovered(const RE::MapMarkerData* mapMarkerData) {
    //
}

void EventHandler::OnLocationCleared(const BGSLocationEx* locationEx) {
    //
}

void EventHandler::OnOpenJournal() {
    auto displayedLocationStates = GetDiscoveredLocationStats();
    JournalManager::UpdateObjectiveText(0, std::format("{} / {}", displayedLocationStates.discoveredLocations, displayedLocationStates.totalLocations).c_str());
    JournalManager::SetStatus(0, true, false);
}
