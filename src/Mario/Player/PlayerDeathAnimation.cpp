/**
 * @file PlayerDeathAnimation.cpp
 * @brief Strategy implementations for player death animation motion.
 * @inheritance IPlayerDeathAnimation <- ClassicPlayerDeathAnimation
 */
#include "Mario/Player/PlayerDeathAnimation.hpp"

namespace Mario {

void ClassicPlayerDeathAnimation::Start() {
    m_Active = true;
    m_Launched = false;
    m_FrameCounter = 0;
    m_VelY = 0.0;
}

void ClassicPlayerDeathAnimation::Tick(float gravity, float tickInterval,
                                       float jumpVelocity, float& playerY) {
    if (!m_Active) return;

    ++m_FrameCounter;
    if (!m_Launched) {
        if (m_FrameCounter >= kFreezeFrames) {
            m_Launched = true;
            m_VelY = -static_cast<double>(jumpVelocity) * kLaunchMultiplier;
        }
        return;
    }

    m_VelY += static_cast<double>(gravity) * static_cast<double>(tickInterval) *
              kGravityMultiplier;
    playerY += static_cast<float>(m_VelY);
}

}  // namespace Mario