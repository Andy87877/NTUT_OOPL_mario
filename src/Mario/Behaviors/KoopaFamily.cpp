/**
 * @file KoopaFamily.cpp
 * @brief Implementations for all Koopa-family enemy behaviors.
 *        Merged from KoopaBehavior.cpp, AxeKoopaBehavior.cpp,
 *        ParaKoopaBehavior.cpp — all Koopa-type variants, no external
 *        consumers outside the entity sub-system.
 * @inheritance IEntityBehavior <- KoopaBehavior
 *              IEntityBehavior <- AxeKoopaBehavior
 *              IEntityBehavior <- ParaKoopaBehavior
 */
#include "Mario/Behaviors/KoopaFamily.hpp"

#include <cmath>

#include "Mario/Collider.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// ============================================================================
// KoopaBehavior
// ============================================================================

KoopaBehavior::KoopaBehavior(KoopaType type) : m_Type(type) {}

void KoopaBehavior::Update(EntityState& state,
                           [[maybe_unused]] const Level& level,
                           [[maybe_unused]] const Player& player,
                           [[maybe_unused]] int gameTimer) {
    if (state.IsSquished() || state.IsDead()) {
        if (!state.IsAnimated()) state.Delete();
        return;
    }

    m_DirectionChangeCounter++;
    if (state.IsAnimated() && m_DirectionChangeCounter % 10 == 0) {
        state.AdvanceAnimationFrame();
    }

    if (m_Type == KoopaType::SHELL) {
        return;  // Shell stays still until kicked by App
    }
}

bool KoopaBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                      [[maybe_unused]] Player& player,
                                      [[maybe_unused]] bool isFromAbove) {
    // Shell spawning handled by App::CheckPlayerEntityCollision
    return false;
}

std::unique_ptr<IEntityBehavior> KoopaBehavior::Clone() const {
    return std::make_unique<KoopaBehavior>(*this);
}

// ============================================================================
// AxeKoopaBehavior
// ============================================================================

void AxeKoopaBehavior::Update(EntityState& state, const Level& level,
                              [[maybe_unused]] const Player& player,
                              int gameTimer) {
    if (state.IsSquished() || state.IsDead()) return;

    m_ThrowTimer++;

    if (m_ThrowTimer >= AXE_THROW_INTERVAL) {
        if (m_AxeSpawnCallback) {
            float axeX = state.GetX() + state.GetWidth() / 2.0f;
            float axeY = state.GetY() + state.GetHeight();
            m_AxeSpawnCallback(axeX, axeY);
        }
        m_ThrowTimer = 0;
    }

    // Wall bounce (grid-based)
    AABB enemyBox = state.GetCollider();
    int topTile = static_cast<int>(enemyBox.top) / GameConfig::TILE_SIZE;
    int bottomTile =
        static_cast<int>(enemyBox.bottom - 1) / GameConfig::TILE_SIZE;
    bool hitWall = false;

    if (state.GetDirection() == 1) {
        int rightTile =
            static_cast<int>(enemyBox.right) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile && !hitWall; y++) {
            const Block* block = level.GetBlockAt(rightTile, y);
            if (block && block->IsSolid() &&
                enemyBox.Intersects(block->GetAABB()))
                hitWall = true;
        }
    } else {
        int leftTile = static_cast<int>(enemyBox.left) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile && !hitWall; y++) {
            const Block* block = level.GetBlockAt(leftTile, y);
            if (block && block->IsSolid() &&
                enemyBox.Intersects(block->GetAABB()))
                hitWall = true;
        }
    }

    if (hitWall) {
        state.SetDirection(state.GetDirection() == 1 ? 0 : 1);
        state.SetVelX(-state.GetVelX());
    }

    if (state.IsAnimated() && gameTimer % 10 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool AxeKoopaBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                         [[maybe_unused]] Player& player,
                                         [[maybe_unused]] bool isFromAbove) {
    // Immune to stomp — only defeated by axe trap mechanism
    return false;
}

std::unique_ptr<IEntityBehavior> AxeKoopaBehavior::Clone() const {
    return std::make_unique<AxeKoopaBehavior>(*this);
}

// ============================================================================
// ParaKoopaBehavior
// ============================================================================

void ParaKoopaBehavior::Update(EntityState& state,
                               [[maybe_unused]] const Level& level,
                               [[maybe_unused]] const Player& player,
                               int gameTimer) {
    if (m_OriginalY == 0.0f && m_FloatPhase == 0.0f) {
        m_OriginalY = state.GetWorldY();
    }

    if (m_IsFlying) {
        state.SetVelY(0.0f);
        state.SetFallHeight(0.0f);
        state.SetGrounded(false);

        m_FloatPhase += FLOAT_FREQUENCY * 0.016f;
        if (m_FloatPhase > 2.0f * 3.14159f) m_FloatPhase -= 2.0f * 3.14159f;

        float newY = m_OriginalY + FLOAT_AMPLITUDE * std::sin(m_FloatPhase);
        state.SetWorldY(newY);
    }
    // Grounded: App handles gravity / block collision entirely

    if (gameTimer % 10 == 0) state.AdvanceAnimationFrame();
}

bool ParaKoopaBehavior::OnPlayerCollision(EntityState& state,
                                          [[maybe_unused]] Player& player,
                                          bool isFromAbove) {
    if (isFromAbove && m_IsFlying) {
        m_IsFlying = false;
        m_FloatPhase = 0.0f;
        state.SetVelY(0.0f);
        state.SetFallHeight(0.0);
        return true;
    }
    if (isFromAbove && !m_IsFlying) return true;
    return false;
}

std::unique_ptr<IEntityBehavior> ParaKoopaBehavior::Clone() const {
    return std::make_unique<ParaKoopaBehavior>(*this);
}

}  // namespace Mario
