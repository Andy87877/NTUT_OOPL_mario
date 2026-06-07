/**
 * @file PlayerDeathAnimation.cpp
 * @brief Strategy implementations for player death animation motion.
 * @inheritance IPlayerDeathAnimation <- ClassicPlayerDeathAnimation <- TumblePlayerDeathAnimation
 */
#include "Mario/Player/PlayerDeathAnimation.hpp"

#include <algorithm>

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

void TumblePlayerDeathAnimation::Start() {
    ClassicPlayerDeathAnimation::Start();
    m_Rotation = 0.0f;
}

void TumblePlayerDeathAnimation::Tick(float gravity, float tickInterval,
                                      float jumpVelocity, float& playerY) {
    ClassicPlayerDeathAnimation::Tick(gravity, tickInterval, jumpVelocity, playerY);
    if (m_Active && m_Launched) {
        // Spin Mario in mid-air (approx 0.12 radians per frame)
        m_Rotation += 0.12f;
    }
}

float TumblePlayerDeathAnimation::GetScaleY() const {
    if (m_Active && m_Launched) {
        // Shrink Mario as he tumbles/falls into the abyss to look like he is fading away
        float scale = 1.0f - static_cast<float>(m_FrameCounter - kFreezeFrames) * 0.004f;
        return std::max(0.5f, scale);
    }
    return 1.0f;
}

}  // namespace Mario