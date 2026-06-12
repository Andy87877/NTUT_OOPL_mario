/**
 * @file LevelConfig.cpp
 * @brief Implementations of LevelConfig static query methods.
 * @inheritance None
 */
#include "Mario/Level/LevelConfig.hpp"
#include <algorithm>

namespace Mario {

const LevelPropertyProfile& LevelConfig::GetProfile(const std::string& levelName) {
    static const LevelPropertyProfile defaultProfile = {
        /*hasFlag=*/false,
        /*hasBoss=*/false,
        /*spawnCastleFireSpawner=*/false,
        /*hasCameraBossLock=*/false,
        /*cameraBossLockOffset=*/0.0f,
        /*podobooSpawns=*/{},
        /*enemyWidthOverrides=*/{
            {EntityType::PIRANHA_PLANT, 64.0f}
        },
        /*projectileWidthOverrides=*/{}
    };

    static std::unordered_map<std::string, LevelPropertyProfile> registry;
    static bool initialized = false;
    if (!initialized) {
        // Configure 1-1
        LevelPropertyProfile p1_1;
        p1_1.hasFlag = true;
        p1_1.subLevelName = "1-1u";
        p1_1.isUnderground = false;
        p1_1.enemyWidthOverrides = {{EntityType::PIRANHA_PLANT, 64.0f}};
        registry["1-1"] = p1_1;

        // Configure 1-1u (Underground bonus area)
        LevelPropertyProfile p1_1u;
        p1_1u.hasFlag = false;
        p1_1u.isUnderground = true;
        p1_1u.enemyWidthOverrides = {{EntityType::PIRANHA_PLANT, 64.0f}};
        registry["1-1u"] = p1_1u;

        // Configure 1-2
        LevelPropertyProfile p1_2;
        p1_2.hasFlag = true;
        p1_2.isUnderground = true;
        p1_2.enemyWidthOverrides = {{EntityType::PIRANHA_PLANT, 64.0f}};
        registry["1-2"] = p1_2;

        // Configure 8-4
        LevelPropertyProfile p8_4;
        p8_4.hasFlag = false;
        p8_4.hasBoss = true;
        p8_4.spawnCastleFireSpawner = true;
        p8_4.hasCameraBossLock = true;
        p8_4.cameraBossLockOffset = 14400.0f;
        p8_4.podobooSpawns = { {68, 13}, {145, 13}, {222, 13}, {336, 11} };
        p8_4.enemyWidthOverrides = {
            {EntityType::BOWSER, 64.0f},
            {EntityType::PIRANHA_PLANT, 64.0f},
            {EntityType::GOOMBA, 32.0f},
            {EntityType::KOOPA_TROOPA, 32.0f},
            {EntityType::PARAKOOPA, 32.0f},
            {EntityType::KOOPA_SHELL, 32.0f},
            {EntityType::AXE_KOOPA, 32.0f},
            {EntityType::FIRE, 48.0f},
            {EntityType::PODOBOO, 32.0f},
            {EntityType::AXE_PROJECTILE, 32.0f}
        };
        p8_4.projectileWidthOverrides = {
            {EntityType::AXE, 32.0f},
            {EntityType::PRINCESS, 32.0f}
        };
        p8_4.isUnderground = true;
        registry["8-4"] = p8_4;

        initialized = true;
    }

    auto it = registry.find(levelName);
    if (it != registry.end()) {
        return it->second;
    }
    
    // Check if the levelName contains any registered name (handling sublevel names like 1-1u etc.)
    for (const auto& [name, prof] : registry) {
        if (levelName.find(name) != std::string::npos) {
            return prof;
        }
    }
    return defaultProfile;
}

bool LevelConfig::HasFlag(const std::string& levelName) {
    return GetProfile(levelName).hasFlag;
}

bool LevelConfig::HasBoss(const std::string& levelName) {
    return GetProfile(levelName).hasBoss;
}

bool LevelConfig::SpawnCastleFireSpawner(const std::string& levelName) {
    return GetProfile(levelName).spawnCastleFireSpawner;
}

bool LevelConfig::HasCameraBossLock(const std::string& levelName) {
    return GetProfile(levelName).hasCameraBossLock;
}

float LevelConfig::GetCameraBossLockOffset(const std::string& levelName) {
    return GetProfile(levelName).cameraBossLockOffset;
}

std::vector<std::pair<int, int>> LevelConfig::GetPodobooSpawns(const std::string& levelName) {
    return GetProfile(levelName).podobooSpawns;
}

float LevelConfig::GetRenderTargetWidthOverride(const std::string& levelName, EntityType type, bool isEnemy) {
    const auto& profile = GetProfile(levelName);
    if (isEnemy) {
        auto it = profile.enemyWidthOverrides.find(type);
        if (it != profile.enemyWidthOverrides.end()) {
            return it->second;
        }
    } else {
        auto it = profile.projectileWidthOverrides.find(type);
        if (it != profile.projectileWidthOverrides.end()) {
            return it->second;
        }
    }
    return 0.0f; // No override, use default DRAW_SCALE
}

} // namespace Mario
