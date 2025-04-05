#pragma once

#include <RE/Skyrim.h>  // For RE::FormID
#include <collections.h>

#include <filesystem>
#include <string>

struct IgnoredLocation {
    RE::FormID  formId;
    std::string plugin;
};

inline collections_set<RE::FormID>  IgnoredMapMarkers;
inline collections_set<std::string> IgnoredLocationNames;

void LoadFileFile(std::filesystem::path jsonFilePath);
void FindAndLoadAllJsonFiles();
