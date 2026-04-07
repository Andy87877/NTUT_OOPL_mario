/**
 * @file DefaultEntityBehavior.hpp
 * @brief Default passive behavior for static entities (coins, fire flowers,
 * etc). Does not move or interact actively - only responds to player collision.
 * @inheritance IEntityBehavior
 */
#ifndef MARIO_DEFAULT_ENTITY_BEHAVIOR_HPP
#define MARIO_DEFAULT_ENTITY_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Default behavior for passive entities that don't move or act independently.
 * Used for coins, fire flowers, mushrooms, and other static power-ups.
 */
class DefaultEntityBehavior : public IEntityBehavior {
   public:
    DefaultEntityBehavior() = default;
    virtual ~DefaultEntityBehavior() = default;

    /**
     * Default update does minimal work - just animation.
     * Passive entities don't move or change state on their own.
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Handle player collision - typically pickup or consume.
     * Behavior depends on entity type (coin score, power-up state, etc).
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "DefaultEntityBehavior"; }
};

}  // namespace Mario

#endif  // MARIO_DEFAULT_ENTITY_BEHAVIOR_HPP
