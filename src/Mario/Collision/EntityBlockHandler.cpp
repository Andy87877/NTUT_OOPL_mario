/**
 * @file EntityBlockHandler.cpp
 * @brief Entity-Block collision: ground snap, wall flip, pit deactivation.
 *        Ported from legacy App::CheckEntityBlockCollision().
 * @inheritance EntityBlockHandler : ICollisionHandler
 */
#include "Mario/Collision/EntityBlockHandler.hpp"

#include <algorithm>
#include <cmath>

#include "Mario/Level/EntityDef.hpp"
#include "Mario/Level/EntityFactory.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Level/Entity.hpp"

namespace Mario {

// ============================================================================
// Resolve — public entry point
// ============================================================================
void EntityBlockHandler::Resolve(
    Entity& entity, Level& level,
    std::vector<std::shared_ptr<Entity>>* outNewEntities) {
    // Bypasses block collision resolution if flagged by the behavior (e.g. projectiles, plants, lava bubbles)
    auto* behavior = entity.GetBehavior();
    bool ignoreBlocks = behavior && behavior->IgnoresBlocks();

    if (ignoreBlocks) {
        // Pit fall: deactivate entities that fall below the level floor.
        if (entity.GetState().GetY() >
            GameConfig::LEVEL_HEIGHT_PX + GameConfig::TILE_SIZE) {
            entity.GetState().Delete();
        }
        return;
    }

    CheckGround(entity, level);
    CheckCeiling(entity, level);
    CheckWalls(entity, level, outNewEntities);

    // Pit fall: deactivate entities that fall below the level floor.
    if (entity.GetState().GetY() >
        GameConfig::LEVEL_HEIGHT_PX + GameConfig::TILE_SIZE) {
        entity.GetState().Delete();
    }
}

// ============================================================================
// CheckGround
// Snaps the entity to the top of any solid block it sinks into.
// ============================================================================
void EntityBlockHandler::CheckGround(Entity& entity, Level& level) {
    EntityState& state = entity.GetState();

    // If the entity is currently rising (e.g. bouncing fireball, jumping Koopa),
    // skip grounding resolution to prevent upward snapping and allow natural arc.
    if (state.GetFallHeight() > 0.0) {
        if (state.IsGrounded()) {
            state.SetGrounded(false);
        }
        return;
    }

    AABB box = entity.GetHitbox();
    const int tileSize = GameConfig::TILE_SIZE;

    // Shift 1.0f downward to check the block directly below the entity.
    // This removes the 1-frame oscillation bug when resting exactly on block tops.
    float checkY = box.bottom + 1.0f;
    int leftTile = static_cast<int>(box.left) / tileSize;
    int rightTile = static_cast<int>(box.right - 1) / tileSize;
    int bottomTile = static_cast<int>(checkY) / tileSize;

    // Velocity-adaptive threshold: scales with downward velocity to prevent
    // fast-falling projectiles (fireballs) or kicked shells from clipping through the floor.
    float threshold = std::max(static_cast<float>(tileSize) * 0.75f,
                               static_cast<float>(std::abs(state.GetVelY()) + 2.0f));

    bool onGround = false;
    for (int x = leftTile; x <= rightTile; x++) {
        Block* block = level.GetBlockAt(x, bottomTile);
        if (block && block->IsSolid()) {
            AABB bb = block->GetAABB();
            float overlap = box.bottom - bb.top;
            if (overlap >= 0.0f && overlap < threshold) {
                state.SetY(bb.top - state.GetHeight());
                state.SetVelY(0.0);
                state.SetFallHeight(0.0);
                state.SetGrounded(true);
                onGround = true;
                break;
            }
        }
    }
    if (!onGround && state.IsGrounded()) {
        state.SetGrounded(false);
    }
}

// ============================================================================
// CheckWalls
// Flip direction when the entity's leading edge enters a solid tile wall.
// Fireball: delete + spawn Explosion instead of flipping.
// ============================================================================
void EntityBlockHandler::CheckWalls(
    Entity& entity, Level& level,
    std::vector<std::shared_ptr<Entity>>* outNewEntities) {
    EntityState& state = entity.GetState();
    AABB box = entity.GetHitbox();
    const int tileSize = GameConfig::TILE_SIZE;
    auto* behavior = entity.GetBehavior();
    bool isFireball = behavior && behavior->ExplodesOnWall();

    auto spawnExplosion = [&]() {
        if (!outNewEntities) return;
        const EntityDef& expDef = level.GetEntityDefByName("Explosion");
        if (expDef.name.empty()) return;
        auto exp = EntityFactory::SpawnEntity(expDef, state.GetX(),
                                              state.GetY(), 1, false);
        if (exp) outNewEntities->push_back(exp);
    };

    if (state.GetVelX() > 0.0f) {
        int rtile = static_cast<int>(box.right) / tileSize;
        for (int y = static_cast<int>(box.top) / tileSize;
             y <= static_cast<int>(box.bottom - 1) / tileSize; y++) {
            Block* block = level.GetBlockAt(rtile, y);
            if (block && block->IsSolid()) {
                if (isFireball) {
                    state.Delete();
                    spawnExplosion();
                } else {
                    state.FlipDirection();
                    state.SetX(block->GetWorldX() - state.GetWidth());
                }
                break;
            }
        }
    } else if (state.GetVelX() < 0.0f) {
        int ltile = static_cast<int>(box.left) / tileSize;
        for (int y = static_cast<int>(box.top) / tileSize;
             y <= static_cast<int>(box.bottom - 1) / tileSize; y++) {
            Block* block = level.GetBlockAt(ltile, y);
            if (block && block->IsSolid()) {
                if (isFireball) {
                    state.Delete();
                    spawnExplosion();
                } else {
                    state.FlipDirection();
                    state.SetX(block->GetWorldX() + tileSize);
                }
                break;
            }
        }
    }
}

// ============================================================================
// CheckCeiling
// Snap head of entity to the bottom of solid block when rising.
// ============================================================================
void EntityBlockHandler::CheckCeiling(Entity& entity, Level& level) {
    EntityState& state = entity.GetState();

    // Only check ceiling if the entity is moving upward (rising in a jump/bounce).
    if (state.GetVelY() >= 0.0 && state.GetFallHeight() <= 0.0) {
        return;
    }

    AABB box = entity.GetHitbox();
    const int tileSize = GameConfig::TILE_SIZE;

    // Shift 1.0f upward to check the block directly above the entity.
    float checkY = box.top - 1.0f;
    int leftTile = static_cast<int>(box.left) / tileSize;
    int rightTile = static_cast<int>(box.right - 1) / tileSize;
    int topTile = static_cast<int>(checkY) / tileSize;

    // Boundary check
    if (topTile < 0) return;

    for (int x = leftTile; x <= rightTile; x++) {
        Block* block = level.GetBlockAt(x, topTile);
        if (block && block->IsSolid()) {
            AABB bb = block->GetAABB();
            // Snap entity top to the bottom of the ceiling block
            state.SetY(bb.bottom);
            state.SetVelY(0.0);
            state.SetFallHeight(0.0);
            break;
        }
    }
}

}  // namespace Mario
