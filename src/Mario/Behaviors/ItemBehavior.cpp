/**
 * @file ItemBehavior.cpp
 * @brief Implementation of power-up and collectible item behavior.
 * @inheritance IEntityBehavior <- ItemBehavior
 */
#include "Mario/Behaviors/ItemBehavior.hpp"

#include "Mario/Core/Collider.hpp"
#include "Mario/Level/EntityState.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/Player/Player.hpp"
#include "Mario/Player/PlayerState.hpp"
#include "Util/Logger.hpp"

namespace Mario {

ItemBehavior::ItemBehavior(ItemType type) : m_Type(type) {}

void ItemBehavior::Update(EntityState& state, [[maybe_unused]] const Level& level,
                          [[maybe_unused]] const Player& player, [[maybe_unused]] int gameTimer) {
    // Coins don't move - just animate in place
    if (m_Type == ItemType::COIN) {
        return;
    }

    // Power-ups that bounce when spawned
    if (state.IsStatic()) {
        // Static items (in blocks) don't move until spawned
        return;
    }

    // NOTE: Physics (gravity + position update) is already applied by
    // App::UpdatePlaying() This method only handles AI behavior logic

    // Star bounces when grounded, mushroom just falls
    if (state.IsGrounded()) {
        if (m_Type == ItemType::STAR) {
            state.SetGrounded(false);   // Jump up
            state.SetFallHeight(20.0);  // Star jump height
        }
    }
}

bool ItemBehavior::OnPlayerCollision(EntityState& state, [[maybe_unused]] Player& player,
                                     [[maybe_unused]] bool isFromAbove) {
    if (m_Type == ItemType::COIN) {
        // Coins are always collected
        state.Delete();
        return true;
    }

    if (m_Type == ItemType::MUSHROOM) {
        // If player is small, grow to big
        // This is handled in App collision manager
        state.Delete();
        return true;
    }

    if (m_Type == ItemType::FIRE_FLOWER) {
        // Grant fire power
        state.Delete();
        return true;
    }

    if (m_Type == ItemType::STAR) {
        // Grant invincibility
        state.Delete();
        return true;
    }

    if (m_Type == ItemType::ONE_UP) {
        // Grant extra life
        state.Delete();
        return true;
    }

    return false;
}

std::unique_ptr<IEntityBehavior> ItemBehavior::Clone() const {
    return std::make_unique<ItemBehavior>(*this);
}

}  // namespace Mario
