/**
 * @file EnemyBehavior.cpp
 * @brief Implementation of enemy AI behavior (Goomba, Koopa Troopa).
 *        Ported from C# Entity.cs Update() and collision logic.
 * @inheritance IEntityBehavior <- EnemyBehavior
 */
#include "Mario/Behaviors/EnemyBehavior.hpp"

#include "Mario/Collider.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Mario/PlayerState.hpp"
#include "Util/Logger.hpp"

namespace Mario {

EnemyBehavior::EnemyBehavior(EnemyType type) : m_Type(type) {}

void EnemyBehavior::Update(EntityState& state, const Level& level,
                           const Player& player, int gameTimer) {
    if (state.IsSquished() || state.IsDead()) {
        // Dead enemies don't move, just animate squish state
        if (!state.IsAnimated()) {
            state.Delete();  // Remove after animation done
        }
        return;
    }

    // Apply gravity
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, state.IsGrounded());
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    // Horizontal movement based on direction
    if (!state.IsStatic()) {
        float xDelta = state.GetVelX() * (state.GetDirection() == 1 ? 1 : -1);
        state.SetWorldX(state.GetWorldX() + xDelta);
    }

    // Simple patrol AI: walk until hitting a wall, then turn around
    // Check for ground collision by looking below
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

    state.SetGrounded(isGroundedNow);

    // Check for wall collision by looking at direction of movement
    AABB wallCheck = state.GetCollider();
    if (state.GetDirection() == 1) {
        wallCheck.left = wallCheck.right;
        wallCheck.right += state.GetVelX();
    } else {
        wallCheck.right = wallCheck.left;
        wallCheck.left -= std::abs(state.GetVelX());
    }

    bool hitWall = false;
    for (const auto& block : blocks) {
        if (block && wallCheck.Intersects(block->GetAABB())) {
            hitWall = true;
            break;
        }
    }

    // Change direction if hit wall or no ground
    if (hitWall || !isGroundedNow) {
        int newDir = state.GetDirection();
        if (newDir == 1) {
            state.SetDirection(0);  // Change to left
        } else {
            state.SetDirection(1);  // Change to right
        }
    }

    // Update animation frame
    m_DirectionChangeCounter++;
    if (state.IsAnimated() && m_DirectionChangeCounter % 10 == 0) {
        // Advance animation frame every 10 ticks
        state.AdvanceAnimationFrame();
    }
}

bool EnemyBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                      bool isFromAbove) {
    if (state.IsSquished() || state.IsDead()) {
        return false;
    }

    if (isFromAbove) {
        // Player jumped on enemy
        if (state.IsSquishable()) {
            state.Squish();  // Mark as squashed
            return true;
        } else if (m_Type == EnemyType::KOOPA_TROOPA) {
            // Koopa: kick it (spawn moving shell)
            state.Squish();  // Mark as squashed/kicked
            return true;
        }
    } else {
        // Player hit from side - enemy hurts player
        // This is handled at App level with star state check
        return true;  // Collision was processed
    }

    return false;
}

std::unique_ptr<IEntityBehavior> EnemyBehavior::Clone() const {
    return std::make_unique<EnemyBehavior>(*this);
}

}  // namespace Mario
