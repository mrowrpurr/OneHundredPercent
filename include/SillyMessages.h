#pragma once

#include <collections.h>

#include <string>
#include <unordered_map>
#include <vector>

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

    bool        HasSpecificLocationMessage(std::string_view locationName);
    std::string GetRandomSpecificLocationMessage(std::string_view locationName);

private:
    SillyMessages();

    std::unordered_map<std::string, std::vector<std::string>> specificLocationMessages;
    bool                                                      loadedSpecificLocationMessages = false;

    void LoadSpecificLocationMessages();
};
