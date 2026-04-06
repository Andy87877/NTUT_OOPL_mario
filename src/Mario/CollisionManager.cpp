/**
 * @file CollisionManager.cpp
 * @brief Implementation of Player-Block collision detection and resolution.
 *        Ported from C# Form1.cs collision logic.
 * @inheritance None
 */
#include "Mario/CollisionManager.hpp"

#include <cmath>

namespace Mario {

void CollisionManager::CheckPlayerBlockCollision(
    Player& player, Level& level, float cameraOffset) {

    PlayerState& state = player.GetState();

    // Apply gravity first
    float yDelta = state.ApplyGravity();

    // Apply velocities to position
    float newX = state.GetX() + state.GetVelX();
    float newY = state.GetY() + yDelta;

    state.SetX(newX);
    state.SetY(newY);

    // Resolve collisions in order: ground, ceiling, walls
    CheckGroundCollision(state, level);
    CheckCeilingCollision(state, level);
    CheckWallCollision(state, level, cameraOffset);

    // Prevent player from going past left boundary
    if (state.GetX() < 0.0f) {
        state.SetX(0.0f);
    }
}

bool CollisionManager::CheckPitFall(const Player& player) const {
    // Player fell below the level (16 rows * 32 pixels = 512)
    float levelBottom = static_cast<float>(
        GameConfig::VIEWPORT_TILES_Y * GameConfig::TILE_SIZE);
    return player.GetState().GetY() > levelBottom + GameConfig::TILE_SIZE;
}

void CollisionManager::CheckGroundCollision(PlayerState& state, Level& level) {
    AABB playerBox = state.GetHitbox();

    // Check blocks below the player's feet
    int leftTile  = static_cast<int>(playerBox.left) / GameConfig::TILE_SIZE;
    int rightTile = static_cast<int>(playerBox.right - 1) / GameConfig::TILE_SIZE;
    int bottomTile = static_cast<int>(playerBox.bottom) / GameConfig::TILE_SIZE;

    bool foundGround = false;

    for (int x = leftTile; x <= rightTile; x++) {
        Block* block = level.GetBlockAt(x, bottomTile);
        if (block && block->IsSolid()) {
            AABB blockBox = block->GetAABB();

            // Check if player is actually overlapping
            if (playerBox.Intersects(blockBox)) {
                // Check it's a downward collision (feet hitting top of block)
                float overlapY = playerBox.bottom - blockBox.top;
                if (overlapY > 0 && overlapY < GameConfig::TILE_SIZE * 0.75f) {
                    // Snap player to top of block
                    state.SetY(blockBox.top - static_cast<float>(state.GetHeight()));
                    state.SetVelY(0.0);
                    state.SetFallHeight(0.0);
                    state.SetGrounded(true);
                    foundGround = true;
                }
            }
        }
    }

    // Also check one tile below current position (fall detection)
    if (!foundGround) {
        int belowTile = (static_cast<int>(playerBox.bottom) + 1) / GameConfig::TILE_SIZE;
        for (int x = leftTile; x <= rightTile; x++) {
            Block* block = level.GetBlockAt(x, belowTile);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                float gap = blockBox.top - playerBox.bottom;
                if (gap >= 0 && gap <= 2.0f) {
                    state.SetY(blockBox.top - static_cast<float>(state.GetHeight()));
                    state.SetVelY(0.0);
                    state.SetFallHeight(0.0);
                    state.SetGrounded(true);
                    foundGround = true;
                    break;
                }
            }
        }
    }

    if (!foundGround && state.IsGrounded()) {
        // Player walked off an edge
        state.SetGrounded(false);
    }
}

void CollisionManager::CheckCeilingCollision(PlayerState& state, Level& level) {
    AABB playerBox = state.GetHitbox();

    int leftTile  = static_cast<int>(playerBox.left) / GameConfig::TILE_SIZE;
    int rightTile = static_cast<int>(playerBox.right - 1) / GameConfig::TILE_SIZE;
    int topTile   = static_cast<int>(playerBox.top) / GameConfig::TILE_SIZE;

    for (int x = leftTile; x <= rightTile; x++) {
        Block* block = level.GetBlockAt(x, topTile);
        if (block && block->IsSolid()) {
            AABB blockBox = block->GetAABB();

            if (playerBox.Intersects(blockBox)) {
                float overlapY = blockBox.bottom - playerBox.top;
                if (overlapY > 0 && overlapY < GameConfig::TILE_SIZE * 0.75f) {
                    // Head bump: push player down
                    state.SetY(blockBox.bottom);
                    state.SetVelY(0.0);
                    state.SetFallHeight(0.0);

                    // Trigger block hit
                    block->OnHit(state.GetState());
                }
            }
        }
    }
}

void CollisionManager::CheckWallCollision(
    PlayerState& state, Level& level, float /*cameraOffset*/) {

    AABB playerBox = state.GetHitbox();

    int topTile    = static_cast<int>(playerBox.top) / GameConfig::TILE_SIZE;
    int bottomTile = static_cast<int>(playerBox.bottom - 1) / GameConfig::TILE_SIZE;

    // Check right wall
    if (state.GetVelX() > 0) {
        int rightTile = static_cast<int>(playerBox.right) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            Block* block = level.GetBlockAt(rightTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (playerBox.Intersects(blockBox)) {
                    state.SetX(blockBox.left -
                        static_cast<float>(state.GetWidth()));
                    state.SetVelX(0.0f);
                    break;
                }
            }
        }
    }

    // Check left wall
    if (state.GetVelX() < 0) {
        int leftTile = static_cast<int>(playerBox.left) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            Block* block = level.GetBlockAt(leftTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (playerBox.Intersects(blockBox)) {
                    state.SetX(blockBox.right);
                    state.SetVelX(0.0f);
                    break;
                }
            }
        }
    }
}

} // namespace Mario
