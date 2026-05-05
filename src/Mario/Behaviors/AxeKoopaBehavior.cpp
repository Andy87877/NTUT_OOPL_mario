/**
 * @file AxeKoopaBehavior.cpp
 * @brief Implementation of Axe-throwing Koopa behavior.
 * @inheritance IEntityBehavior <- AxeKoopaBehavior
 */
#include "Mario/Behaviors/AxeKoopaBehavior.hpp"

#include "Mario/EntityState.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void AxeKoopaBehavior::Update(EntityState& state, const Level& level,
                              [[maybe_unused]] const Player& player, int gameTimer) {
    if (state.IsSquished() || state.IsDead()) {
        return;
    }

    m_ThrowTimer++;

    // NOTE: Physics (gravity + position update) is already applied by
    // App::UpdatePlaying() This method only handles AI behavior logic

    // Periodic axe throw
    if (m_ThrowTimer >= AXE_THROW_INTERVAL) {
        // ✨ Spawn axe projectile below this entity (if callback is set)
        if (m_AxeSpawnCallback) {
            // Axe spawns at center bottom of AxeKoopa
            float axeX = state.GetX() + state.GetWidth() / 2.0f;
            float axeY = state.GetY() + state.GetHeight();
            m_AxeSpawnCallback(axeX, axeY);
        }
        m_ThrowTimer = 0;
    }

    // Check for wall collision and turn around (using grid-based collision like
    // Mario)
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

    if (hitWall) {
        int newDir = state.GetDirection() == 1 ? 0 : 1;
        state.SetDirection(newDir);
        state.SetVelX(
            -state.GetVelX());  // Reverse velocity when direction changes
    }

    // Animation
    if (state.IsAnimated() && gameTimer % 10 == 0) {
        state.AdvanceAnimationFrame();
    }
}

bool AxeKoopaBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state, [[maybe_unused]] Player& player,
                                         [[maybe_unused]] bool isFromAbove) {
    if (state.IsSquished() || state.IsDead()) {
        return false;
    }

    // AxeKoopa is immune to jump attacks (like Bowser)
    // Only damaged by game_end mechanism (axe trap, etc.)
    return false;
}

std::unique_ptr<IEntityBehavior> AxeKoopaBehavior::Clone() const {
    return std::make_unique<AxeKoopaBehavior>(*this);
}

}  // namespace Mario
