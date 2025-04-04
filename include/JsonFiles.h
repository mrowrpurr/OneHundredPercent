#pragma once

#include <RE/Skyrim.h>  // For RE::FormID
#include <collections.h>

#include <filesystem>
#include <string>

struct IgnoredLocation {
    RE::FormID  formId;
    std::string plugin;
};

inline collections_set<RE::FormID>  IgnoredLocationIDs;
inline collections_set<std::string> IgnoredMapMarkers;

void LoadSillyMessagesFromJsonFile(std::filesystem::path jsonFilePath);
void FindAndLoadAllJsonFiles();
