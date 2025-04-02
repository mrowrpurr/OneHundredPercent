#pragma once

#include <filesystem>

void LoadSillyMessagesFromJsonFile(std::filesystem::path jsonFilePath);
void FindAndLoadAllJsonFiles();
