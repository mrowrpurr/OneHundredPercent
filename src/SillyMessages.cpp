#pragma once

#include "SillyMessages.h"

#include <SKSE/SKSE.h>
#include <SkyrimScripting/Logging.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <regex>

using json = nlohmann::json;

namespace {
    // Helper to get a random message from a vector of strings
    std::string GetRandomMessage(const std::vector<std::string>& messages) {
        if (messages.empty()) {
            return "";
        }

        static std::random_device rd;
        static std::mt19937       gen(rd());

        std::uniform_int_distribution<> distrib(0, static_cast<int>(messages.size() - 1));
        return messages[distrib(gen)];
    }

    // Helper to replace capture groups in a message
    std::string ReplaceCaptureGroups(const std::string& message, const std::smatch& matches) {
        std::string result = message;

        for (size_t i = 1; i < matches.size(); ++i) {
            std::string placeholder = "$" + std::to_string(i);
            std::string replacement = matches[i].str();

            size_t pos = result.find(placeholder);
            while (pos != std::string::npos) {
                result.replace(pos, placeholder.length(), replacement);
                pos = result.find(placeholder, pos + replacement.length());
            }
        }

        return result;
    }
}

SillyMessages& SillyMessages::instance() {
    static SillyMessages instance;
    return instance;
}

SillyMessages::SillyMessages() {
    // Load percentage messages from PercentageDiscovered.json
    try {
        auto pluginPath = std::filesystem::path("Data/SKSE/Plugins/HazTheCompletionizt");
        auto jsonPath   = pluginPath / "PercentageDiscovered.json";

        if (std::filesystem::exists(jsonPath)) {
            std::ifstream jsonFile(jsonPath);
            if (jsonFile.is_open()) {
                json root;
                jsonFile >> root;

                if (root.contains("PercentageDiscoveredMessages")) {
                    auto& percentageData = root["PercentageDiscoveredMessages"];
                    for (auto& [percentage, messages] : percentageData.items()) {
                        std::vector<std::string> percentageMessageList;
                        for (auto& message : messages) {
                            percentageMessageList.push_back(message.get<std::string>());
                        }
                        PercentageDiscoveredMessages[percentage] = percentageMessageList;
                    }
                }
            }
        }
    } catch (std::exception& e) {
        // Handle error
    }

    // Don't load location messages immediately - defer until needed
}

bool SillyMessages::HasSpecificLocationMessage(const std::string& locationName) {
    if (!loadedSpecificLocationMessages) {
        LoadSpecificLocationMessages();
    }

    return specificLocationMessages.contains(locationName) && !specificLocationMessages[locationName].empty();
}

std::string SillyMessages::GetRandomSpecificLocationMessage(const std::string& locationName) {
    if (!loadedSpecificLocationMessages) {
        LoadSpecificLocationMessages();
    }

    if (!specificLocationMessages.contains(locationName) || specificLocationMessages[locationName].empty()) {
        return locationName;  // Fall back to just the location name if no message found
    }

    const auto&                     messages = specificLocationMessages[locationName];
    static std::random_device       rd;
    static std::mt19937             gen(rd());
    std::uniform_int_distribution<> distrib(0, messages.size() - 1);

    return messages[distrib(gen)];
}

void SillyMessages::LoadSpecificLocationMessages() {
    try {
        auto pluginPath = std::filesystem::path("Data/SKSE/Plugins/HazTheCompletionizt");
        auto jsonPath   = pluginPath / "OnSpecificLocationDiscovered.json";

        if (!std::filesystem::exists(jsonPath)) {
            // Log error or print debug message
            return;
        }

        std::ifstream jsonFile(jsonPath);
        if (!jsonFile.is_open()) {
            // Log error or print debug message
            return;
        }

        json root;
        jsonFile >> root;

        if (root.contains("OnSpecificLocationDiscovered")) {
            auto& locationData = root["OnSpecificLocationDiscovered"];
            for (auto& [location, messages] : locationData.items()) {
                std::vector<std::string> locationMessageList;
                for (auto& message : messages) {
                    locationMessageList.push_back(message.get<std::string>());
                }
                specificLocationMessages[location] = locationMessageList;
            }
        }

        loadedSpecificLocationMessages = true;
    } catch (std::exception& e) {
        // Log error or print debug message
        loadedSpecificLocationMessages = true;  // Set to true anyway to prevent repeated loading attempts
    }
}

std::string SillyMessages::GetRandomMessage_LocationDiscovered(std::string_view locationName) {
    // Try to find an exact match (case sensitive)
    auto specificIt = OnSpecificLocationDiscovered.find(std::string(locationName));
    if (specificIt != OnSpecificLocationDiscovered.end()) {
        return GetRandomMessage(specificIt->second);
    }

    // Fall back to regex matching (case insensitive)
    std::string location(locationName);
    for (const auto& [pattern, messages] : OnMatchingLocationDiscovered) {
        std::regex  regexPattern(pattern, std::regex_constants::icase);
        std::smatch matches;

        if (std::regex_match(location, matches, regexPattern)) {
            std::string message = GetRandomMessage(messages);
            return ReplaceCaptureGroups(message, matches);
        }
    }

    return "";
}

std::string SillyMessages::GetRandomMessage_LocationCleared(std::string_view locationName) {
    Log("Looking for cleared location message for: '{}'", locationName);

    // First check if we have a specific named location message
    if (HasSpecificLocationMessage(std::string(locationName))) {
        return GetRandomSpecificLocationMessage(std::string(locationName));
    }

    // Try to find an exact match (case sensitive)
    auto specificIt = OnSpecificLocationCleared.find(std::string(locationName));
    if (specificIt != OnSpecificLocationCleared.end()) {
        return GetRandomMessage(specificIt->second);
    }

    Log("Trying regex patterns for location: '{}'", locationName);
    std::string location(locationName);
    for (const auto& [pattern, messages] : OnMatchingLocationCleared) {
        Log("  Checking pattern: '{}'", pattern);
        std::regex  regexPattern(pattern, std::regex_constants::icase);
        std::smatch matches;

        if (std::regex_match(location, matches, regexPattern)) {
            Log("  MATCHED with pattern: '{}'", pattern);
            std::string message = GetRandomMessage(messages);
            return ReplaceCaptureGroups(message, matches);
        }
    }

    Log("No matching pattern found for: '{}'", locationName);
    return "";
}

std::string SillyMessages::GetRandomMessage_PercentageDiscovered(float percentage) {
    // Round down to nearest integer
    int percentInt = static_cast<int>(std::floor(percentage));

    // Convert to string key
    std::string percentKey = std::to_string(percentInt);

    // Look for an exact match for the percentage
    auto it = PercentageDiscoveredMessages.find(percentKey);
    if (it != PercentageDiscoveredMessages.end()) {
        return GetRandomMessage(it->second);
    }

    // If no exact match, try range matching
    // For example, check for ranges like "0-10", "11-20", etc.
    for (const auto& [range, messages] : PercentageDiscoveredMessages) {
        // Check if the range key contains a hyphen, indicating a range
        size_t hyphenPos = range.find('-');
        if (hyphenPos != std::string::npos) {
            try {
                int rangeStart = std::stoi(range.substr(0, hyphenPos));
                int rangeEnd   = std::stoi(range.substr(hyphenPos + 1));

                if (percentInt >= rangeStart && percentInt <= rangeEnd) {
                    return GetRandomMessage(messages);
                }
            } catch (const std::exception&) {
                // Ignore malformed ranges
                continue;
            }
        }
    }

    return "";
}
