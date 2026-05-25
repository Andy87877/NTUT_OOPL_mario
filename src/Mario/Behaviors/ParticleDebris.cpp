/**
 * @file ParticleDebris.cpp
 * @brief Implementation of ParticleDebris falling behavior
 * @inheritance IEntityBehavior -> ParticleDebris
 */
#include "Mario/Behaviors/ParticleDebris.hpp"

#include "Mario/Level/EntityState.hpp"

namespace Mario {

ParticleDebris::ParticleDebris()
    : m_InitialVelX(0),
      m_InitialVelY(0),
      m_LifetimeFrames(0),
      m_RotationAngle(0.0f) {}

void ParticleDebris::Update(EntityState& state, const Level& /*level*/,
                            const Player& /*player*/, int /*gameTimer*/) {
    if (m_LifetimeFrames == 0) {
        // Velocity is set at spawn time via SetInitialVelocity()
        state.SetVelX(m_InitialVelX);
        state.SetVelY(m_InitialVelY);
    }

    // Apply gravity manually (particles don't ignore gravity)
    m_RotationAngle += 15.0f;  // Track rotation angle for visual reference
    if (m_RotationAngle >= 360.0f) m_RotationAngle -= 360.0f;

    m_LifetimeFrames++;

    // Clean up if it falls off screen (safe assumption: low Y coordinate or
    // high lifetime limit)
    if (m_LifetimeFrames > 180 || state.GetY() < -100.0f) {
        state.Delete();
    }
}

bool ParticleDebris::OnPlayerCollision(EntityState& /*state*/,
                                       Player& /*player*/,
                                       bool /*isFromAbove*/) {
    // Debris doesn't interact with the player
    return false;
}

std::unique_ptr<IEntityBehavior> ParticleDebris::Clone() const {
    auto clone = std::make_unique<ParticleDebris>();
    clone->SetInitialVelocity(m_InitialVelX, m_InitialVelY);
    return clone;
}

}  // namespace Mario