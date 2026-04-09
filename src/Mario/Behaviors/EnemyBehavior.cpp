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

    // Horizontal movement: VelX already includes direction (set in
    // EntityState::Init)
    if (!state.IsStatic()) {
        state.SetWorldX(state.GetWorldX() + state.GetVelX());
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

    // Check for wall collision using the same logic as CollisionManager for
    // Mario Only change direction when actually colliding with a solid block
    AABB enemyBox = state.GetCollider();
    int topTile = static_cast<int>(enemyBox.top) / GameConfig::TILE_SIZE;
    int bottomTile =
        static_cast<int>(enemyBox.bottom - 1) / GameConfig::TILE_SIZE;

    bool hitWall = false;

    // Check collision in the direction of movement
    if (state.GetDirection() == 1) {
        // Moving right: check right side of enemy
        int rightTile =
            static_cast<int>(enemyBox.right) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            const Block* block = level.GetBlockAt(rightTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (enemyBox.Intersects(blockBox)) {
                    hitWall = true;
                    break;
                }
            }
        }
    } else {
        // Moving left: check left side of enemy
        int leftTile = static_cast<int>(enemyBox.left) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            const Block* block = level.GetBlockAt(leftTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (enemyBox.Intersects(blockBox)) {
                    hitWall = true;
                    break;
                }
            }
        }
    }

    // Change direction if hit wall or no ground (fell off platform)
    // But only allow direction change every 15 frames to prevent rapid flipping
    if ((hitWall || !isGroundedNow) &&
        (m_DirectionChangeCounter - m_LastDirectionChangeFrame) > 15) {
        int newDir = state.GetDirection();
        if (newDir == 1) {
            state.SetDirection(0);  // Change to left
        } else {
            state.SetDirection(1);  // Change to right
        }
        // Also reverse velocity when changing direction
        state.SetVelX(-state.GetVelX());
        m_LastDirectionChangeFrame = m_DirectionChangeCounter;
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
