// https://github.com/zax-ftw/Experience/blob/52002dbc04558e91a9b27d7ebf4612f3b1863435/src/Hooks/BGSLocation.h

#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

class BGSLocationEx : public RE::BGSLocation {
public:
    static BGSLocationEx* GetLastChecked() { return lastChecked; };
    static void           Install(SKSE::Trampoline& trampoline);

    RE::MARKER_TYPE GetMapMarkerType();

    bool ClearedCheck(int time, bool force);

private:
    bool ClearedCheck_Hook(int time, bool force);

    // members
    static BGSLocationEx* lastChecked;
};