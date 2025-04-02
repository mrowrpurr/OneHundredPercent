#pragma once

#include "JsonFiles.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include <nlohmann/json.hpp>

#include "Config.h"
#include "SillyMessages.h"

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
                    if (key == "PercentageDiscoveredMessages") return SillyMessages::instance().PercentageDiscoveredMessages;
                    if (key == "OnSpecificLocationDiscovered") return SillyMessages::instance().OnSpecificLocationDiscovered;
                    if (key == "OnMatchingLocationDiscovered") return SillyMessages::instance().OnMatchingLocationDiscovered;
                    if (key == "OnSpecificLocationCleared") return SillyMessages::instance().OnSpecificLocationCleared;
                    return SillyMessages::instance().OnMatchingLocationCleared;  // OnMatchingLocationCleared
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
        Log("Looking for JSON files in: {}", Config::JSON_FILES_FOLDER.string());

        if (!std::filesystem::exists(Config::JSON_FILES_FOLDER)) {
            SKSE::log::error("JSON files folder does not exist: {}", Config::JSON_FILES_FOLDER.string());
            return;
        }

        // Iterate through all files in the directory
        for (const auto& entry : std::filesystem::directory_iterator(Config::JSON_FILES_FOLDER)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                LoadSillyMessagesFromJsonFile(entry.path());
            }
        }

        Log("Finished loading all JSON files");
    } catch (const std::exception& e) {
        SKSE::log::error("Error finding JSON files: {}", e.what());
    }
}