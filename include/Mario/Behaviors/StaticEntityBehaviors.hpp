/**
 * @file StaticEntityBehaviors.hpp
 * @brief Passive / static / projectile behaviors with no autonomous AI logic.
 *        Contains AxeBehavior, PrincessBehavior, and AxeProjectileBehavior.
 * @inheritance IEntityBehavior <- AxeBehavior
 *              IEntityBehavior <- PrincessBehavior
 *              IEntityBehavior <- AxeProjectileBehavior
 */
#ifndef MARIO_STATIC_ENTITY_BEHAVIORS_HPP
#define MARIO_STATIC_ENTITY_BEHAVIORS_HPP

#include <memory>

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

// ============================================================================
// AxeBehavior — spinning axe at the end of Bowser's bridge
// ============================================================================
/**
 * Animates the bridge axe and detects player contact.
 * On contact: marks entity deleted so App can trigger bridge collapse.
 * @inheritance IEntityBehavior <- AxeBehavior
 */
class AxeBehavior : public IEntityBehavior {
   public:
    AxeBehavior() = default;
    ~AxeBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /** On contact: consume the event and signal bridge collapse. */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    std::unique_ptr<IEntityBehavior> Clone() const override {
        return std::make_unique<AxeBehavior>(*this);
    }
    const char* GetName() const override { return "AxeBehavior"; }

   private:
    int m_AnimationFrame = 0;
};

// ============================================================================
// PrincessBehavior — static NPC that marks the game-clear goal
// ============================================================================
/**
 * Idle animation only — no movement, no combat.
 * Player contact is detected externally by LevelCompleteController.
 * @inheritance IEntityBehavior <- PrincessBehavior
 */
class PrincessBehavior : public IEntityBehavior {
   public:
    PrincessBehavior() = default;
    ~PrincessBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "PrincessBehavior"; }
};

// ============================================================================
// AxeProjectileBehavior — thrown axe projectile behavior
// ============================================================================
/**
 * Thrown axe projectile behavior.
 * Always behaves as an enemy projectile, damaging player on contact.
 * @inheritance IEntityBehavior <- AxeProjectileBehavior
 */
class AxeProjectileBehavior : public IEntityBehavior {
   public:
    AxeProjectileBehavior() = default;
    ~AxeProjectileBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "AxeProjectileBehavior"; }

    bool IsEnemyProjectile() const override { return true; }
    bool IsImmuneToStomp() const override { return true; }
};

}  // namespace Mario

#endif  // MARIO_STATIC_ENTITY_BEHAVIORS_HPP
