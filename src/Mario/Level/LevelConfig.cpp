/**
 * @file LevelConfig.cpp
 * @brief Implementations of LevelConfig static query methods.
 * @inheritance None
 */
#include "Mario/Level/LevelConfig.hpp"
#include <algorithm>

namespace Mario {

bool LevelConfig::HasFlag(const std::string& levelName) {
    // 1-1 and its variants have flagpole ending. 8-4 does not (uses Axe).
    return (levelName == "1-1" || levelName.find("1-1") != std::string::npos || levelName == "1-2");
}

bool LevelConfig::HasBoss(const std::string& levelName) {
    // 8-4 is the boss level containing Bowser, Princess, and Axe.
    return (levelName == "8-4");
}

bool LevelConfig::SpawnCastleFireSpawner(const std::string& levelName) {
    // 8-4 contains dynamic CastleFireSpawner.
    return (levelName == "8-4");
}

bool LevelConfig::HasCameraBossLock(const std::string& levelName) {
    // 8-4 boss room restricts camera movement.
    return (levelName == "8-4");
}

float LevelConfig::GetCameraBossLockOffset(const std::string& levelName) {
    (void)levelName;
    // 14400.0f is the absolute offset of 8-4 boss room.
    return 14400.0f;
}

std::vector<std::pair<int, int>> LevelConfig::GetPodobooSpawns(const std::string& levelName) {
    if (levelName == "8-4") {
        return { {68, 13}, {145, 13}, {222, 13}, {336, 11} };
    }
    return {};
}

float LevelConfig::GetRenderTargetWidthOverride(const std::string& levelName, EntityType type, bool isEnemy) {
    if (levelName == "8-4") {
        // Default target width for 8-4 is 32.0f.
        switch (type) {
            case EntityType::BOWSER:
                return 64.0f; // Bowser is 2 tiles wide
            case EntityType::FIRE:
                if (isEnemy) return 48.0f; // Bowser fireball is 1.5 tiles wide
                break;
            case EntityType::PIRANHA_PLANT:
                return 64.0f; // Piranha plant is 2 tiles wide
            default:
                return 32.0f; // Default scaling
        }
    } else {
        // Default levels scaling:
        if (type == EntityType::PIRANHA_PLANT) {
            return 64.0f; // Piranha plant is 2 tiles wide
        }
    }
    return 0.0f; // No override, use default DRAW_SCALE
}

} // namespace Mario
