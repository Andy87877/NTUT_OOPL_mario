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
#include <cstdlib>

#include "Mario/Core/Collider.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/Level/EntityState.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Player/Player.hpp"
#include "Mario/Services/AudioManager.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// ============================================================================
// KoopaBehavior
// ============================================================================

KoopaBehavior::KoopaBehavior(KoopaType type) : m_Type(type) {}

void KoopaBehavior::Update(EntityState& state, const Level& level,
                           const Player& player,
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

    // -------------------------------------------------------------
    // AI for Living Koopa Troopa (TROOPA)
    // -------------------------------------------------------------
    float baseSpeed =
        GameConfig::SCALED_SPEED / GameConfig::ENEMY_SPEED_DIVISOR;
    state.SetVelX(state.GetDirection() == 1 ? baseSpeed : -baseSpeed);

    // Ledge/Cliff Awareness
    // RED_TROOPA (red shell) turns around at cliffs; TROOPA (green) walks off.
    // The type is set by EntityFactory from CSV entity name — no string
    // comparison needed at runtime here (OCP / SRP).
    bool avoidsLedges = (m_Type == KoopaType::RED_TROOPA);
    if (avoidsLedges && state.IsGrounded()) {
        float checkX = (state.GetVelX() > 0.0f)
                           ? state.GetWorldX() + state.GetWidth() + 4.0f
                           : state.GetWorldX() - 4.0f;
        float checkY = state.GetWorldY() + state.GetHeight() + 4.0f;

        const Block* nextGround = level.GetBlockAtWorld(checkX, checkY);
        if (!nextGround || !nextGround->IsSolid()) {
            state.FlipDirection();
            LOG_INFO("Koopa: Red Koopa turning back at ledge.");
            return;
        }
    }
}

bool KoopaBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                      bool isFromAbove, [[maybe_unused]] GameStateManager& gameState,
                                      [[maybe_unused]] UIManager& uiManager, [[maybe_unused]] Camera& camera) {
    if (state.IsDead()) return false;

    PlayerState& ps = player.GetState();
    if (isFromAbove) {
        if (m_Type == KoopaType::TROOPA || m_Type == KoopaType::RED_TROOPA) {
            m_Type = KoopaType::SHELL;
            state.SetName("KoopaTroopaShell");
            state.SetSquashed(true);
            state.SetCollidable(true);
            state.SetVelY(0.0f);
            state.SetFallHeight(0.0f);
            state.SetGrounded(false);
            state.SetVelX(0.0f);
            AudioManager::GetInstance().PlaySFX(SFXName::Squish);
            return true;
        } else if (m_Type == KoopaType::SHELL) {
            // Kick the shell
            AABB playerBox = ps.GetHitbox();
            AABB entityBox = state.GetHitbox();
            float playerCX = playerBox.left + (playerBox.right - playerBox.left) * 0.5f;
            float shellCX = entityBox.left + (entityBox.right - entityBox.left) * 0.5f;
            float kickSpeed = (playerCX > shellCX) ? -GameConfig::SCALED_SPEED : GameConfig::SCALED_SPEED;
            state.KickShell(kickSpeed);
            ps.SetInvTimer(5);
            return true;
        }
    } else {
        // Side collision
        if (m_Type == KoopaType::SHELL) {
            if (std::abs(state.GetVelX()) < 0.1f) {
                // Stationary shell: kick it
                AABB playerBox = ps.GetHitbox();
                AABB entityBox = state.GetHitbox();
                float playerCX = playerBox.left + (playerBox.right - playerBox.left) * 0.5f;
                float shellCX = entityBox.left + (entityBox.right - entityBox.left) * 0.5f;
                float kickSpeed = (playerCX > shellCX) ? -GameConfig::SCALED_SPEED : GameConfig::SCALED_SPEED;
                state.KickShell(kickSpeed);
                ps.SetInvTimer(5);
            } else {
                // Moving shell: damage player
                if (!ps.IsInvincible()) {
                    ps.TakeDamage();
                }
            }
            return true;
        } else {
            // Normal living Koopa
            if (!ps.IsInvincible() && !state.IsSquished()) {
                ps.TakeDamage();
            }
            return true;
        }
    }
    return false;
}

std::unique_ptr<IEntityBehavior> KoopaBehavior::Clone() const {
    return std::make_unique<KoopaBehavior>(*this);
}

AABB KoopaBehavior::GetHitbox(const EntityState& state) const {
    float w = static_cast<float>(state.GetWidth());
    float h = static_cast<float>(state.GetHeight());
    float hitW = w * 0.75f;
    float offsetX = (w - hitW) * 0.5f;
    return AABB::FromPosSize(state.GetX() + offsetX, state.GetY(), hitW, h);
}

bool KoopaBehavior::IsShell() const { return m_Type == KoopaType::SHELL; }

float KoopaBehavior::GetVisualYOffset(const std::string& levelName) const {
    (void)levelName;
    return 0.0f;  // Now handled dynamically at View layer (Entity::UpdateView)
}

// ============================================================================
// AxeKoopaBehavior
// ============================================================================

void AxeKoopaBehavior::Update(EntityState& state, const Level& level,
                              const Player& player,
                              int gameTimer) {
    if (state.IsSquished() || state.IsDead()) return;

    // 1. Sync patrol direction from current velocity (allows physical wall bounces from CheckWalls to propagate)
    if (state.GetVelX() > 0.0f) {
        m_PatrolDirection = 1;
    } else if (state.GetVelX() < 0.0f) {
        m_PatrolDirection = -1;
    } else if (!m_PatrolInitialized) {
        // Initial setup
        m_PatrolDirection = (state.GetDirection() == 0) ? -1 : 1;
        m_PatrolInitialized = true;
    }

    // 2. Active player tracking (Always face the player like Bowser)
    int facingDir = (player.GetWorldX() < state.GetWorldX()) ? 0 : 1;
    state.SetDirection(facingDir);

    // 3. Periodic axe-throw projectile attack
    m_ThrowTimer++;
    if (m_ThrowTimer >= AXE_THROW_INTERVAL) {
        // Set pending spawn — consumed by PlayingSceneHandler via
        // ConsumeSpawnRequest(), same pattern as BowserBehavior.
        m_AxePending = true;
        m_AxeX = state.GetX() + state.GetWidth() * 0.5f;
        // Throw upward: spawn 1 tile above AxeKoopa's top edge
        m_AxeY = state.GetY() - GameConfig::TILE_SIZE;
        m_AxeDir = state.GetDirection();  // Faces player
        m_ThrowTimer = 0;
    }

    // 3b. Periodic hopping behavior (every ~3 seconds, hop up/down to feel smart and lively)
    m_HopTimer++;
    if (m_HopTimer >= 150) {
        m_HopTimer = 0;
        if (state.IsGrounded() && (std::rand() % 2 == 0)) {
            state.SetGrounded(false);
            state.SetFallHeight(12.0f);  // Energetic jump to land on platforms or jump in place
        }
    }

    // 4. Ledge/Cliff awareness and pit/lava avoidance
    if (state.IsGrounded()) {
        float checkX = (m_PatrolDirection > 0)
                           ? state.GetWorldX() + state.GetWidth() + 4.0f
                           : state.GetWorldX() - 4.0f;
        float checkY = state.GetWorldY() + state.GetHeight() + 4.0f;

        const Block* nextGround = level.GetBlockAtWorld(checkX, checkY);
        if (!nextGround || !nextGround->IsSolid()) {
            m_PatrolDirection *= -1;
        }
    }

    // 5. Decoupled patrol movement
    float walkSpeed = GameConfig::SCALED_SPEED / GameConfig::ENEMY_SPEED_DIVISOR;
    state.SetVelX(static_cast<float>(m_PatrolDirection) * walkSpeed);

    if (state.IsAnimated() && gameTimer % 10 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool AxeKoopaBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state, Player& player,
                                         [[maybe_unused]] bool isFromAbove, [[maybe_unused]] GameStateManager& gameState,
                                         [[maybe_unused]] UIManager& uiManager, [[maybe_unused]] Camera& camera) {
    PlayerState& ps = player.GetState();
    if (!ps.IsInvincible()) {
        ps.TakeDamage();
    }
    return true;
}

std::unique_ptr<IEntityBehavior> AxeKoopaBehavior::Clone() const {
    return std::make_unique<AxeKoopaBehavior>(*this);
}

AABB AxeKoopaBehavior::GetHitbox(const EntityState& state) const {
    float w = static_cast<float>(state.GetWidth());
    float h = static_cast<float>(state.GetHeight());
    float hitW = w * 0.75f;
    float offsetX = (w - hitW) * 0.5f;
    return AABB::FromPosSize(state.GetX() + offsetX, state.GetY(), hitW, h);
}

bool AxeKoopaBehavior::ConsumeSpawnRequest(EntityType& outType, float& outX,
                                           float& outY, int& outDir) {
    if (!m_AxePending) return false;
    m_AxePending = false;
    outType = EntityType::AXE_PROJECTILE;
    outX = m_AxeX;
    outY = m_AxeY;
    outDir = m_AxeDir;
    return true;
}

// ============================================================================
// ParaKoopaBehavior
// ============================================================================

void ParaKoopaBehavior::Update(EntityState& state,
                               [[maybe_unused]] const Level& level,
                               [[maybe_unused]] const Player& player,
                               int gameTimer) {
    if (state.IsSquished() || state.IsDead()) {
        return;
    }
    if (m_Mode != Mode::SHELL && state.IsInShellMode()) {
        m_Mode = Mode::SHELL;
        state.SetCollidable(true);
        state.SetGrounded(false);
        state.SetVelY(0.0f);
        state.SetFallHeight(0.0f);
    }

    if (!m_AnchorInitialized) {
        m_AnchorY = state.GetWorldY();
        m_AnchorInitialized = true;
    }

    if (m_Mode == Mode::FLYING) {
        // Flying ParaKoopa should be a pure vertical patrol enemy.
        // Keep it out of generic gravity/block-collision side effects.
        state.SetCollidable(false);
        state.SetGrounded(true);
        state.SetVelX(0.0f);
        state.SetVelY(0.0f);
        state.SetFallHeight(0.0f);

        float newY = state.GetWorldY() + PATROL_SPEED * m_VerticalDir;
        float upperBound = m_AnchorY - PATROL_RANGE;
        float lowerBound = m_AnchorY + PATROL_RANGE;

        if (newY <= upperBound) {
            newY = upperBound;
            m_VerticalDir = 1;
        } else if (newY >= lowerBound) {
            newY = lowerBound;
            m_VerticalDir = -1;
        }

        state.SetWorldY(newY);
    } else if (m_Mode == Mode::WALKING) {
        // Walking Koopa: no vertical patrol, normal gravity + block collision.
        state.SetCollidable(true);
        if (std::abs(state.GetVelX()) < 0.05f) {
            float walkSpeed =
                GameConfig::SCALED_SPEED / GameConfig::ENEMY_SPEED_DIVISOR;
            state.SetVelX(state.GetDirection() == 0 ? -walkSpeed : walkSpeed);
        }
    } else {
        // Shell mode: preserve shell movement, no patrol forcing.
        state.SetCollidable(true);
    }

    if (gameTimer % 10 == 0) state.AdvanceAnimationFrame();
}

bool ParaKoopaBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                          bool isFromAbove, [[maybe_unused]] GameStateManager& gameState,
                                          [[maybe_unused]] UIManager& uiManager, [[maybe_unused]] Camera& camera) {
    if (state.IsDead()) return false;

    PlayerState& ps = player.GetState();
    if (isFromAbove) {
        if (m_Mode == Mode::FLYING) {
            // 1st stomp: winged koopa -> normal walking koopa.
            m_Mode = Mode::WALKING;
            state.SetName("KoopaTroopa");
            state.SetSquashed(false);
            state.SetCollidable(true);
            state.SetVelY(0.0f);
            state.SetFallHeight(0.0f);
            state.SetGrounded(false);
            float walkSpeed =
                GameConfig::SCALED_SPEED / GameConfig::ENEMY_SPEED_DIVISOR;
            state.SetVelX(state.GetDirection() == 0 ? -walkSpeed : walkSpeed);
            AudioManager::GetInstance().PlaySFX(SFXName::Squish);
            return true;
        } else if (m_Mode == Mode::WALKING) {
            // 2nd stomp: walking koopa -> shell, starts stationary.
            m_Mode = Mode::SHELL;
            state.SetName("KoopaTroopaShell");
            state.SetSquashed(true);
            state.SetCollidable(true);
            state.SetVelY(0.0f);
            state.SetFallHeight(0.0f);
            state.SetGrounded(false);
            state.SetVelX(0.0f);
            AudioManager::GetInstance().PlaySFX(SFXName::Squish);
            return true;
        } else if (m_Mode == Mode::SHELL) {
            // Kick the shell
            AABB playerBox = ps.GetHitbox();
            AABB entityBox = state.GetHitbox();
            float playerCX = playerBox.left + (playerBox.right - playerBox.left) * 0.5f;
            float shellCX = entityBox.left + (entityBox.right - entityBox.left) * 0.5f;
            float kickSpeed = (playerCX > shellCX) ? -GameConfig::SCALED_SPEED : GameConfig::SCALED_SPEED;
            state.KickShell(kickSpeed);
            ps.SetInvTimer(5);
            return true;
        }
    } else {
        // Side collision
        if (m_Mode == Mode::SHELL) {
            if (std::abs(state.GetVelX()) < 0.1f) {
                // Stationary shell: kick
                AABB playerBox = ps.GetHitbox();
                AABB entityBox = state.GetHitbox();
                float playerCX = playerBox.left + (playerBox.right - playerBox.left) * 0.5f;
                float shellCX = entityBox.left + (entityBox.right - entityBox.left) * 0.5f;
                float kickSpeed = (playerCX > shellCX) ? -GameConfig::SCALED_SPEED : GameConfig::SCALED_SPEED;
                state.KickShell(kickSpeed);
                ps.SetInvTimer(5);
            } else {
                // Moving shell: damage player
                if (!ps.IsInvincible()) {
                    ps.TakeDamage();
                }
            }
            return true;
        } else {
            // Normal living ParaKoopa
            if (!ps.IsInvincible() && !state.IsSquished()) {
                ps.TakeDamage();
            }
            return true;
        }
    }
    return false;
}

std::unique_ptr<IEntityBehavior> ParaKoopaBehavior::Clone() const {
    return std::make_unique<ParaKoopaBehavior>(*this);
}

AABB ParaKoopaBehavior::GetHitbox(const EntityState& state) const {
    float w = static_cast<float>(state.GetWidth());
    float h = static_cast<float>(state.GetHeight());
    float hitW = w * 0.75f;
    float offsetX = (w - hitW) * 0.5f;
    return AABB::FromPosSize(state.GetX() + offsetX, state.GetY(), hitW, h);
}

bool ParaKoopaBehavior::IsShell() const { return m_Mode == Mode::SHELL; }

float ParaKoopaBehavior::GetVisualYOffset(const std::string& levelName) const {
    (void)levelName;
    return 0.0f;  // Now handled dynamically at View layer (Entity::UpdateView)
}

}  // namespace Mario
