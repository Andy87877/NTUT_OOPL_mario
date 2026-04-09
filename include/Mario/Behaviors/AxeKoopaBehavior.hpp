/**
 * @file AxeKoopaBehavior.hpp
 * @brief Axe-throwing Koopa enemy behavior using Strategy pattern.
 *        Implements walking with periodic axe throwing attack.
 * @inheritance IEntityBehavior <- AxeKoopaBehavior
 */
#ifndef MARIO_AXE_KOOPA_BEHAVIOR_HPP
#define MARIO_AXE_KOOPA_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * AxeKoopaBehavior — Walking Koopa that throws axes periodically.
 * Attacks player at range with falling axe projectiles.
 * Cannot be jumped on (immune like Bowser fragments).
 *
 * Strategy Pattern Implementation:
 *  - Extends base patrol behavior
 *  - On timer, triggers axe spawn
 */
class AxeKoopaBehavior : public IEntityBehavior {
   public:
    AxeKoopaBehavior() = default;
    virtual ~AxeKoopaBehavior() = default;

    /**
     * Update Axe Koopa with walking and axe throw timer.
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Handle collision (immune to jump damage).
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "AxeKoopaBehavior"; }

   private:
    static constexpr float WALK_SPEED = 0.5f;
    static constexpr int AXE_THROW_INTERVAL = 150;  // ~2.5 sec at 60 FPS

    int m_ThrowTimer = 0;
};

}  // namespace Mario

#endif  // MARIO_AXE_KOOPA_BEHAVIOR_HPP
