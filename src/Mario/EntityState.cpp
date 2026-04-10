/**
 * @file EntityState.cpp
 * @brief Implementation of EntityState (Model layer).
 *        Contains entity physics, animation, squish/death logic.
 *        Ported from C# Entity.cs Update/Squish/Delete methods.
 * @inheritance None (pure Model)
 */
#include "Mario/EntityState.hpp"

#include <cmath>

#include "Mario/PhysicsEngine.hpp"

namespace Mario {

void EntityState::Init(const std::string& name, float worldX, float worldY,
                       int direction, bool isEnemy, bool isPowerUp, bool isCoin,
                       bool isStatic, bool doesCollide, bool squishable,
                       bool koopaSquash, bool doesJump, bool isBounce,
                       int scoreWorth, bool isAnimated, int animFrames,
                       int animBuffer, bool oneLoop, bool fromBlock,
                       int powerUpState) {
    m_Name = name;
    m_PosX = worldX;
    m_PosY = worldY;
    m_Direction = direction;
    m_IsEnemy = isEnemy;
    m_IsPowerUp = isPowerUp;
    m_IsCoin = isCoin;
    m_IsStatic = isStatic;
    m_DoesCollide = doesCollide;
    m_Squishable = squishable;
    m_KoopaSquash = koopaSquash;
    m_DoesJump = doesJump;
    m_IsBounce = isBounce;
    m_ScoreWorth = scoreWorth;
    m_IsAnimated = isAnimated;
    m_AnimFrames = animFrames;
    m_AnimBuffer = (animBuffer > 0) ? animBuffer : 1;
    m_OneLoop = oneLoop;
    m_FromBlock = fromBlock;
    m_PowerUpState = powerUpState;
    m_SizeX = GameConfig::TILE_SIZE;
    m_SizeY = GameConfig::TILE_SIZE;

    // Offset up if spawned from block (C# line 121: posY -= scaleSize)
    if (m_FromBlock) {
        m_PosY -= GameConfig::TILE_SIZE;
    }

    // Enemy/Item speed configuration (C# line 155)
    if (!m_IsStatic) {
        float baseSpeed = GameConfig::SCALED_SPEED;
        if (m_IsEnemy) {
            m_VelX = baseSpeed / GameConfig::ENEMY_SPEED_DIVISOR;
        } else if (m_IsPowerUp) {
            // Slow down power-up items (mushroom, fire flower, etc.)
            m_VelX = baseSpeed / GameConfig::ITEM_SPEED_DIVISOR;
        } else {
            m_VelX = baseSpeed;
        }

        // Flip if starting direction is left
        if (m_Direction == 0) {
            m_VelX = -m_VelX;
        } else if (m_Direction == 2) {
            m_VelX = 0.0f;
        }
    }

    if (m_DoesJump) {
        Jump();
    }
}

void EntityState::Tick() {
    if (!m_Active) return;

    m_ActiveCounter++;

    // Death timer
    if (m_DeathActive) {
        if (m_ActiveCounter > m_SquishCounter) {
            Delete();
        }
        return;
    }

    // Animation
    if (m_IsAnimated && !m_Squashed) {
        m_AnimBufferCount++;
        if (m_AnimBufferCount >= m_AnimBuffer) {
            m_AnimBufferCount = 0;
            m_CurrentFrame++;
            if (m_CurrentFrame >= m_AnimFrames) {
                if (m_OneLoop) {
                    Delete();
                } else {
                    m_CurrentFrame = 0;
                }
            }
        }
    }

    // Apply movement if not static and not squashed
    if (!m_IsStatic && !m_Squashed) {
        m_PosX += m_VelX;

        // Apply gravity
        float yDelta = ApplyGravity();
        m_PosY += yDelta;
    }
}

void EntityState::FlipDirection() {
    m_VelX = -m_VelX;
    if (m_VelX > 0)
        m_Direction = 1;
    else if (m_VelX < 0)
        m_Direction = 0;
}

void EntityState::Squish() {
    if (m_KoopaSquash) {
        // Koopa turns into shell
        m_Squashed = true;
        m_VelX = 0.0f;
        m_SquishCounter = m_ActiveCounter + 300;  // Shell stays 6 seconds
        m_DeathActive = false;                    // Shell doesn't auto-die
    } else if (m_Squishable) {
        // Goomba squish
        m_Squashed = true;
        m_VelX = 0.0f;
        m_SquishCounter = m_ActiveCounter + 30;  // Squish sprite for 0.6s
        m_DeathActive = true;
    }
}

void EntityState::KickShell(float speed) {
    if (m_Squashed) {
        m_VelX = speed * GameConfig::SHELL_SPEED_MULTIPLIER;
        m_Squashed = false;
    }
}

void EntityState::Delete() { m_Active = false; }

void EntityState::Jump() {
    m_FallHeight = PhysicsEngine::GetJumpHeight(0);
    m_IsGrounded = false;
}

AABB EntityState::GetHitbox() const {
    float w = static_cast<float>(GetWidth());
    float h = static_cast<float>(GetHeight());
    return AABB::FromPosSize(m_PosX, m_PosY, w, h);
}

float EntityState::ApplyGravity() {
    return PhysicsEngine::ApplyGravity(m_FallHeight, m_VelY, m_IsGrounded);
}

}  // namespace Mario
