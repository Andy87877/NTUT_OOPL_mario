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
#include "Mario/Services/AudioManager.hpp"
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

bool GoombaBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                       bool isFromAbove, [[maybe_unused]] GameStateManager& gameState,
                                       [[maybe_unused]] UIManager& uiManager, [[maybe_unused]] Camera& camera) {
    if (state.IsSquished() || state.IsDead()) {
        return false;
    }

    if (isFromAbove) {
        if (state.IsSquishable()) {
            state.Squish();
            AudioManager::GetInstance().PlaySFX(SFXName::Squish);
            return true;
        }
    } else {
        PlayerState& ps = player.GetState();
        if (!ps.IsInvincible()) {
            ps.TakeDamage();
        }
        return true;
    }
    return false;
}

AABB GoombaBehavior::GetHitbox(const EntityState& state) const {
    float w = static_cast<float>(state.GetWidth());
    float h = static_cast<float>(state.GetHeight());
    float hitW = w * 0.75f;
    float offsetX = (w - hitW) * 0.5f;
    return AABB::FromPosSize(state.GetX() + offsetX, state.GetY(), hitW, h);
}

std::unique_ptr<IEntityBehavior> GoombaBehavior::Clone() const {
    return std::make_unique<GoombaBehavior>(*this);
}

}  // namespace Mario
