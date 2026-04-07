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

void FireballBehavior::Update(EntityState& state, const Level& level,
                              const Player& player, int gameTimer) {
    // Check lifetime
    m_LifetimeFrames--;
    if (m_LifetimeFrames <= 0) {
        state.Delete();  // Remove expired fireball
        return;
    }

    // Apply gravity for parabolic trajectory
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, state.IsGrounded());
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    // Horizontal movement
    float xDelta = state.GetVelX() * (state.GetDirection() == 1 ? 1 : -1);
    state.SetWorldX(state.GetWorldX() + xDelta);

    // Check ground collision for bouncing
    AABB footprint = state.GetCollider();
    footprint.top = footprint.bottom;
    footprint.bottom += GameConfig::GRAVITY_ACCELERATION;

    bool isGroundedNow = false;
    const std::vector<std::shared_ptr<Block>>& blocks = level.GetAllBlocks();
    for (const auto& block : blocks) {
        if (block && footprint.Intersects(block->GetAABB())) {
            isGroundedNow = true;
            break;
        }
    }

    if (isGroundedNow && !state.IsGrounded()) {
        // Bounce logic
        if (m_BounceCount < MAX_BOUNCES) {
            state.SetFallHeight(GameConfig::FIREBALL_BOUNCE_HEIGHT);
            m_BounceCount++;
        } else {
            // Max bounces reached - destroy
            state.Delete();
            return;
        }
    }

    state.SetGrounded(isGroundedNow);

    // Check for wall collision
    AABB wallCheck = state.GetCollider();
    if (state.GetDirection() == 1) {
        wallCheck.left = wallCheck.right;
    } else {
        wallCheck.right = wallCheck.left;
    }

    bool hitWall = false;
    for (const auto& block : blocks) {
        if (block && wallCheck.Intersects(block->GetAABB())) {
            hitWall = true;
            break;
        }
    }

    if (hitWall) {
        // Fireball destroyed on wall collision
        state.Delete();
        return;
    }

    // Animation
    if (state.IsAnimated()) {
        state.AdvanceAnimationFrame();
    }
}

bool FireballBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                         bool isFromAbove) {
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
