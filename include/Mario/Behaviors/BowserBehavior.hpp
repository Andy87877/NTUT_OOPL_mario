/**
 * @file BowserBehavior.hpp
 * @brief Behavior for Bowser (8-4 Boss).
 *        Handles patrol, fireball attacks, and phase transitions.
 * @inheritance IEntityBehavior
 */
#ifndef MARIO_BOWSER_BEHAVIOR_HPP
#define MARIO_BOWSER_BEHAVIOR_HPP

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
     * Consume a pending Bowser fireball spawn request.
     * Set by UpdateFireAttackPhase() when it's time to shoot.
     */
    bool ConsumeSpawnRequest(int& outType, float& outX, float& outY,
                             int& outDir) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "BowserBehavior"; }

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

    // Pending fireball spawn request (set by UpdateFireAttackPhase)
    bool m_FireballPending = false;
    float m_FireballX = 0.0f;
    float m_FireballY = 0.0f;
    int m_FireballDir = 0;  // direction toward player

    static constexpr int PATROL_PHASE_LENGTH = 180;   // 3 seconds at 60fps
    static constexpr int FIRE_ATTACK_LENGTH = 120;    // 2 seconds
    static constexpr int ATTACK_INTERVAL = 40;        // Frames between attacks
    static constexpr int DAMAGE_FLASH_DURATION = 60;  // Frames to flash

    // Helper methods
    void UpdatePatrol(EntityState& state, const Level& level,
                      const Player& player);
    void UpdateFireAttackPhase(const EntityState& state, const Player& player);
    void UpdateJumpAttack(EntityState& state, const Level& level,
                          const Player& player);
    void UpdateDamaged(EntityState& state);
    void UpdateDefeated(EntityState& state);
};

}  // namespace Mario

#endif  // MARIO_BOWSER_BEHAVIOR_HPP
