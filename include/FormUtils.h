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

inline const RE::FormID GetLocalFormID(const RE::TESForm* form) {
    if (!form) return 0;

    auto file = form->GetFile(0);
    if (!file) return 0;

    RE::FormID fileIndex = file->compileIndex << (3 * 8);
    fileIndex += file->smallFileCompileIndex << ((1 * 8) + 4);

    return form->GetFormID() & ~fileIndex;
}

struct FormIdentifier {
    RE::FormID  localFormID;
    std::string pluginName;

    inline static FormIdentifier CreateIdentifier(std::string_view pluginName, RE::FormID localFormId) {
        FormIdentifier formIdentifier;
        formIdentifier.localFormID = localFormId;
        formIdentifier.pluginName  = ToLowerCase(pluginName);
        return formIdentifier;
    }

    inline static FormIdentifier CreateIdentifier(RE::TESForm* form) { return CreateIdentifier(ToLowerCase(form->GetFile(0)->GetFilename()), form->GetLocalFormID()); }
    inline static FormIdentifier CreateIdentifier(const RE::TESForm* form) { return CreateIdentifier(ToLowerCase(form->GetFile(0)->GetFilename()), GetLocalFormID(form)); }

    inline bool operator==(const FormIdentifier& other) const { return localFormID == other.localFormID && pluginName == other.pluginName; }
};

namespace std {
    template <>
    struct hash<FormIdentifier> {
        std::size_t operator()(const FormIdentifier& identifier) const {
            std::size_t hash1 = std::hash<RE::FormID>{}(identifier.localFormID);
            std::size_t hash2 = std::hash<std::string>{}(identifier.pluginName);
            return hash1 ^ (hash2 << 1);  // Combine the two hashes
        }
    };
}
