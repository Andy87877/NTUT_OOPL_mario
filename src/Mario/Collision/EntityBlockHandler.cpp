/**
 * @file EntityBlockHandler.cpp
 * @brief Entity-Block collision: ground snap, wall flip, pit deactivation.
 *        Ported from legacy App::CheckEntityBlockCollision().
 * @inheritance EntityBlockHandler : ICollisionHandler
 */
#include "Mario/Collision/EntityBlockHandler.hpp"

#include "Mario/Level/EntityDef.hpp"
#include "Mario/Level/EntityFactory.hpp"
#include "Mario/Core/GameConfig.hpp"

namespace Mario {

// ============================================================================
// Resolve — public entry point
// ============================================================================
void EntityBlockHandler::Resolve(
    Entity& entity, Level& level,
    std::vector<std::shared_ptr<Entity>>* outNewEntities) {
    // Bowser's fireball and thrown axes ignore blocks/walls completely in the original game
    bool ignoreBlocks =
        (entity.GetDef().type == EntityType::FIRE && entity.GetState().IsEnemy()) ||
        (entity.GetDef().type == EntityType::AXE_PROJECTILE);

    if (ignoreBlocks) {
        // Pit fall: deactivate entities that fall below the level floor.
        if (entity.GetState().GetY() >
            GameConfig::LEVEL_HEIGHT_PX + GameConfig::TILE_SIZE) {
            entity.GetState().Delete();
        }
        return;
    }

    CheckGround(entity.GetState(), level);
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
void EntityBlockHandler::CheckGround(EntityState& state, Level& level) {
    AABB box = state.GetHitbox();
    const int tileSize = GameConfig::TILE_SIZE;

    int leftTile = static_cast<int>(box.left) / tileSize;
    int rightTile = static_cast<int>(box.right - 1) / tileSize;
    int bottomTile = static_cast<int>(box.bottom) / tileSize;

    bool onGround = false;
    for (int x = leftTile; x <= rightTile; x++) {
        Block* block = level.GetBlockAt(x, bottomTile);
        if (block && block->IsSolid()) {
            AABB bb = block->GetAABB();
            float overlap = box.bottom - bb.top;
            if (overlap > 0 && overlap < tileSize * 0.75f) {
                state.SetY(bb.top - state.GetHeight());
                state.SetVelY(0.0f);
                state.SetGrounded(true);
                onGround = true;
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
    AABB box = state.GetHitbox();
    const int tileSize = GameConfig::TILE_SIZE;
    bool isFireball = (entity.GetDef().type == EntityType::FIRE) ||
                      (entity.GetDef().type == EntityType::AXE_PROJECTILE);

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

}  // namespace Mario
