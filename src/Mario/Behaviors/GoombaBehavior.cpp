/**
 * @file GoombaBehavior.cpp
 * @brief Standard C# patrol AI implementation for Goombas.
 *        Moves back and forth, turning on wall collisions.
 * @inheritance IEntityBehavior -> GoombaBehavior
 */
#include "Mario/Behaviors/GoombaBehavior.hpp"

#include <cmath>

#include "Mario/Core/Collider.hpp"
#include "Mario/Level/EntityState.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/Player/Player.hpp"
#include "Mario/Player/PlayerState.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void GoombaBehavior::Update(EntityState& state, const Level& level,
                            const Player& player, int gameTimer) {
    (void)level;
    (void)player;
    (void)gameTimer;

    if (state.IsSquished() || state.IsDead()) {
        if (!state.IsAnimated()) {
            state.Delete();
        }
        return;
    }

    // Standard walk patrol speed (no smart AI pursuit, dodge hops, or cliff-awareness)
    float baseSpeed = GameConfig::SCALED_SPEED / GameConfig::ENEMY_SPEED_DIVISOR;
    state.SetVelX(state.GetDirection() == 1 ? baseSpeed : -baseSpeed);

    // Advance animation frame every 10 ticks
    m_DirectionChangeCounter++;
    if (state.IsAnimated() && m_DirectionChangeCounter % 10 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool GoombaBehavior::OnPlayerCollision(EntityState& state, [[maybe_unused]] Player& player,
                                       bool isFromAbove) {
    if (state.IsSquished() || state.IsDead()) {
        return false;
    }

    if (isFromAbove && state.IsSquishable()) {
        state.Squish();
        return true;
    }
    return false;
}

std::unique_ptr<IEntityBehavior> GoombaBehavior::Clone() const {
    return std::make_unique<GoombaBehavior>(*this);
}

}  // namespace Mario
