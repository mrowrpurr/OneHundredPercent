#pragma once

#include <RE/Skyrim.h>
#include <collections.h>

std::uint32_t                     GetNumberOfPlayerDiscoveredLocations();
collections_set<RE::BGSLocation*> GetPlayerDiscoveredLocationList();
