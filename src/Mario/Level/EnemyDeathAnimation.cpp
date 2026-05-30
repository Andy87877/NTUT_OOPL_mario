/**
 * @file EnemyDeathAnimation.cpp
 * @brief Strategy implementations for enemy death animation behaviors.
 * @inheritance IEnemyDeathAnimation <- GoombaSquishDeathAnimation
 *              IEnemyDeathAnimation <- KoopaRetreatDeathAnimation
 *              IEnemyDeathAnimation <- FireballFlipDeathAnimation
 *              IEnemyDeathAnimation <- ClassicEnemyDeathAnimation
 */
#include "Mario/Level/EnemyDeathAnimation.hpp"

#include "Mario/Core/GameConfig.hpp"

namespace Mario {

void ClassicEnemyDeathAnimation::Start(EnemyDeathCause cause,
                                       EnemyDeathRuntime& runtime) {
    if (cause == EnemyDeathCause::STOMP || cause == EnemyDeathCause::GENERIC) {
        runtime.isEnemy = false;
        runtime.isSquashed = true;
        runtime.applyGravity = false;
        runtime.velX = 0.0f;
        runtime.velY = 0.0;
        m_DeleteAtFrame = runtime.currentFrameCounter + kQuickSquashFrames;
        m_Active = true;
        return;
    }

    // FIREBALL/SHELL/STAR-style flip fallback
    runtime.isEnemy = false;
    runtime.isSquashed = false;
    runtime.applyGravity = true;
    runtime.velX = (runtime.velX >= 0.0f) ? 1.6f : -1.6f;
    runtime.velY = -8.0;
    m_Active = true;
    m_DeleteAtFrame = runtime.currentFrameCounter + 45;
}

void ClassicEnemyDeathAnimation::Tick(EnemyDeathRuntime& runtime, float gravity,
                                      float tickInterval) {
    (void)gravity;
    (void)tickInterval;
    if (!m_Active) return;
    if (runtime.currentFrameCounter > m_DeleteAtFrame) {
        runtime.isActive = false;
        m_Active = false;
    }
}

void GoombaSquishDeathAnimation::Start(EnemyDeathCause cause,
                                        EnemyDeathRuntime& runtime) {
    if (cause == EnemyDeathCause::STOMP || cause == EnemyDeathCause::GENERIC) {
        runtime.isEnemy = false;
        runtime.isSquashed = true;
        runtime.applyGravity = false;
        runtime.velX = 0.0f;
        runtime.velY = 0.0;
        m_DeleteAtFrame = runtime.currentFrameCounter + kQuickSquashFrames;
        m_Active = true;
        return;
    }

    // Fireball/Shell/Star cause falls back to flip style in-place.
    runtime.isEnemy = false;
    runtime.isSquashed = false;
    runtime.applyGravity = true;
    runtime.velX = (runtime.velX >= 0.0f) ? 1.6f : -1.6f;
    runtime.velY = -8.0;
    m_DeleteAtFrame = runtime.currentFrameCounter + kFlipDespawnFrames;
    m_Active = true;
}

void GoombaSquishDeathAnimation::Tick(EnemyDeathRuntime& runtime, float gravity,
                                      float tickInterval) {
    if (!m_Active) return;

    if (!runtime.isSquashed) {
        runtime.velY += static_cast<double>(gravity) *
                        static_cast<double>(tickInterval) * 2.0;
        runtime.posY += static_cast<float>(runtime.velY);
    }

    if (runtime.currentFrameCounter > m_DeleteAtFrame) {
        runtime.isActive = false;
        m_Active = false;
    }
}

void KoopaRetreatDeathAnimation::Start(EnemyDeathCause cause,
                                       EnemyDeathRuntime& runtime) {
    if (cause == EnemyDeathCause::STOMP) {
        runtime.isSquashed = true;
        runtime.velX = 0.0f;
        runtime.velY = 0.0;
        runtime.applyGravity = true;
        m_Mode = Mode::SHELL;
        m_Active = false;
        return;
    }

    // Fire/star/shell-hit kill Koopa with flipped death arc.
    runtime.isEnemy = false;
    runtime.isSquashed = false;
    runtime.applyGravity = true;
    runtime.velX = (runtime.velX >= 0.0f) ? 1.8f : -1.8f;
    runtime.velY = -8.5;
    m_DeleteAtFrame = runtime.currentFrameCounter + kFlipDespawnFrames;
    m_Mode = Mode::FLIPPED;
    m_Active = true;
}

void KoopaRetreatDeathAnimation::Tick(EnemyDeathRuntime& runtime, float gravity,
                                      float tickInterval) {
    if (!m_Active) return;
    if (m_Mode != Mode::FLIPPED) {
        m_Active = false;
        return;
    }

    runtime.velY +=
        static_cast<double>(gravity) * static_cast<double>(tickInterval) * 2.0;
    runtime.posY += static_cast<float>(runtime.velY);

    if (runtime.currentFrameCounter > m_DeleteAtFrame) {
        runtime.isActive = false;
        m_Active = false;
        m_Mode = Mode::NONE;
    }
}

void FireballFlipDeathAnimation::Start(EnemyDeathCause cause,
                                       EnemyDeathRuntime& runtime) {
    (void)cause;
    runtime.isEnemy = false;
    runtime.isSquashed = false;
    runtime.applyGravity = true;
    runtime.velX = (runtime.velX >= 0.0f) ? 1.8f : -1.8f;
    runtime.velY = kInitialFlipVelY;
    m_DeleteAtFrame = runtime.currentFrameCounter + kFlipDespawnFrames;
    m_Active = true;
}

void FireballFlipDeathAnimation::Tick(EnemyDeathRuntime& runtime, float gravity,
                                      float tickInterval) {
    if (!m_Active) return;

    runtime.velY +=
        static_cast<double>(gravity) * static_cast<double>(tickInterval) * 2.0;
    runtime.posY += static_cast<float>(runtime.velY);

    if (runtime.currentFrameCounter > m_DeleteAtFrame) {
        runtime.isActive = false;
        m_Active = false;
    }
}

}  // namespace Mario