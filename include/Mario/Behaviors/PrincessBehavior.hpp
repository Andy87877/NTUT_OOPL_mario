/**
 * @file PrincessBehavior.hpp
 * @brief Princess NPC behavior using Strategy pattern.
 *        Static display with idle animation - no AI logic needed.
 * @inheritance IEntityBehavior <- PrincessBehavior
 */
#ifndef MARIO_PRINCESS_BEHAVIOR_HPP
#define MARIO_PRINCESS_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * PrincessBehavior — Peach NPC waiting for rescue.
 * Display only - no movement, no collision damage.
 * Used as level goal marker.
 */
class PrincessBehavior : public IEntityBehavior {
   public:
    PrincessBehavior() = default;
    virtual ~PrincessBehavior() = default;

    /**
     * Update is minimal - just idle animation.
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Princess handles no collisions.
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "PrincessBehavior"; }
};

}  // namespace Mario

#endif  // MARIO_PRINCESS_BEHAVIOR_HPP
