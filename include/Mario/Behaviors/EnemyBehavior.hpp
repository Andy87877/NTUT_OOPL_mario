/**
 * @file EnemyBehavior.hpp
 * @brief Behavior for walking enemies (Goomba, Koopa Troopa).
 *        Handles patrol movement, direction changes, and squishing.
 * @inheritance IEntityBehavior
 */
#ifndef MARIO_ENEMY_BEHAVIOR_HPP
#define MARIO_ENEMY_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Implements AI behavior for common Mario enemies:
 *   - Goomba: Simple patrol, squishable
 *   - Koopa Troopa: Patrol, can be squashed or kicked
 *
 * Behavior logic ported from C# Entity.cs Update() and Squish() methods.
 */
class EnemyBehavior : public IEntityBehavior {
   public:
    enum class EnemyType {
        GOOMBA,         // Basic walking enemy
        KOOPA_TROOPA,   // Can be kicked
        KOOPA_SHELL,    // Kicked shell (special mode)
        PIRANHA_PLANT,  // Vertical movement
    };

    explicit EnemyBehavior(EnemyType type = EnemyType::GOOMBA);
    virtual ~EnemyBehavior() = default;

    /**
     * Update enemy patrol behavior.
     * - Apply gravity
     * - Move in current direction
     * - Handle block collisions for direction changes
     * - Update animation frames
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Handle player collision.
     * - If from above: squish enemy (Goomba) or bounce (Koopa)
     * - If from side: hurt player (unless invincible star)
     * @return True if enemy was squished/defeated
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "EnemyBehavior"; }

   private:
    EnemyType m_Type;
    int m_DirectionChangeCounter = 0;  // For patrol behavior pattern
};

}  // namespace Mario

#endif  // MARIO_ENEMY_BEHAVIOR_HPP
