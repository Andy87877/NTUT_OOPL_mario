/**
 * @file ResourceResolver.cpp
 * @brief Dynamic path resolution for game resources.
 *        Checks if a local "./Resources" directory is present next to the
 *        executable, otherwise falls back to the compile-time RESOURCE_DIR
 * macro.
 * @inheritance None
 */
#define BUILDING_RESOURCE_RESOLVER
#include <windows.h>

#include <fstream>

#include "pch.hpp"

std::string GetResourceDirectory() {
    static std::string cachedPath = "";
    if (!cachedPath.empty()) {
        return cachedPath;
    }

    // Retrieve the path of the current executable
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string exePath(path);
    std::string::size_type pos = exePath.find_last_of("\\/");
    std::string exeDir =
        (pos != std::string::npos) ? exePath.substr(0, pos) : ".";

    // Test if there is a local "Resources/Levels/LevelSequence.csv" next to the
    // exe
    std::string localPath = exeDir + "/Resources";
    std::ifstream testFile(localPath + "/Levels/LevelSequence.csv");
    if (testFile.good()) {
        cachedPath = localPath;
    } else {
        // Fallback to compile-time RESOURCE_DIR defined via compiler flag

#ifdef RESOURCE_DIR
        cachedPath = RESOURCE_DIR;
#else
        cachedPath = "./Resources";
#endif
    }
    return cachedPath;
}
