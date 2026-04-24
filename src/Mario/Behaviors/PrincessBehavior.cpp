/**
 * @file PrincessBehavior.cpp
 * @brief Implementation of Princess static NPC behavior.
 * @inheritance IEntityBehavior <- PrincessBehavior
 */
#include "Mario/Behaviors/PrincessBehavior.hpp"

#include "Mario/EntityState.hpp"
#include "Mario/Level.hpp"
#include "Mario/Player.hpp"

namespace Mario {

void PrincessBehavior::Update(EntityState& state, const Level& /*level*/,
                              const Player& /*player*/, int gameTimer) {
    // Princess is static - no movement
    state.SetVelX(0);
    state.SetVelY(0);

    // Minimal idle animation (if applicable)
    if (state.IsAnimated() && gameTimer % 30 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool PrincessBehavior::OnPlayerCollision(EntityState& state, Player& /*player*/,
                                         bool /*isFromAbove*/) {
    // Princess collision marks level as complete
    // The LevelCompleteController will detect princess collision and trigger
    // game clear Princess is consumable (disappears after player touches) Note:
    // Game completion logic handled by App/LevelCompleteController
    return false;
}

std::unique_ptr<IEntityBehavior> PrincessBehavior::Clone() const {
    return std::make_unique<PrincessBehavior>(*this);
}

}  // namespace Mario
