
void RecalculateTotalNumberOfDiscoveredLocationsWithMapMarkers() {
    auto now = std::chrono::steady_clock::now();
    Log("Recalculating total number of discovered locations with map markers...");
    auto  countOfDiscoveredLocations = 0;
    auto* player                     = RE::PlayerCharacter::GetSingleton();
    for (auto& markerPtr : player->currentMapMarkers) {
        if (auto marker = markerPtr.get()) {
            if (const auto* extraMapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
                if (auto* mapData = extraMapMarker->mapData) {
                    Log("{}", mapData->locationName.GetFullName());
                    if (mapData->flags.any(RE::MapMarkerData::Flag::kVisible) && mapData->flags.any(RE::MapMarkerData::Flag::kCanTravelTo)) {
                        countOfDiscoveredLocations++;
                    }
                }
            }
        }
    }

    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::steady_clock::now()).count();
}
auto* clearableKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypeClearable");
if (clearableKeyword) {
    Log("Found keyword: {} {}", clearableKeyword->GetName(), clearableKeyword->GetFormID());

    auto allLocationsInGame = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSLocation>();
    for (const auto* location : allLocationsInGame) {
        if (location->fullName.empty()) continue;
        if (location->worldLocMarker && location->worldLocMarker.get()) {
            if (location->HasKeyword(clearableKeyword)) {
                Log("CLEARABLE location: {}", location->GetName());
                // } else {
                //     Log("not clearable: {}", location->GetName());
            }
        }
    }
} else {
    Log("Keyword not found: LocTypeClearable");
}