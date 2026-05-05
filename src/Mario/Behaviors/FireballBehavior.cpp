/**
 * @file FireballBehavior.cpp
 * @brief Implementation of fireball projectile behavior.
 * @inheritance IEntityBehavior <- FireballBehavior
 */
#include "Mario/Behaviors/FireballBehavior.hpp"

#include "Mario/Collider.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Util/Logger.hpp"

namespace Mario {

FireballBehavior::FireballBehavior(FireballType type)
    : m_Type(type), m_LifetimeFrames(240) {}

void FireballBehavior::Update(EntityState& state, [[maybe_unused]] const Level& level,
                              [[maybe_unused]] const Player& player, [[maybe_unused]] int gameTimer) {
    // Check lifetime
    m_LifetimeFrames--;
    if (m_LifetimeFrames <= 0) {
        state.Delete();  // Remove expired fireball
        return;
    }

    // NOTE: Basic physics (gravity + velocity) are handled by
    // EntityState::Tick() DO NOT apply them here or they'll be applied twice!
    // FireballBehavior only handles special fireball logic (bounce, etc.)

    // Ground bounding is managed by App::CheckEntityBlockCollision.
    // If App.cpp set us as grounded, we bounce!
    if (state.IsGrounded()) {
        // Bounce logic (bounce forever like C# reference)
        state.SetGrounded(false);  // jump up
        state.SetFallHeight(GameConfig::JUMP_LOW_VELOCITY);
    }

    // All collision checking (ground, wall, entities) is handled by App.cpp
    // This behavior only updates position and animation

    // Animation
    if (state.IsAnimated()) {
        state.AdvanceAnimationFrame();
    }
}

bool FireballBehavior::OnPlayerCollision(EntityState& state, [[maybe_unused]] Player& player,
                                         [[maybe_unused]] bool isFromAbove) {
    // Player fireball: doesn't collide with player, only with enemies
    // This method is not called for player fireball hitting player
    // Bowser fireball: damages player (handled at App level)
    if (m_Type == FireballType::BOWSER) {
        // Bowser fireball hit player - damage is handled by App
        state.Delete();
        return true;
    }

    return false;
}

std::unique_ptr<IEntityBehavior> FireballBehavior::Clone() const {
    return std::make_unique<FireballBehavior>(*this);
}

}  // namespace Mario
