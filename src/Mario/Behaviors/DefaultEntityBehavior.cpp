/**
 * @file DefaultEntityBehavior.cpp
 * @brief Implementation of DefaultEntityBehavior passive entity behavior.
 * @inheritance IEntityBehavior <- DefaultEntityBehavior
 */
#include "Mario/Behaviors/DefaultEntityBehavior.hpp"

#include "Mario/EntityState.hpp"
#include "Mario/GameStateManager.hpp"
#include "Mario/Level.hpp"
#include "Mario/Player.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void DefaultEntityBehavior::Update([[maybe_unused]] EntityState& state, [[maybe_unused]] const Level& level,
                                   [[maybe_unused]] const Player& player, [[maybe_unused]] int gameTimer) {
    // Passive entities don't move or update actively
    // Just let animation play if animated
    if (state.IsAnimated()) {
        // Animation frame update is handled by EntityState
        // This method does nothing for default behavior
    }
}

bool DefaultEntityBehavior::OnPlayerCollision(EntityState& state,
                                              [[maybe_unused]] Player& player,
                                              [[maybe_unused]] bool isFromAbove) {
    // For coins: add score, mark for removal
    if (state.IsCoin()) {
        state.Delete();
        // Score is handled by GameStateManager in App
        return true;  // Consumed
    }

    // For power-ups: apply power state to player
    if (state.IsPowerUp()) {
        [[maybe_unused]] int powerUpState = state.GetPowerUpState();
        // Power-up application logic handled in App collision manager
        state.Delete();  // Remove the item
        return true;     // Consumed
    }

    return false;  // No special collision
}

std::unique_ptr<IEntityBehavior> DefaultEntityBehavior::Clone() const {
    return std::make_unique<DefaultEntityBehavior>(*this);
}

}  // namespace Mario
