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

void PrincessBehavior::Update(EntityState& state, const Level& level,
                              const Player& player, int gameTimer) {
    // Princess is static - no movement
    state.SetVelX(0);
    state.SetVelY(0);

    // Minimal idle animation (if applicable)
    if (state.IsAnimated() && gameTimer % 30 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool PrincessBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                         bool isFromAbove) {
    // Princess takes no collision damage
    // May trigger level_complete event, but handled elsewhere
    return false;
}

std::unique_ptr<IEntityBehavior> PrincessBehavior::Clone() const {
    return std::make_unique<PrincessBehavior>(*this);
}

}  // namespace Mario
