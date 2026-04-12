/**
 * @file KoopaBehavior.cpp
 * @brief Implementation of Koopa Troopa and Shell behavior.
 *        Logic ported from C# Entity.cs Update() method.
 *        Shell spawning handled by App::CheckPlayerEntityCollision (C# Squish).
 * @inheritance IEntityBehavior <- KoopaBehavior
 */
#include "Mario/Behaviors/KoopaBehavior.hpp"

#include "Mario/Collider.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Util/Logger.hpp"

namespace Mario {

KoopaBehavior::KoopaBehavior(KoopaType type) : m_Type(type) {}

void KoopaBehavior::Update(EntityState& state, const Level& level,
                           const Player& player, int gameTimer) {
    if (state.IsSquished() || state.IsDead()) {
        // Dead koopas don't move
        if (!state.IsAnimated()) {
            state.Delete();
        }
        return;
    }

    // Apply gravity (for both TROOPA and SHELL)
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, state.IsGrounded());
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    // Horizontal movement: velocity is set by constructor or App layer
    if (!state.IsStatic()) {
        state.SetWorldX(state.GetWorldX() + state.GetVelX());
    }

    // SHELL type: no AI (just gravity + App-controlled velocity)
    if (m_Type == KoopaType::SHELL) {
        // Check ground collision for shell falling
        AABB footprint = state.GetCollider();
        footprint.top = footprint.bottom;
        footprint.bottom += GameConfig::GRAVITY_ACCELERATION;

        bool isGroundedNow = false;
        const std::vector<std::shared_ptr<Block>>& blocks =
            level.GetAllBlocks();
        for (const auto& block : blocks) {
            if (block && footprint.Intersects(block->GetAABB())) {
                isGroundedNow = true;
                break;
            }
        }
        state.SetGrounded(isGroundedNow);
        return;
    }

    // TROOPA type: patrol AI with wall/pit detection
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

    // Check for wall collision (direction-based)
    AABB enemyBox = state.GetCollider();
    int topTile = static_cast<int>(enemyBox.top) / GameConfig::TILE_SIZE;
    int bottomTile =
        static_cast<int>(enemyBox.bottom - 1) / GameConfig::TILE_SIZE;

    bool hitWall = false;

    if (state.GetDirection() == 1) {
        // Moving right: check right side
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
        // Moving left: check left side
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

    // Change direction on wall collision (C# checkEntityCollisionLeft/Right)
    // Wall flips should be IMMEDIATE per C# Form1.cs lines 1888, 1909
    if (hitWall &&
        (m_DirectionChangeCounter - m_LastDirectionChangeFrame) > 15) {
        int newDir = state.GetDirection();
        if (newDir == 1) {
            state.SetDirection(0);  // Right → Left
        } else {
            state.SetDirection(1);  // Left → Right
        }
        state.SetVelX(-state.GetVelX());
        m_LastDirectionChangeFrame = m_DirectionChangeCounter;
    }

    // Secondary: Flip on falling off platform (slower, lower priority)
    // Only use if not detecting wall collision
    if (!hitWall && !isGroundedNow &&
        (m_DirectionChangeCounter - m_LastDirectionChangeFrame) > 30) {
        int newDir = state.GetDirection();
        if (newDir == 1) {
            state.SetDirection(0);
        } else {
            state.SetDirection(1);
        }
        state.SetVelX(-state.GetVelX());
        m_LastDirectionChangeFrame = m_DirectionChangeCounter;
    }

    // Update animation
    m_DirectionChangeCounter++;
    if (state.IsAnimated() && m_DirectionChangeCounter % 10 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool KoopaBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                      bool isFromAbove) {
    // Shell spawning is handled by App::CheckPlayerEntityCollision (C# Squish)
    // This method just returns false to let App handle all collision logic
    return false;
}

std::unique_ptr<IEntityBehavior> KoopaBehavior::Clone() const {
    return std::make_unique<KoopaBehavior>(*this);
}

}  // namespace Mario
