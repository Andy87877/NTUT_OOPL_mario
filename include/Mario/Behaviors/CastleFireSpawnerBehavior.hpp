/**
 * @file CastleFireSpawnerBehavior.hpp
 * @brief Behavior strategy for periodic off-screen fire spawner in Stage 8-4.
 *        Spawns hostile fireballs flying from right to left relative to Mario.
 * @inheritance IEntityBehavior
 */
#ifndef MARIO_CASTLE_FIRE_SPAWNER_BEHAVIOR_HPP
#define MARIO_CASTLE_FIRE_SPAWNER_BEHAVIOR_HPP

#include <vector>

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Strategy class implementing the off-screen fire breath spawner for Stage 8-4.
 * Invisible entity spawned at level start. It monitors player distance and
 * periodically spawns hostile fireballs crossing from right to left.
 */
class CastleFireSpawnerBehavior : public IEntityBehavior {
   public:
    struct SpawnRequest {
        EntityType type;
        float x;
        float y;
        int dir;
    };

    CastleFireSpawnerBehavior();
    virtual ~CastleFireSpawnerBehavior() = default;

    /**
     * Periodic timing logic. Spawns fireballs at right edge of camera.
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Unused since spawner is invisible and doesn't participate in physics.
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Consumes pending fireball spawn requests.
     */
    bool ConsumeSpawnRequest(EntityType& outType, float& outX, float& outY,
                             int& outDir) override;

    /**
     * Strategy pattern cloning interface.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "CastleFireSpawnerBehavior"; }

    /**
     * Ensure the spawner updates off-screen to generate fires continuously.
     */
    bool AlwaysUpdate() const override { return true; }

    /**
     * CastleFireSpawner produces enemy fireballs.
     * EntityFactory::SpawnProjectile uses this to configure projectile
     * physics without comparing EntityType (OCP).
     */
    bool IsEnemySpawner() const override { return true; }

   private:
    int m_FireballTimer = 0;
    int m_AttackCounter = 0;
    std::vector<SpawnRequest> m_PendingSpawns;

    static constexpr int FIREBALL_INTERVAL =
        180;  // Spawns a fireball every 3 seconds (180 frames)
};

}  // namespace Mario

#endif  // MARIO_CASTLE_FIRE_SPAWNER_BEHAVIOR_HPP
