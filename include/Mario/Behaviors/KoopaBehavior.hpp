/**
 * @file KoopaBehavior.hpp
 * @brief Behavior for Koopa Troopa and its shell form.
 *        Handles:
 *        - KoopaTroopa: Patrol + wall/pit collision direction changes
 *        - KoopaTroopaShell: Static movement (controlled by parent App
 * velocity)
 * @inheritance IEntityBehavior
 */
#ifndef MARIO_KOOPA_BEHAVIOR_HPP
#define MARIO_KOOPA_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Koopa Troopa exclusive behavior
 * Core difference from EnemyBehavior: used for Koopa-type entities.
 * Shell spawning is handled by App::CheckPlayerEntityCollision
 */
class KoopaBehavior : public IEntityBehavior {
   public:
    enum class KoopaType {
        TROOPA,  // Living Koopa (patrol movement)
        SHELL,   // Shell form (passive, moved only via external velocity)
    };

    explicit KoopaBehavior(KoopaType type = KoopaType::TROOPA);
    virtual ~KoopaBehavior() = default;

    /**
     * Update Koopa movement.
     * - TROOPA: Patrol + direction change on wall/pit
     * - SHELL: Apply gravity only (App controls horizontal velocity)
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Handle player collision
     * Shell spawning happens in App layer
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "KoopaBehavior"; }

   private:
    KoopaType m_Type;
    int m_DirectionChangeCounter = 0;
    int m_LastDirectionChangeFrame = -100;
};

}  // namespace Mario

#endif  // MARIO_KOOPA_BEHAVIOR_HPP
