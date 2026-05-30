/**
 * @file EntityState.cpp
 * @brief Implementation of EntityState (Model layer).
 *        Contains entity physics, animation, squish/death logic.
 *        Ported from C# Entity.cs Update/Squish/Delete methods.
 * @inheritance None (pure Model)
 */
#include "Mario/Level/EntityState.hpp"

#include <cmath>

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Core/PhysicsEngine.hpp"

namespace Mario {

EntityState::EntityState()
    : m_DeathAnimation(std::make_unique<ClassicEnemyDeathAnimation>()) {}

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
    m_Active = true;
    m_IsGrounded = false;
    m_Squashed = false;
    m_CurrentFrame = 0;
    m_AnimBufferCount = 0;
    m_ActiveCounter = 0;
    m_Hidden = false;
    m_VelY = 0.0;
    m_FallHeight = 0.0;
    m_DeathAnimation = std::make_unique<ClassicEnemyDeathAnimation>();

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

    // Death animation timer (strategy-driven)
    if (m_DeathAnimation && m_DeathAnimation->IsActive()) {
        EnemyDeathRuntime runtime{m_ActiveCounter, m_IsEnemy, m_Squashed,
                                  m_ApplyGravity,  m_VelX,    m_VelY,
                                  m_PosY,          m_Active};
        m_DeathAnimation->Tick(runtime, GameConfig::GRAVITY,
                               GameConfig::TICK_INTERVAL);
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

    // Apply movement if not static and not in squash-pause.
    // Koopa shell mode (koopaSquash + squashed) should still move when kicked.
    if (!m_IsStatic && (!m_Squashed || m_KoopaSquash)) {
        m_PosX += m_VelX;

        // Apply gravity if enabled; otherwise, allow gravity-free diagonal movement via VelY
        if (m_ApplyGravity) {
            float yDelta = ApplyGravity();
            m_PosY += yDelta;
        } else {
            m_PosY += static_cast<float>(m_VelY);
        }
    }
}

void EntityState::FlipDirection() {
    m_VelX = -m_VelX;
    if (m_VelX > 0)
        m_Direction = 1;
    else if (m_VelX < 0)
        m_Direction = 0;
}

void EntityState::Squish() { TriggerDeath(EnemyDeathCause::STOMP); }

void EntityState::TriggerDeath(EnemyDeathCause cause) {
    if (!m_Active || !m_DeathAnimation || m_DeathAnimation->IsActive()) return;

    // Koopa stomp uses in-place shell conversion (no new entity spawn).
    // Match classic feel by shrinking to 1-tile shell and dropping Y so the
    // shell sits on the same ground contact point instead of floating mid-air.
    if (cause == EnemyDeathCause::STOMP && m_KoopaSquash && !m_Squashed) {
        if (m_SizeY > GameConfig::TILE_SIZE) {
            m_PosY += static_cast<float>(m_SizeY - GameConfig::TILE_SIZE);
        }
        m_SizeX = GameConfig::TILE_SIZE;
        m_SizeY = GameConfig::TILE_SIZE;
    }

    // If it's not a Koopa squishing in-place into a shell, the enemy is completely
    // dead and should fall through blocks off the screen. Disable collision!
    if (!(cause == EnemyDeathCause::STOMP && m_KoopaSquash)) {
        m_DoesCollide = false;
    }

    EnemyDeathRuntime runtime{m_ActiveCounter, m_IsEnemy, m_Squashed,
                              m_ApplyGravity,  m_VelX,    m_VelY,
                              m_PosY,          m_Active};
    m_DeathAnimation->Start(cause, runtime);
}

void EntityState::SetDeathAnimationStrategy(
    std::unique_ptr<IEnemyDeathAnimation> strategy) {
    if (!strategy) return;
    m_DeathAnimation = std::move(strategy);
}

void EntityState::KickShell(float speed) {
    if (m_Squashed) {
        m_VelX = speed * GameConfig::SHELL_SPEED_MULTIPLIER;
        if (!m_KoopaSquash) {
            m_Squashed = false;
        }
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Kick);
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

// -- Gravity --
float EntityState::ApplyGravity() {
    if (!m_ApplyGravity) {
        return 0.0f;
    }
    if (m_IsGrounded) {
        m_VelY = 0.0;
        m_FallHeight = 0.0;
        return 0.0f;
    }
    m_FallHeight -= GameConfig::GRAVITY * GameConfig::TICK_INTERVAL * 4.0;
    m_VelY = -m_FallHeight;
    return static_cast<float>(m_VelY);
}

}  // namespace Mario
