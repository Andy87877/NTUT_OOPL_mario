/**
 * @file GoombaBehavior.hpp
 * @brief Behavior for Goombas implementing standard C# patrol logic.
 *        Walks left or right at a constant speed, flips on wall collision, and squishes when stomped.
 * @inheritance IEntityBehavior -> GoombaBehavior
 */
#ifndef MARIO_GOOMBA_BEHAVIOR_HPP
#define MARIO_GOOMBA_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Implements standard NES/C# patrol behavior for Goombas:
 *   - Constant walking patrol.
 *   - Bounces off solid walls and turns around (handled by CollisionManager).
 *   - Squishes when stomped by Mario from above.
 */
class GoombaBehavior : public IEntityBehavior {
   public:
    GoombaBehavior() = default;
    virtual ~GoombaBehavior() override = default;

    /**
     * Update Goomba smart AI behavior.
     * - Perform dodge-hop / cliff avoidance checks
     * - Handle proximity pursuit speed adjustments
     * - Advance animations
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Handle player collision.
     * - If from above: squish Goomba
     * - If from side: damage player (handled by CollisionManager)
     * @return True if Goomba was squished
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "GoombaBehavior"; }

   private:
    int m_DirectionChangeCounter = 0;
};

}  // namespace Mario

#endif  // MARIO_GOOMBA_BEHAVIOR_HPP
