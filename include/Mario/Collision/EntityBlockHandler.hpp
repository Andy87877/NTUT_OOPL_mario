/**
 * @file EntityBlockHandler.hpp
 * @brief Handles Entity-Block collision for one non-player entity per frame.
 *        Checks ground snap, wall direction-flip, and pit-fall deactivation.
 *        For Fireball entities, wall contact spawns an Explosion instead of
 *        flipping direction.
 * @inheritance EntityBlockHandler : ICollisionHandler
 */
#ifndef MARIO_COLLISION_ENTITY_BLOCK_HANDLER_HPP
#define MARIO_COLLISION_ENTITY_BLOCK_HANDLER_HPP

#include <memory>
#include <vector>

#include "Mario/Collision/ICollisionHandler.hpp"
#include "Mario/Level/Entity.hpp"
#include "Mario/Level/Level.hpp"

namespace Mario {

/**
 * Resolves entity-block collisions each game frame.
 *
 * Ground: snaps the entity to the tile top when the entity sinks into it.
 * Walls:  flips GetVelX() sign when the entity's leading edge enters a solid
 *         tile. Fireball skips the flip and is deleted + Explosion spawned.
 * Pit:    deactivates any entity whose Y exceeds LEVEL_HEIGHT_PX + TILE_SIZE.
 */
class EntityBlockHandler : public ICollisionHandler {
   public:
    EntityBlockHandler() = default;

    /**
     * Run ground/wall/pit collision for one entity.
     * @param entity          The entity to check.
     * @param level           Current level block data.
     * @param outNewEntities  Optional: receives newly spawned entities
     *                        (e.g., Explosion when a Fireball hits a wall).
     */
    void Resolve(
        Entity& entity, Level& level,
        std::vector<std::shared_ptr<Entity>>* outNewEntities = nullptr);

   private:
    /** Snap entity feet to the tile top when sinking below a solid block. */
    void CheckGround(EntityState& state, Level& level);

    /**
     * Flip direction (or destroy Fireball) when the entity's leading edge
     * penetrates a solid tile wall.
     */
    void CheckWalls(Entity& entity, Level& level,
                    std::vector<std::shared_ptr<Entity>>* outNewEntities);
};

}  // namespace Mario

#endif  // MARIO_COLLISION_ENTITY_BLOCK_HANDLER_HPP
