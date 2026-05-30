/**
 * @file BowserBehavior.hpp
 * @brief Behavior for Bowser (8-4 Boss).
 *        Handles patrol, fireball attacks, and phase transitions.
 * @inheritance IEntityBehavior
 */
#ifndef MARIO_BOWSER_BEHAVIOR_HPP
#define MARIO_BOWSER_BEHAVIOR_HPP

#include <vector>

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

// Forward declaration
class EntityFactory;

/**
 * Implements AI for the Bowser boss fight (8-4).
 * Phases:
 *   1. Patrol left-right
 *   2. Fire pattern attack
 *   3. Jump attack
 *   4. Damage state (flashing red)
 *   5. Defeat sequence
 *
 * Behavior custom-designed for 8-4 level.
 */
class BowserBehavior : public IEntityBehavior {
   public:
    enum class BowserPhase {
        PATROL,       // Walking back-and-forth
        FIRE_ATTACK,  // Shooting fireballs
        JUMP_ATTACK,  // Jumping at player
        DAMAGED,      // Just took damage, flashing
        DEFEATED,     // Defeated, falling into lava
    };

    struct SpawnRequest {
        EntityType type;
        float x;
        float y;
        int dir;
    };

    BowserBehavior();
    virtual ~BowserBehavior() = default;

    /**
     * Update Bowser AI each frame.
     * - Check current phase
     * - Execute phase-specific behavior
     * - Handle state transitions
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Handle player fireball hit.
     * Damaging Bowser transitions to next phase.
     * Player normal contact deals damage to player.
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Handle fireball hit using Bowser's 3-HP system.
     * Returns true so CollisionManager only deletes the fireball.
     */
    bool OnFireballHit(EntityState& state) override;

    /**
     * Consume a pending Bowser fireball or axe spawn request.
     * Set by Update AI when it's time to shoot/throw.
     */
    bool ConsumeSpawnRequest(EntityType& outType, float& outX, float& outY,
                             int& outDir) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "BowserBehavior"; }

    bool AlwaysUpdate() const override { return true; }

    /**
     * Bowser cannot be stomped — contact always damages Mario.
     * Star power still defeats him via the handler's star-kill path.
     */
    bool IsImmuneToStomp() const override { return true; }
    bool IsImmuneToStarPower() const override { return true; }

    /** Used by AxeSequenceSceneHandler to locate Bowser without EntityType
     * check. */
    bool IsBowser() const override { return true; }

    /**
     * Bowser spawns enemy fireballs and axe projectiles.
     * EntityFactory::SpawnProjectile uses this to configure projectile
     * physics (speed, gravity) without comparing EntityType (OCP).
     */
    bool IsEnemySpawner() const override { return true; }

    /**
     * Check if Bowser is defeated.
     */
    bool IsDefeated() const { return m_Phase == BowserPhase::DEFEATED; }

    /**
     * Get current phase for debugging.
     */
    BowserPhase GetPhase() const { return m_Phase; }

   private:
    BowserPhase m_Phase = BowserPhase::PATROL;
    int m_PhaseTimer = 0;          // Timer for phase duration
    int m_AttackCounter = 0;       // Number of attacks in current phase
    int m_HealthPoints = 3;        // Takes 3 fireball hits to defeat
    int m_DamageFlashCounter = 0;  // Flashing effect after damage
    int m_PatrolDirection = 1;     // 1 = right, -1 = left

    // Queued spawn requests (fireballs, axes, etc.)
    std::vector<SpawnRequest> m_PendingSpawns;

    int m_AxeThrowTimer = 0;
    static constexpr int AXE_THROW_INTERVAL =
        24;  // Throws an axe every 24 frames!

    int m_FireballTimer = 0;
    static constexpr int FIREBALL_INTERVAL =
        120;  // Spits a fireball every 120 frames (slower CD)!

    static constexpr int PATROL_PHASE_LENGTH = 180;   // 3 seconds at 60fps
    static constexpr int FIRE_ATTACK_LENGTH = 120;    // 2 seconds
    static constexpr int ATTACK_INTERVAL = 40;        // Frames between attacks
    static constexpr int DAMAGE_FLASH_DURATION = 60;  // Frames to flash

    // Bridge range constraints for 8-4 Boss Room
    float m_BridgeLeft = -1.0f;
    float m_BridgeRight = -1.0f;

    // Helper methods
    void UpdatePatrol(EntityState& state, const Level& level,
                      const Player& player);
    void UpdateFireAttackPhase(const EntityState& state, const Player& player);
    void UpdateJumpAttack(EntityState& state, const Level& level,
                          const Player& player);
    void UpdateDamaged(EntityState& state);
    void UpdateDefeated(EntityState& state);

    /**
     * Check wall tiles in m_PatrolDirection and reverse if blocked.
     * Also checks for ledge-edge and reverses. Eliminates the duplicated
     * grid-scan loop that existed in both UpdatePatrol and UpdateJumpAttack.
     */
    void ResolveWallAndEdge(EntityState& state, const Level& level);
};

}  // namespace Mario

#endif  // MARIO_BOWSER_BEHAVIOR_HPP
