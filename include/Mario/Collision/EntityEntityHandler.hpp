/**
 * @file EntityEntityHandler.hpp
 * @brief Handles Entity-Entity collision for all active entity pairs per frame.
 *        Supports Fireball vs Enemy (kill + Explosion) and moving Koopa Shell
 *        vs Enemy (kill). Uses camera-based culling to skip far-off-screen
 * pairs.
 * @inheritance EntityEntityHandler : ICollisionHandler
 */
#ifndef MARIO_COLLISION_ENTITY_ENTITY_HANDLER_HPP
#define MARIO_COLLISION_ENTITY_ENTITY_HANDLER_HPP

#include <memory>
#include <vector>

#include "Mario/Collision/ICollisionHandler.hpp"
#include "Mario/Level/Entity.hpp"

namespace Mario {

class GameStateManager;

/**
 * Resolves entity-entity collisions for all active pairs each game frame.
 *
 * Supported interactions:
 *   - Fireball vs Enemy : enemy dies (FIREBALL), fireball deleted.
 *                         Delegates to entity behavior first (e.g. Bowser HP).
 *   - Moving Shell vs Enemy : enemy dies (SHELL_HIT).
 *
 * Camera culling: pairs where both entities are more than 200 px outside the
 * visible window are skipped. Pass cameraOffset = -9999 to disable culling.
 */
class EntityEntityHandler : public ICollisionHandler {
   public:
    EntityEntityHandler() = default;

    /**
     * Check all entity-entity pairs for one frame.
     * @param entities      All entities in the level.
     * @param gameState     For score management.
     * @param cameraOffset  Left edge of the visible window; -9999 disables
     * culling.
     */
    void Resolve(std::vector<std::shared_ptr<Entity>>& entities,
                 GameStateManager& gameState, float cameraOffset = -9999.0f);

   private:
    /** Returns true when entity is a Koopa shell that is currently moving. */
    static bool IsMovingShell(const std::shared_ptr<Entity>& entity,
                              const EntityState& state);
};

}  // namespace Mario

#endif  // MARIO_COLLISION_ENTITY_ENTITY_HANDLER_HPP
