#pragma once

#include "JsonFiles.h"

#include <RE/Skyrim.h>  // For RE::FormID
#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include <nlohmann/json.hpp>

#include "Config.h"
#include "FormUtils.h"
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

        collections_map<std::string, const RE::TESFile*> plugins;

        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            SKSE::log::error("Failed to get TESDataHandler");
            return;
        }

        for (auto& [fileKey, fileValue] : jsonData.items()) {
            Log("File: {}, Key: {}", jsonFilePath.string(), fileKey);

            if (fileKey == "IgnoredLocations" && fileValue.is_array()) {
                for (const auto& location : fileValue) {
                    if (location.is_object() && location.contains("formId") && location.contains("plugin")) {
                        try {
                            auto               localFormId    = std::stoul(location["formId"].get<std::string>(), nullptr, 16);
                            auto               pluginFilename = location["plugin"].get<std::string>();
                            const RE::TESFile* plugin;
                            auto               foundPlugin = plugins.find(pluginFilename);
                            if (foundPlugin == plugins.end()) {
                                plugin                  = dataHandler->LookupModByName(pluginFilename);
                                plugins[pluginFilename] = plugin;
                            }
                            if (plugin) {
                                Log("Configured location to ignore: {:x} from plugin: {}", localFormId, pluginFilename);
                                IgnoredLocationIDs.insert(GetFormID(plugin, localFormId));
                            } else {
                                Log("Plugin not found or invalid: {}", pluginFilename);
                            }
                        } catch (const std::exception& e) {
                            SKSE::log::error("Error parsing IgnoredLocation in file {}: {}", jsonFilePath.string(), e.what());
                        }
                    }
                }
            } else {
                const std::array<std::string, 5> knownKeys = {
                    "PercentageDiscoveredMessages", "OnSpecificLocationDiscovered", "OnMatchingLocationDiscovered", "OnSpecificLocationCleared", "OnMatchingLocationCleared"
                };

                for (const auto& key : knownKeys) {
                    if (fileKey == key && fileValue.is_object()) {
                        Log("Found nested structure for key: {}", key);
                        auto& targetCollection = [&]() -> MessageCollection& {
                            if (key == "PercentageDiscoveredMessages") return SillyMessages::instance().PercentageDiscoveredMessages;
                            if (key == "OnSpecificLocationDiscovered") return SillyMessages::instance().OnSpecificLocationDiscovered;
                            if (key == "OnMatchingLocationDiscovered") return SillyMessages::instance().OnMatchingLocationDiscovered;
                            if (key == "OnSpecificLocationCleared") return SillyMessages::instance().OnSpecificLocationCleared;
                            if (key == "OnMatchingLocationCleared") return SillyMessages::instance().OnMatchingLocationCleared;
                            return SillyMessages::instance().OnMatchingLocationCleared;
                        }();

                        for (auto& [subKey, value] : fileValue.items()) {
                            if (value.is_array()) {
                                std::vector<std::string> messages;
                                for (const auto& message : value) {
                                    if (message.is_string()) {
                                        messages.push_back(message.get<std::string>());
                                    }
                                }

                                if (!messages.empty()) {
                                    targetCollection[subKey] = messages;
                                }
                            }
                        }
                    }
                }
            }
        }

        Log("Finished loading JSON file: {}", jsonFilePath.string());
    } catch (const nlohmann::json::exception& e) {
        SKSE::log::error("JSON parsing error in file {}: {}", jsonFilePath.string(), e.what());
    } catch (const std::exception& e) {
        SKSE::log::error("Error processing JSON file {}: {}", jsonFilePath.string(), e.what());
    }
}

void FindAndLoadAllJsonFiles() {
    try {
        Log("Looking for JSON files in: {}", Config::JSON_FILES_FOLDER.string());

        if (!std::filesystem::exists(Config::JSON_FILES_FOLDER)) {
            SKSE::log::error("JSON files folder does not exist: {}", Config::JSON_FILES_FOLDER.string());
            return;
        }

        // Clear IgnoredLocations before loading
        IgnoredLocationIDs.clear();

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