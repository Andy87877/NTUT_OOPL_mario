/**
 * @file FireballBehavior.hpp
 * @brief Behavior for projectiles (player fireball, Bowser fireball).
 *        Handles movement, lifetime, and collision with enemies.
 * @inheritance IEntityBehavior
 */
#ifndef MARIO_FIREBALL_BEHAVIOR_HPP
#define MARIO_FIREBALL_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Implements behavior for projectiles:
 *   - Player Fireball: Thrown by fire Mario, bounces, kills enemies
 *   - Bowser Fireball: Thrown by Bowser boss, damages player
 *
 * Both types have limited lifetime and arc motion (gravity-affected).
 */
class FireballBehavior : public IEntityBehavior {
   public:
    enum class FireballType {
        PLAYER,  // Thrown by fire Mario
        BOWSER,  // Thrown by boss
    };

    explicit FireballBehavior(FireballType type = FireballType::PLAYER);
    virtual ~FireballBehavior() = default;

    /**
     * Update fireball trajectory and lifetime.
     * - Apply gravity for parabolic path
     * - Handle block collisions (bounce or destroy)
     * - Decrement lifetime counter
     * - Remove if expired
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Handle collision with enemy or player.
     * Player fireball hits enemies: squish them.
     * Bowser fireball hits player: deal damage.
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "FireballBehavior"; }

    bool AlwaysUpdate() const override { return true; }

    /**
     * Get remaining lifetime in frames.
     */
    int GetRemainingLifetime() const { return m_LifetimeFrames; }

   private:
    FireballType m_Type;
    int m_LifetimeFrames = 240;  // 4 seconds at 60fps
};

}  // namespace Mario

#endif  // MARIO_FIREBALL_BEHAVIOR_HPP
