#pragma once

#include <RE/Skyrim.h>

#include "StringUtils.h"

inline RE::FormID GetFormID(const RE::TESFile* plugin, RE::FormID localFormId) {
    // Special case for Skyrim.esm which should have 0 as its index (instead of 80)
    if (ToLowerCase(plugin->GetFilename()) == "skyrim.esm") return localFormId;  // For Skyrim.esm, we keep the local FormID as is
    if (plugin->IsLight()) {
        return (localFormId & 0xFFF) | (0xFE000 | (plugin->GetSmallFileCompileIndex() << 12));
    } else {
        return (localFormId & 0xFFFFFF) | (plugin->GetCompileIndex() << 24);
    }
}

/*
From CommonLibSSE:
    FormID GetLocalFormID() {
        auto file = GetFile(0);
        RE::FormID fileIndex = file->compileIndex << (3 * 8);
        fileIndex += file->smallFileCompileIndex << ((1 * 8) + 4);
        return formID & ~fileIndex;
    }
*/

inline const RE::FormID GetLocalFormID(const RE::BGSLocation* location) {
    if (!location) return 0;

    auto file = location->GetFile(0);
    if (!file) return 0;

    RE::FormID fileIndex = file->compileIndex << (3 * 8);
    fileIndex += file->smallFileCompileIndex << ((1 * 8) + 4);

    return location->GetFormID() & ~fileIndex;
}
