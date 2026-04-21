/**
 * @file ParaKoopaBehavior.cpp
 * @brief Implementation of Paratroopa floating behavior with landing mechanic.
 * @inheritance IEntityBehavior <- ParaKoopaBehavior
 */
#include "Mario/Behaviors/ParaKoopaBehavior.hpp"

#include <cmath>

#include "Mario/EntityState.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"

namespace Mario {

void ParaKoopaBehavior::Update(EntityState& state, const Level& level,
                               const Player& /* player */, int gameTimer) {
    // Initialize original Y on first frame
    if (m_OriginalY == 0.0f && m_FloatPhase == 0.0f) {
        m_OriginalY = state.GetWorldY();
    }

    if (m_IsFlying) {
        // App.cpp's Phase 6 applied gravity to this entity.
        // We override this by resetting VelY to 0 and enforcing our sine-wave
        // Y.
        state.SetVelY(0.0f);
        state.SetFallHeight(0.0f);
        state.SetGrounded(false);

        // -- Floating behavior (oscillate vertically with sine wave) --
        m_FloatPhase += FLOAT_FREQUENCY * 0.016f;  // ~60fps delta
        if (m_FloatPhase > 2.0f * 3.14159f) {
            m_FloatPhase -= 2.0f * 3.14159f;
        }

        // Calculate offset from original Y position
        float offset = FLOAT_AMPLITUDE * std::sin(m_FloatPhase);
        float newY = m_OriginalY + offset;
        state.SetWorldY(newY);

        // Horizontal movement is handled by App.cpp (VelX is added
        // automatically)
    } else {
        // -- Grounded behavior (normal gravity like regular Koopa) --
        // Gravity and Position updates are handled completely by App.cpp.
        // Wall collisions and ground bounds are also managed by
        // App::CheckEntityBlockCollision. We just need to let the entity act
        // normally!
    }

    // Animation update every 10 ticks
    if (gameTimer % 10 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool ParaKoopaBehavior::OnPlayerCollision(EntityState& state,
                                          Player& /* player */,
                                          bool isFromAbove) {
    if (isFromAbove && m_IsFlying) {
        // Player jumped on flying Koopa - lose wings and fall to ground
        m_IsFlying = false;
        m_FloatPhase = 0.0f;
        state.SetVelY(0.0f);
        state.SetFallHeight(0.0);
        return true;  // Collision handled
    } else if (isFromAbove && !m_IsFlying) {
        // Grounded Para-Koopa can be squished like normal enemy
        return true;  // Collision handled
    }

    return false;
}

std::unique_ptr<IEntityBehavior> ParaKoopaBehavior::Clone() const {
    return std::make_unique<ParaKoopaBehavior>(*this);
}

}  // namespace Mario
