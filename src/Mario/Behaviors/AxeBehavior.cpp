/**
 * @file AxeBehavior.cpp
 * @brief Logic for Bowser's bridge axe trigger
 * @inheritance IEntityBehavior -> AxeBehavior
 */
#include "Mario/Behaviors/AxeBehavior.hpp"

#include "Mario/AudioManager.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/Player.hpp"

namespace Mario {
void AxeBehavior::Update(EntityState& state, [[maybe_unused]] const Level& level,
                         [[maybe_unused]] const Player& player, int gameTimer) {
    if (gameTimer % 8 == 0) {
        m_AnimationFrame = (m_AnimationFrame + 1) % 4;
        state.AdvanceAnimationFrame();
    }
}

bool AxeBehavior::OnPlayerCollision(EntityState& state, [[maybe_unused]] Player& player,
                                    [[maybe_unused]] bool isFromAbove) {
    if (state.IsDead()) return false;

    // Mark as deleted so app can detect bridge collapse trigger
    state.Delete();

    // Since we're hit, we consume the event
    return true;
}
}  // namespace Mario