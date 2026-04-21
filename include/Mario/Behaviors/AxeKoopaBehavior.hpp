/**
 * @file AxeKoopaBehavior.hpp
 * @brief Axe-throwing Koopa enemy behavior using Strategy pattern.
 *        Implements walking with periodic axe throwing attack.
 * @inheritance IEntityBehavior <- AxeKoopaBehavior
 */
#ifndef MARIO_AXE_KOOPA_BEHAVIOR_HPP
#define MARIO_AXE_KOOPA_BEHAVIOR_HPP

#include <functional>

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * AxeKoopaBehavior — Walking Koopa that throws axes periodically.
 * Attacks player at range with falling axe projectiles.
 * Cannot be jumped on (immune like Bowser fragments).
 *
 * Strategy Pattern Implementation:
 *  - Extends base patrol behavior
 *  - On timer, triggers axe spawn via callback
 */
class AxeKoopaBehavior : public IEntityBehavior {
   public:
    // Callback type: Called to spawn axe at given position
    using AxeSpawnCallback = std::function<void(float x, float y)>;

    AxeKoopaBehavior() = default;
    virtual ~AxeKoopaBehavior() = default;

    /**
     * Set callback for axe spawning.
     * Called by App when attaching behavior to entity.
     */
    void SetAxeSpawnCallback(AxeSpawnCallback callback) {
        m_AxeSpawnCallback = callback;
    }

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
    AxeSpawnCallback m_AxeSpawnCallback;  // ✨ Callback for spawning axe
};

}  // namespace Mario

#endif  // MARIO_AXE_KOOPA_BEHAVIOR_HPP
