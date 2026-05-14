/**
 * @file StaticEntityBehaviors.hpp
 * @brief Passive / static entity behaviors with no autonomous movement.
 *        Contains AxeBehavior (bridge-axe kill trigger) and PrincessBehavior
 *        (NPC goal marker). Both are used exclusively in 8-4 and have trivial
 *        implementations that do not justify separate files.
 * @inheritance IEntityBehavior <- AxeBehavior
 *              IEntityBehavior <- PrincessBehavior
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

}  // namespace Mario

#endif  // MARIO_STATIC_ENTITY_BEHAVIORS_HPP
