#include <SkyrimScripting/Plugin.h>
#include <collections.h>

#include <nlohmann/json.hpp>

constexpr auto QUEST_EDITOR_ID = "MP_HazTheCompletionizt";

const std::filesystem::path JSON_FILES_FOLDER = "Data/SKSE/Plugins/HazTheCompletionizt";

using MessageCollection = collections_map<std::string, std::vector<std::string>>;

struct SillyMessages {
    MessageCollection PercentageDiscoveredMessages;
    MessageCollection OnSpecificLocationDiscovered;
    MessageCollection OnMatchingLocationDiscovered;
    MessageCollection OnSpecificLocationCleared;
    MessageCollection OnMatchingLocationCleared;
};

SillyMessages TheSillyMessages;

void LoadSillyMessagesFromJsonFile(std::filesystem::path jsonFilePath) {
    try {
        Log("Loading JSON file: {}", jsonFilePath.string());

        std::ifstream file(jsonFilePath);
        if (!file.is_open()) {
            SKSE::log::error("Failed to open JSON file: {}", jsonFilePath.string());
            return;
        }

        nlohmann::json jsonData;
        file >> jsonData;

        // Check for each of the top-level keys
        const std::array<std::string, 5> topLevelKeys = {
            "PercentageDiscoveredMessages", "OnSpecificLocationDiscovered", "OnMatchingLocationDiscovered", "OnSpecificLocationCleared", "OnMatchingLocationCleared"
        };

        for (const auto& key : topLevelKeys) {
            if (jsonData.contains(key) && jsonData[key].is_object()) {
                auto& targetCollection = [&]() -> MessageCollection& {
                    if (key == "PercentageDiscoveredMessages") return TheSillyMessages.PercentageDiscoveredMessages;
                    if (key == "OnSpecificLocationDiscovered") return TheSillyMessages.OnSpecificLocationDiscovered;
                    if (key == "OnMatchingLocationDiscovered") return TheSillyMessages.OnMatchingLocationDiscovered;
                    if (key == "OnSpecificLocationCleared") return TheSillyMessages.OnSpecificLocationCleared;
                    return TheSillyMessages.OnMatchingLocationCleared;  // OnMatchingLocationCleared
                }();

                // Iterate through each sub-object key
                for (auto& [subKey, value] : jsonData[key].items()) {
                    if (value.is_array()) {
                        std::vector<std::string> messages;
                        for (const auto& message : value) {
                            if (message.is_string()) {
                                messages.push_back(message.get<std::string>());
                            }
                        }

                        // Add messages to the collection
                        if (!messages.empty()) {
                            targetCollection[subKey] = messages;
                            Log("Added {} messages for key '{}' in {}", messages.size(), subKey, key);
                        }
                    }
                }
            }
        }

    } catch (const nlohmann::json::exception& e) {
        SKSE::log::error("JSON parsing error in file {}: {}", jsonFilePath.string(), e.what());
    } catch (const std::exception& e) {
        SKSE::log::error("Error processing JSON file {}: {}", jsonFilePath.string(), e.what());
    }

    Log("Finished loading JSON file: {}", jsonFilePath.string());
}

void FindAndLoadAllJsonFiles() {
    try {
        Log("Looking for JSON files in: {}", JSON_FILES_FOLDER.string());

        if (!std::filesystem::exists(JSON_FILES_FOLDER)) {
            SKSE::log::error("JSON files folder does not exist: {}", JSON_FILES_FOLDER.string());
            return;
        }

        // Iterate through all files in the directory
        for (const auto& entry : std::filesystem::directory_iterator(JSON_FILES_FOLDER)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                LoadSillyMessagesFromJsonFile(entry.path());
            }
        }

        Log("Finished loading all JSON files");
    } catch (const std::exception& e) {
        SKSE::log::error("Error finding JSON files: {}", e.what());
    }
}

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

class EventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>, public RE::BSTEventSink<RE::LocationDiscovery::Event>, public RE::BSTEventSink<RE::LocationCleared::Event> {
public:
    static EventSink* instance() {
        static EventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
        if (event->opening && event->menuName == RE::JournalMenu::MENU_NAME) {
            std::thread([&]() { RecalculateTotalNumberOfDiscoveredLocationsWithMapMarkers(); }).detach();
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationDiscovery::Event* event, RE::BSTEventSource<RE::LocationDiscovery::Event>*) override {
        Log("LOCATION DISCOVERED: {} - {}", event->worldspaceID, event->mapMarkerData->locationName.GetFullName());
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::LocationCleared::Event* event, RE::BSTEventSource<RE::LocationCleared::Event>*) override {
        Log("LOCATION CLEARED");
        return RE::BSEventNotifyControl::kContinue;
    }
};

SKSEPlugin_OnDataLoaded {
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(EventSink::instance());
    RE::LocationDiscovery::GetEventSource()->AddEventSink(EventSink::instance());

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
}