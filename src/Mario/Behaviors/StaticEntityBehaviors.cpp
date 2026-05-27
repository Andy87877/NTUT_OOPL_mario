/**
 * @file StaticEntityBehaviors.cpp
 * @brief Implementations for AxeBehavior, PrincessBehavior, and AxeProjectileBehavior.
 * @inheritance IEntityBehavior <- AxeBehavior
 *              IEntityBehavior <- PrincessBehavior
 *              IEntityBehavior <- AxeProjectileBehavior
 */
#include "Mario/Behaviors/StaticEntityBehaviors.hpp"

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Level/EntityState.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Player/Player.hpp"

namespace Mario {

// ============================================================================
// AxeBehavior
// ============================================================================

void AxeBehavior::Update(EntityState& state,
                         [[maybe_unused]] const Level& level,
                         [[maybe_unused]] const Player& player, int gameTimer) {
    if (gameTimer % 8 == 0) {
        m_AnimationFrame = (m_AnimationFrame + 1) % 4;
        state.AdvanceAnimationFrame();
    }
}

bool AxeBehavior::OnPlayerCollision(EntityState& state,
                                    [[maybe_unused]] Player& player,
                                    [[maybe_unused]] bool isFromAbove) {
    if (state.IsDead()) return false;
    state.Delete();  // App detects deletion to trigger bridge collapse
    return true;
}

// ============================================================================
// PrincessBehavior
// ============================================================================

void PrincessBehavior::Update(EntityState& state,
                              [[maybe_unused]] const Level& level,
                              [[maybe_unused]] const Player& player,
                              int gameTimer) {
    state.SetVelX(0);
    state.SetVelY(0);
    if (state.IsAnimated() && gameTimer % 30 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool PrincessBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                         [[maybe_unused]] Player& player,
                                         [[maybe_unused]] bool isFromAbove) {
    // Level completion detected externally by LevelCompleteController
    return false;
}

std::unique_ptr<IEntityBehavior> PrincessBehavior::Clone() const {
    return std::make_unique<PrincessBehavior>(*this);
}

// ============================================================================
// AxeProjectileBehavior
// ============================================================================

void AxeProjectileBehavior::Update(EntityState& state,
                                   [[maybe_unused]] const Level& level,
                                   [[maybe_unused]] const Player& player,
                                   int gameTimer) {
    // Axe spin animation: rotate every 4 frames (matches C# visual feel)
    if (gameTimer % 4 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool AxeProjectileBehavior::OnPlayerCollision(EntityState& state,
                                              [[maybe_unused]] Player& player,
                                              [[maybe_unused]] bool isFromAbove) {
    if (state.IsDead()) return false;
    state.Delete();  // Consume and destroy projectile on player hit
    return true;
}

std::unique_ptr<IEntityBehavior> AxeProjectileBehavior::Clone() const {
    return std::make_unique<AxeProjectileBehavior>(*this);
}

}  // namespace Mario
