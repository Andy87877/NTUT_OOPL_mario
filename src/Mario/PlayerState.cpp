/**
 * @file PlayerState.cpp
 * @brief Implementation of PlayerState (Model layer).
 *        All game logic: physics, power-up transitions, animation key
 * generation. Matches the C# Player.cs logic.
 * @inheritance None (pure Model)
 */
#include "Mario/PlayerState.hpp"

#include <algorithm>
#include <cmath>

#include "Mario/AudioManager.hpp"
#include "Mario/PhysicsEngine.hpp"

namespace Mario {

PlayerState::PlayerState() = default;

void PlayerState::Init(float worldX, float worldY, int startState) {
    m_PosX = worldX;
    m_PosY = worldY;
    m_VelX = 0.0f;
    m_VelY = 0.0;
    m_FallHeight = 0.0;
    m_PowerState = static_cast<PowerState>(startState);
    m_Grounded = false;
    m_Crouching = false;
    m_MovingRight = false;
    m_MovingLeft = false;
    m_FacingRight = true;
    m_Running = false;
    m_Dead = false;
    m_Controllable = true;
    m_PoleSliding = false;
    m_AnimFrame = 0;
    m_AnimTimer = 0;
    m_InvTimer = -1;
    m_StarTimer = 0;
    m_SpecialActive = false;
}

void PlayerState::Tick() {
    // Update invincibility timer
    if (m_InvTimer > 0) {
        m_InvTimer--;
    }

    // Star timer
    if (m_StarTimer > 0) {
        m_StarTimer--;
        if (m_StarTimer <= 0) {
            // Revert from star state
            if (m_PowerState == PowerState::SMALL_STAR) {
                m_PowerState = PowerState::SMALL;
            } else if (m_PowerState == PowerState::BIG_STAR) {
                m_PowerState = PowerState::BIG;
            }
        }
    }

    // Fire shooting cooldown
    if (m_SpecialActive) {
        m_SpecialCounter++;
        if (m_SpecialCounter >= m_SpecialLength) {
            m_SpecialActive = false;
            m_SpecialCounter = 0;
        }
    }

    // Animation timer (boomerang effect from C# reference)
    m_AnimTimer++;
    if (m_AnimTimer >= m_TargetRate) {
        m_AnimTimer = 0;
        if (!m_BoomerangAnim) {
            if (m_AnimFrame >= m_TotalFrames) {
                m_BoomerangAnim = true;
                m_AnimFrame--;
            } else {
                m_AnimFrame++;
            }
        } else {
            if (m_AnimFrame == 0) {
                m_BoomerangAnim = false;
            } else {
                m_AnimFrame--;
            }
        }
        if (m_AnimFrame < 0) {
            m_AnimFrame = 0;
        }
    }

    // Set totalFrames based on current state (matching C# Update logic)
    if (m_PoleSliding) {
        m_TotalFrames = 0;
    } else if (!m_SpecialActive) {
        if (m_Grounded) {
            if (m_MovingRight || m_MovingLeft) {
                m_TotalFrames = 2;  // Walk cycle: 3 frames (0,1,2)
            } else {
                m_TotalFrames = 0;  // Idle or crouch: single frame
                m_AnimFrame = 0;
            }
        } else {
            m_TotalFrames = 0;  // Jump: single frame
            m_AnimFrame = 0;
        }
    } else {
        m_TotalFrames = 0;
        m_AnimFrame = 0;
    }

    // Update facing direction
    if (m_MovingRight) m_FacingRight = true;
    if (m_MovingLeft) m_FacingRight = false;
}

void PlayerState::SetJumping(bool v) {
    if (v && m_Grounded && !m_Dead) {
        m_Grounded = false;
        m_FallHeight = PhysicsEngine::GetJumpHeight(0);
        if (m_PowerState == PowerState::SMALL) {
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Jump);
        } else {
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::BigJump);
        }
    }
}

void PlayerState::ApplyMovement(float speed) {
    if (!m_Controllable || m_Dead) {
        m_VelX = 0.0f;
        return;
    }

    if (m_MovingRight) {
        float s = m_Running ? speed * GameConfig::RUN_MULTIPLIER : speed;
        m_VelX = s;
    } else if (m_MovingLeft) {
        float s = m_Running ? speed * GameConfig::RUN_MULTIPLIER : speed;
        // Prevent going past left boundary
        if (m_PosX >= 5.0f) {
            m_VelX = -s;
        } else {
            m_VelX = 0.0f;
        }
    } else {
        m_VelX = 0.0f;
    }
}

float PlayerState::ApplyGravity() {
    return PhysicsEngine::ApplyGravity(m_FallHeight, m_VelY, m_Grounded);
}

void PlayerState::SetPowerState(PowerState ps) { m_PowerState = ps; }

void PlayerState::TakeDamage() {
    if (m_InvTimer > 0) return;  // Already invincible
    if (m_Dead) return;

    if (m_PowerState == PowerState::SMALL ||
        m_PowerState == PowerState::SMALL_STAR) {
        m_Dead = true;
        m_Controllable = false;
    } else {
        // Big/Fire -> Small with invincibility frames
        m_PowerState = PowerState::SMALL;
        m_InvTimer = 60;  // ~1.2 seconds of invincibility

        // When shrinking from big to small, adjust Y downwards
        // (big=90px height, small=45px height, so move down by 45px to stay on
        // ground)
        if (!m_Crouching) {
            SetY(GetY() + GameConfig::TILE_SIZE);
        }
    }
}

void PlayerState::PowerUp(PowerState newState) { m_PowerState = newState; }

void PlayerState::StartStar() {
    if (m_PowerState == PowerState::SMALL) {
        m_PowerState = PowerState::SMALL_STAR;
    } else {
        m_PowerState = PowerState::BIG_STAR;
    }
    m_StarTimer = 500;  // ~10 seconds
}

int PlayerState::GetWidth() const { return GameConfig::TILE_SIZE; }

int PlayerState::GetHeight() const {
    if (m_PowerState == PowerState::BIG || m_PowerState == PowerState::FIRE ||
        m_PowerState == PowerState::BIG_STAR) {
        return m_Crouching ? GameConfig::TILE_SIZE : GameConfig::TILE_SIZE * 2;
    }
    return GameConfig::TILE_SIZE;
}

AABB PlayerState::GetHitbox() const {
    // Hitbox is slightly thinner (0.6875 ratio, matching C# reference)
    float w = static_cast<float>(GetWidth()) * GameConfig::HITBOX_WIDTH_RATIO;
    float h = static_cast<float>(GetHeight());
    float offsetX = (static_cast<float>(GetWidth()) - w) / 2.0f;
    return AABB::FromPosSize(m_PosX + offsetX, m_PosY, w, h);
}

std::string PlayerState::GetAnimPrefix() const {
    if (m_PoleSliding) return "Pole";

    if (m_SpecialActive) return "Fire";

    if (m_Grounded) {
        if (m_MovingRight || m_MovingLeft) return "Right";
        if (m_Crouching) return "Crouch";
        return "Idle";
    }
    return "Jump";
}

void PlayerState::SetFireShooting(bool v) {
    if (v && m_PowerState == PowerState::FIRE && !m_SpecialActive) {
        m_SpecialActive = true;
        m_SpecialCounter = 0;
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::FireBall);
    }
}

}  // namespace Mario
