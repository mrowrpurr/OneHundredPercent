#pragma once

#include <collections.h>
#include <string>
#include <vector>
#include <unordered_map>

using MessageCollection = collections_map<std::string, std::vector<std::string>>;

struct SillyMessages {
    MessageCollection PercentageDiscoveredMessages;
    MessageCollection OnSpecificLocationDiscovered;
    MessageCollection OnMatchingLocationDiscovered;
    MessageCollection OnSpecificLocationCleared;
    MessageCollection OnMatchingLocationCleared;

    static SillyMessages& instance();

    std::string GetRandomMessage_LocationDiscovered(std::string_view locationName);
    std::string GetRandomMessage_LocationCleared(std::string_view locationName);
    std::string GetRandomMessage_PercentageDiscovered(float percentage);

    // New methods for specific location messages
    bool HasSpecificLocationMessage(const std::string& locationName);
    std::string GetRandomSpecificLocationMessage(const std::string& locationName);

private:
    SillyMessages();

    // Storage for specific location messages loaded from JSON
    std::unordered_map<std::string, std::vector<std::string>> specificLocationMessages;
    bool loadedSpecificLocationMessages = false;

    void LoadSpecificLocationMessages();
};
