/**
 * @file LevelConfig.hpp
 * @brief Configuration data and overrides for game levels to decouple string checks from logic.
 * @inheritance None
 */
#ifndef MARIO_LEVEL_CONFIG_HPP
#define MARIO_LEVEL_CONFIG_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include "Mario/Level/EntityDef.hpp"

namespace Mario {

struct LevelPropertyProfile {
    bool hasFlag = false;
    bool hasBoss = false;
    bool spawnCastleFireSpawner = false;
    bool hasCameraBossLock = false;
    float cameraBossLockOffset = 0.0f;
    std::vector<std::pair<int, int>> podobooSpawns;
    std::unordered_map<EntityType, float> enemyWidthOverrides;
    std::unordered_map<EntityType, float> projectileWidthOverrides;
    bool isUnderground = false;
    std::string subLevelName = "";
};

class LevelConfig {
   public:
    /**
     * Retrieve the configuration profile for a specific level.
     */
    static const LevelPropertyProfile& GetProfile(const std::string& levelName);

    /**
     * Check if flagpole sequence is present in this level.
     */
    static bool HasFlag(const std::string& levelName);

    /**
     * Check if a boss sequence exists in this level.
     */
    static bool HasBoss(const std::string& levelName);

    /**
     * Check if off-screen CastleFireSpawner should be created automatically.
     */
    static bool SpawnCastleFireSpawner(const std::string& levelName);

    /**
     * Check if camera boss room lock is active for this level.
     */
    static bool HasCameraBossLock(const std::string& levelName);

    /**
     * Get camera scroll locking coordinate.
     */
    static float GetCameraBossLockOffset(const std::string& levelName);

    /**
     * Get dynamic Podoboo spawn coordinates (column, row).
     */
    static std::vector<std::pair<int, int>> GetPodobooSpawns(const std::string& levelName);

    /**
     * Get override rendering width for specific entity types.
     * Returns 0.0f if there is no override.
     */
    static float GetRenderTargetWidthOverride(const std::string& levelName, EntityType type, bool isEnemy);
};

} // namespace Mario

#endif // MARIO_LEVEL_CONFIG_HPP
